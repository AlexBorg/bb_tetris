///////////////////////////////////////////////////////////////////////////////
/// \file define the InputHandler class
///////////////////////////////////////////////////////////////////////////////

// external includes
#include <mqueue.h>
#include <linux/joystick.h>
#include <linux/input.h>
#include <string>

///////////////////////////////////////////////////////////////////////////////
/// \class handles input from keyboards and joysticks. Filters the expected events
/// into the event enumerations expected by the tetris game logic
///////////////////////////////////////////////////////////////////////////////
class InputHandler
{
public :
    InputHandler ( ) ;
  void start () ;
  
private :
  mqd_t out_queue ;
  pthread_t thread ;
  std :: string ps_dev_name ;
  std :: string keyboard_dev_name ;

  static int processEvent ( const js_event &e , mqd_t output ) ;
  static int processEvent ( const input_event &e , mqd_t output ) ;

  static void* thread_func ( void* ) ;
  static void local_thread_func ( void* ) ;
} ; 

