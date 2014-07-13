///////////////////////////////////////////////////////////////////////////////
/// \file implement the InputHandler class
///////////////////////////////////////////////////////////////////////////////
#include "InputHandler.hpp"

#include <mqueue.h>
#include <pthread.h>
#include "BBTdefines.hpp"

//needed for sleep
#include <unistd.h>


///////////////////////////////////////////////////////////////////////////////
/// \brief default constructor, creates a message queue
///
InputHandler :: InputHandler ()
{
  struct mq_attr attr ;   // To store queue attributes

  // First we need to set up the attribute structure
  attr . mq_maxmsg = BBT_EVENT_QUEUE_SIZE ;
  attr . mq_msgsize = BBT_EVENT_MSG_SIZE ;
  attr . mq_flags = 0 ;
  
  out_queue = mq_open ( BBT_EVENT_QUEUE_NAME
                      , O_RDWR | O_CREAT
                      , 0664
                      , &attr ) ;
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

  pthread_create ( &thread , NULL ,  ( void* (*) ( void*) ) ( thread_func ) , this ) ;
  pthread_setschedprio ( thread , max_prio_for_policy ) ;
  pthread_attr_destroy ( &attr ) ;
}



///////////////////////////////////////////////////////////////////////////////
/// \brief static thread function passed to pthread. calls local_thread_func
///
void* InputHandler :: thread_func ( void* in_pointer )
{
  InputHandler* local_ptr = static_cast < InputHandler* > ( in_pointer ) ;
  local_ptr -> local_thread_func () ;
  return NULL ;
}



///////////////////////////////////////////////////////////////////////////////
/// \brief local thread function of this instance
///
void InputHandler :: local_thread_func ()
{
  // do processing here
  while ( 1 )
  {
    sleep ( 1000 ) ;
  }
}




