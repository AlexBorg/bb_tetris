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
  if(!game_state.active.tryMove(game_state.board, 0, -1, 0) &&
     !game_state.active.tryMove(game_state.board, 1, 0, 0)) {
    game_state.active.place(game_state.board);
    game_state.active = game_state.next;

    game_state.next.reinitialize();
    game_state.next.pos_x = 0;
    game_state.next.pos_y = 20;
  }
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
      game_state.active.tryMove(game_state.board, -1, 0, 0);
      break ;
    case EV_RIGHT :
      game_state.active.tryMove(game_state.board, 1, 0, 0);
      break ;
    case EV_ROT_LEFT :
      game_state.active.tryMove(game_state.board, 0, 0, -1);
      break ;
    case EV_ROT_RIGHT :
      game_state.active.tryMove(game_state.board, 0, 0, 1);
      break ;
    case EV_DOWN :
      // Move down until failure
      while(game_state.active.tryMove(game_state.board, 0, -1, 0));

      game_state.active.place(game_state.board);
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
                      , NULL ) != -1)
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
                      , O_RDONLY | O_NONBLOCK ) ;

  if ( input < 0 )
  {
    rt_printf ( "GameController: failed to open queue" ) ;
    return NULL ;
  }
  
  while ( 1 )
  { // make periodic?
    processTick ( input ) ;
    usleep ( 100000 ) ;
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




