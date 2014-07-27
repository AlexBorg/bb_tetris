///////////////////////////////////////////////////////////////////////////////
/// \file implement the InputHandler class
///////////////////////////////////////////////////////////////////////////////

// this module's h file.
#include "GameController.hpp"

// external includes
/// xenomai/posix includes
#include <mqueue.h>
#include <pthread.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>

//needed for sleep
#include <unistd.h>

// local includes
#include "BBTdefines.hpp"

// defines


///////////////////////////////////////////////////////////////////////////////
/// \brief
///
GameController :: GameController ()
{
  pthread_mutex_init ( &output_lock , NULL ) ;
}


///////////////////////////////////////////////////////////////////////////////
/// \brief
///
GameController :: ~GameController ()
{

  pthread_mutex_destroy ( &output_lock ) ;
}


///////////////////////////////////////////////////////////////////////////////
/// \brief start thread function
///
void GameController :: start ()
{
  pthread_attr_t attr ;
  pthread_attr_init ( &attr ) ;
  int policy = 0 ;
  int max_prio_for_policy = 0 ;
  
  pthread_attr_getschedpolicy ( &attr , &policy ) ;
  max_prio_for_policy = sched_get_priority_max ( policy ) ;

  rt_printf ( "GameController starting thread \n" ) ;
  
  pthread_create ( &thread , NULL ,  ( void* (*) ( void*) ) ( thread_func ) , this ) ;
  pthread_setschedprio ( thread , max_prio_for_policy ) ;
  pthread_attr_destroy ( &attr ) ;
}


///////////////////////////////////////////////////////////////////////////////
/// \brief copies the current data to the out_state passed to this function
/// \return true on success (always true)
///
bool GameController :: getGameState ( GameState &out_state )
{
  pthread_mutex_lock ( &output_lock ) ;
  out_state = game_state ; // FIXME: Borg: do we need to overload the copy?
  pthread_mutex_unlock ( &output_lock ) ;
  return true ;
}




///////////////////////////////////////////////////////////////////////////////
/// \brief lower the current block one square. If it is at an end, load the next
///   block
/// \return true on success
///
bool GameController :: downTick ()
{
  //check that current block can lower
  
  //if the current block is the lowest it can be, load next
}


///////////////////////////////////////////////////////////////////////////////
/// \brief toggle pause state
/// \return true on success
///
bool GameController :: pause ()
{
  
  return true ;
}


///////////////////////////////////////////////////////////////////////////////
/// \brief move the current block left if no collision
/// \return true on success
///
bool GameController :: left ()
{
  return true ;
}


///////////////////////////////////////////////////////////////////////////////
/// \brief move the current block right if no collision
/// \return true on success
///
bool GameController :: right ()
{
  return true ;
}


///////////////////////////////////////////////////////////////////////////////
/// \brief rotate the current block left if no collision
/// \return true on success
///
bool GameController :: rotateLeft ()
{
  return true ;
}


///////////////////////////////////////////////////////////////////////////////
/// \brief rotate the current block right if no collision
/// \return true on success
///
bool GameController :: rotateRight ()
{
  return true ;
}


///////////////////////////////////////////////////////////////////////////////
/// \brief drop the current block until a collision occurs
/// \return true on success
///
bool GameController :: down ()
{
  return true ;
}


///////////////////////////////////////////////////////////////////////////////
/// \brief move the current block based on the input event.
/// \return true on success
///
bool GameController :: processEvent ( int event )
{
  switch ( event )
  {
    case EV_PAUSE :
      return pause () ;
      break ;
    case EV_LEFT :
      return left () ;
      break ;
    case EV_RIGHT :
      return right () ;
      break ;
    case EV_ROT_LEFT :
      return rotateLeft () ;
      break ;
    case EV_ROT_RIGHT :
      return rotateRight () ;
      break ;
    case EV_DOWN :
      return down () ;
      break ;
  }

  rt_printf ( "unknown event\n" ) ;
  return false ;
}


///////////////////////////////////////////////////////////////////////////////
/// \brief process all events and update game board state for each one
/// \return true on success
///
bool GameController :: processTick ( mqd_t input )
{
  bool result = true ;
  pthread_mutex_lock ( &output_lock ) ;
  
  int event = 0 ;
  
  while ( mq_receive ( input
                      , ( char* ) &event
                      , sizeof ( event )
                      , NULL ))
  {
    processEvent ( event ) ;
  }
  
  // FIXME: Borg: the block shouldn't drop every frame, need timer for this.
  downTick () ;

CLEANUP:
  pthread_mutex_unlock ( &output_lock ) ;
  return result ;
}


///////////////////////////////////////////////////////////////////////////////
/// \brief thread function loop
/// \return always NULL
///
void* GameController :: exec ()
{
  mqd_t input = mq_open ( BBT_EVENT_QUEUE_NAME
                      , O_RDONLY ) ;

  if ( input < 0 )
  {
    rt_printf ( "GameController: failed to open queue" ) ;
    return NULL ;
  }
  
  while ( 1 )
  { // make periodic?
    processTick ( input ) ;
    sleep ( 1 ) ;
  }
  
  return NULL ;
}


///////////////////////////////////////////////////////////////////////////////
/// \brief thread function start point, calls local_thread_func
/// \return always NULL
///
void* GameController :: thread_func ( void* in_thread_obj )
{
  GameController* object = ( GameController* ) in_thread_obj ;
  return object -> exec () ;
}




