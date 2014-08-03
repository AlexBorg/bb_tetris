///////////////////////////////////////////////////////////////////////////////
/// \file implement the InputHandler class
///////////////////////////////////////////////////////////////////////////////

// this module's h file.
#include "InputHandler.hpp"

// external includes
/// xenomai/posix includes
#include <mqueue.h>
#include <pthread.h>

#include <linux/joystick.h>
#include <linux/input.h>

#include <stdio.h>
#include <glob.h>
#include <string.h>
#include <errno.h>

//needed for sleep
#include <unistd.h>

// local includes
#include "BBTdefines.hpp"

// defines
#define JOY_DEV "/dev/input/js0"

///////////////////////////////////////////////////////////////////////////////
/// \brief default constructor, creates a message queue
///
InputHandler :: InputHandler ()
{
  struct mq_attr attr ;   // To store queue attributes

  // First we need to set up the attribute structure
  memset ( &attr , 0 , sizeof ( attr ) ) ;
  attr . mq_maxmsg = BBT_EVENT_QUEUE_SIZE ;
  attr . mq_msgsize = BBT_EVENT_MSG_SIZE ;
  attr . mq_flags = 0 ;
  
  out_queue = mq_open ( BBT_EVENT_QUEUE_NAME
                      , O_WRONLY | O_CREAT
                      , 0664
                      , &attr ) ;

  if ( out_queue < 0 )
  {
    rt_printf ( "InputHandler failed to create queue %d\n" , errno ) ;
  }

}


///////////////////////////////////////////////////////////////////////////////
/// \brief check if an event file is of the correct type to open
///
bool isValidInputEventFile ( char* filename )
{
  bool result = false ;
  int fd = -1 ;
  char name [ 256 ] = "Unknown" ;

  if ( ( fd = open ( filename , O_RDONLY ) ) < 0 ) {
    perror("evdev open");
    return false ; //exit(1);
  }

  if ( ioctl ( fd , EVIOCGNAME ( sizeof ( name ) ) , name ) < 0) {
    rt_printf ( "evdev ioctl" ) ;
    goto CLEANUP ;
  }

  rt_printf ( "The device on %s says its name is %s\n"
             , filename 
             , name ) ;

  if ( strcasestr ( name , "PLAYSTATION" )
      || strcasestr ( name , "Keyboard" )
      || strcasestr ( name , "Gamepad" )
      )
  {
    rt_printf ( "using device%s\n" , filename ) ;
    result = true ;
  }

CLEANUP:
  close ( fd ) ;
  return result ;
}



///////////////////////////////////////////////////////////////////////////////
/// \brief starts the processing thread and sets priority to max
///
void InputHandler :: start ()
{
  pthread_attr_t attr ;
  pthread_attr_init ( &attr ) ;
  int policy = 0 ;
  int max_prio_for_policy = 0 ;
  
  pthread_attr_getschedpolicy ( &attr , &policy ) ;
  max_prio_for_policy = sched_get_priority_max ( policy ) ;

  glob_t globbuf ;
  glob ( "/dev/input/event*" , GLOB_TILDE , NULL , &globbuf ) ;
  printf ( "number of events found %d\n" , ( int ) globbuf . gl_pathc ) ;

  for ( unsigned int loop = 0 ; loop < globbuf . gl_pathc ; ++loop )
  {
    if ( isValidInputEventFile ( globbuf . gl_pathv [ loop ] ) )
    {
      std :: string *filename = new std :: string ( globbuf . gl_pathv [ loop ] ) ;   
      rt_printf ( "InputHandler starting thread: %s\n" , filename -> c_str () ) ;

      pthread_create ( &thread , NULL ,  ( void* (*) ( void*) ) ( thread_func ) , filename ) ;
      pthread_setschedprio ( thread , max_prio_for_policy ) ;
      pthread_attr_destroy ( &attr ) ;
    }
  }
}



///////////////////////////////////////////////////////////////////////////////
/// \brief static thread function passed to pthread. opens the device and 
///  attempts to process inputs from it.
///
void* InputHandler :: thread_func ( void* in_ptr )
{
  if ( in_ptr == NULL )
  {
    rt_printf ( "InputHandler :: thread_func: received invalide pointer" ) ;
    return NULL ;
  }

  std :: string *filename = ( std :: string* ) in_ptr ;
  int fd = -1 ;

  if ( ( fd = open ( filename -> c_str () , O_RDONLY ) ) < 0 ) {
    perror("evdev open");
    return NULL ; //exit(1);
  }

  mqd_t output = mq_open ( BBT_EVENT_QUEUE_NAME
                      , O_WRONLY ) ;

  if ( output < 0 )
  {
    rt_printf ( "InputHandler: failed to open queue" ) ;
  }

  /* the events (up to 64 at once) */
  //struct input_event ev[64];
  //rb=read(fd,ev,sizeof(struct input_event)*64);
  struct input_event ev;
  while ( 1 )
  {
    size_t rb = read ( fd , &ev , sizeof ( struct input_event ) ) ;
    if ( rb > 0 )
      processEvent ( ev , output ) ;
  }

/*******
// js# device operation. currently not used to have only a single processEvent
// function
  int joy_fd , *axis = NULL , num_of_axis = 0, num_of_buttons = 0 , x ;
  char *button = NULL , name_of_joystick [ 80 ] ;
  struct js_event js ;

  if ( ( joy_fd = open ( JOY_DEV , O_RDONLY ) ) == -1 )
  {
    rt_printf ( "Couldn't open joystick\n" ) ;
    return ; //-1;
  }

  ioctl ( joy_fd , JSIOCGAXES , &num_of_axis ) ;
  ioctl ( joy_fd , JSIOCGBUTTONS , &num_of_buttons ) ;
  ioctl ( joy_fd , JSIOCGNAME ( 80 ) , &name_of_joystick ) ;

  //axis = (int *) calloc( num_of_axis, sizeof( int ) );
  //button = (char *) calloc( num_of_buttons, sizeof( char ) );

  rt_printf ( "Joystick detected: %s\n\t%d axis\n\t%d buttons\n\n"
    , name_of_joystick
    , num_of_axis
    , num_of_buttons ) ;
  struct js_event e ;

  // do processing here
  while ( 1 )
  {
    read ( joy_fd , &e , sizeof ( e ) ) ;
    processEvent ( e ) ;
  }
// ****/
  mq_close ( output ) ;
  close ( fd ) ;
  return NULL ;
}



///////////////////////////////////////////////////////////////////////////////
/// \brief parse the event and pass messages to the next thread
/// \return 0 on success, 1 on ignore, negative for error
///
int InputHandler :: processEvent ( const input_event &e , mqd_t output )
{
  if ( ( e . type & EV_KEY ) && ( e . value == 1 ) )
  { // only worry about button presses, not releases
    //debugging statement used to find button mappings
    //rt_printf ( "type %d, button code %d, value %d\n", e . type , e . code , e . value ) ;
    
    int msg = EV_NONE ;
    switch ( e . code )
    {
       case 1 :   // kb 'esc'
       case 291 : // ps3 start
         msg = EV_PAUSE ;
         break ;
       case 75 :  // kb numpad 4 (left)
       case 105 : // kb left
       case 295 : // ps3 d-pad left
         msg = EV_LEFT ;
         break ;
       case 77 :  // kb numpad 6 (right)
       case 106 : // kb right
       case 293 : // ps3 d-pad right
         msg = EV_RIGHT ;
         break ;
       case 30 : // kb 'a'
       case 71 : // kb numpad 7 
       case 298 : // ps3 L1
       case 303 : // ps3 square
         msg = EV_ROT_LEFT ;
         break ;
       case 32 : // kb 'd'
       case 72 : // kb numpad 8 (up)
       case 73 : // kb numpad 9
       case 103 : // kb arrow up
       case 299 : // ps3 r1
       case 301 : // ps3 circle
       case 292 : // ps3 d-pad up
         msg = EV_ROT_RIGHT ;
         break ;
       case 80 :  // kb numpad 2 (down)
       case 108 : // kb arrow down
       case 294 : // ps3 d-pad down
         msg = EV_DOWN ;
         break ;
    }
    if ( msg != EV_NONE )
    {
      mq_send ( output , ( char* ) &msg , sizeof ( msg ) , 0 ) ;
      return 0 ;
    }   
  }
  return 1 ;
}



///////////////////////////////////////////////////////////////////////////////
/// \brief parse the event and pass messages to the next thread
/// \return 0 on success, 1 on ignore, negative for error
///
/// the following are button mappings for the PS3 controller from button to event . number
/// select = 0
/// start = 3
/// 
/// D-pad:
/// up = 4
/// right = 5
/// down = 6
/// left = 7
/// 
/// triangle = 12
/// x = 14
/// square = 15
/// o = 13
/// 
/// L1 = 10
/// L2 = 8
/// L3 = 1
/// R1 = 11
/// R2 = 9
/// R3 = 2
///
///
int InputHandler :: processEvent ( const js_event &e , mqd_t output )
{
  if ( ( e . type & JS_EVENT_BUTTON ) && ( e . value == 1 ) )
  { // only worry about button presses, not releases
    int msg = EV_NONE ;
    // debugging statement used to find button mappings
    rt_printf ( "button # %d, value %d\n", e . number , e . value ) ;
    switch ( e . number )
    {
       case 3 :
         msg = EV_PAUSE ;
         break ;
       case 7 :
         msg = EV_LEFT ;
         break ;
       case 5 :
         msg = EV_RIGHT ;
         break ;
       case 10 :
       case 15 :
         msg = EV_ROT_LEFT ;
         break ;
       case 4 :
       case 11 :
       case 13 :
         msg = EV_ROT_RIGHT ;
         break ;
       case 6 :
         msg = EV_DOWN ;
         break ;
    }
    if ( msg != EV_NONE )
    {
      mq_send ( output , ( char* ) &msg , sizeof ( msg ) , 0 ) ;
      return 0 ;
    }   
  }
  return 1 ;
}


