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
#define TICKS_TIL_DROP_MAX 100
#define TICKS_TIL_DROP_MIN 1
#define FULL_LINE_COLOR_MAX 7


///////////////////////////////////////////////////////////////////////////////
/// \brief
///
GameController :: GameController ()
{
  pthread_mutex_init ( &output_lock , NULL ) ;
  input_queue = mq_open ( BBT_EVENT_QUEUE_NAME
                      , O_RDONLY | O_NONBLOCK ) ;

  if ( input_queue < 0 )
  {
    rt_printf ( "GameController: failed to open queue" ) ;
  }
  reset () ;
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
  reset () ;
  pthread_attr_t attr ;
  pthread_attr_init ( &attr ) ;
  int policy = 0 ;
  int max_prio_for_policy = 0 ;
  
  pthread_attr_getschedpolicy ( &attr , &policy ) ;
  max_prio_for_policy = sched_get_priority_max ( policy ) ;

  rt_printf ( "GameController starting thread \n" ) ;
  
  pthread_create ( &thread , NULL ,  ( void* (*) ( void*) ) ( threadFunc ) , this ) ;
  pthread_setschedprio ( thread , max_prio_for_policy ) ;
  pthread_attr_destroy ( &attr ) ;
}



///////////////////////////////////////////////////////////////////////////////
/// \brief reset - initializes the game board for a new game
///
void GameController :: reset ()
{
  ticks_til_drop = TICKS_TIL_DROP_MAX ;
  tick_count = 0 ;
  game_state . reset () ;
  full_lines . clear () ;
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
  if(!game_state.active.tryMove(game_state.board, 0, -1, 0) )
  {
    game_state.active.place(game_state.board);
    if ( game_state . active . pos_y >= BOARD_HEIGHT - 3 ) // HEIGHT - 3 is the starting position
    {
      game_state . game_over = true ;
    }
    game_state.active = game_state.next;

    game_state.next.reinitialize();

    int lines = getFullLines () ;
    
    game_state . lines_cleared += lines ;
    game_state . level = game_state . lines_cleared / 10 + 1 ;
    ticks_til_drop = TICKS_TIL_DROP_MAX - ( game_state . level * 5 ) ;
    if ( ticks_til_drop > TICKS_TIL_DROP_MAX ) // overflow
      ticks_til_drop = 0 ;
    
    game_state.score += lines * lines * 10 ;
    game_state.score++ ; // one point for each block dropped
  }
  return true ;
}



///////////////////////////////////////////////////////////////////////////////
/// \brief determine if any lines are full
/// \return the number of full lines
///
int GameController :: getFullLines ()
{
  for ( unsigned int y = 0 ; y < BOARD_HEIGHT ; ++y )
  {
    bool line_full = true ;
    for ( unsigned int x = 0 ; x < BOARD_WIDTH ; ++x )
    {
      if ( game_state . board [ x ] [ y ] . color == 0 )
      {
        line_full = false ;
        break ;
      }
    }
    if ( line_full )
    {
      full_lines . push_back ( y ) ;
    }
  }
  return full_lines . size () ;
}



///////////////////////////////////////////////////////////////////////////////
/// \brief blink the full lines and then remove if count has passed.
/// \return true on success
///
bool GameController :: processFullLines ()
{ 
  if ( tick_count >= FULL_LINE_COLOR_MAX )
  {
    return removeFullLines () ;
  }
  
  for ( unsigned int loop = 0 ; loop < full_lines . size () ; ++loop )
  {
    int y = full_lines [ loop ] ;
    for ( unsigned int x = 0 ; x < BOARD_WIDTH ; ++x )
    {
      game_state . board [ x ] [ y ] . color = tick_count ;
    }
  }
  return true ;
}




///////////////////////////////////////////////////////////////////////////////
/// \brief remove the full lines and drop the remaining lines
/// \return true on success
///
bool GameController :: removeFullLines ()
{
  unsigned int line_offset = 0 ;
  unsigned int full_line_index = 0 ;
  for ( unsigned int y = 0 ; y < BOARD_HEIGHT ; ++y )
  {
    while ( full_line_index < full_lines . size ()
        && ( y + line_offset == full_lines [ full_line_index ] ))
    {
      ++line_offset ;
      ++full_line_index ;
    }
    if ( line_offset )
    {
      for ( unsigned int x = 0 ; x < BOARD_WIDTH ; ++x )
      {
        if ( y + line_offset < game_state . board [ 0 ] . size () )
          game_state . board [ x ] [ y ] = game_state . board [ x ] [ y + line_offset ] ;
        else
          game_state . board [ x ] [ y ] = BlockData ( 0 , 0 ) ;
      }
    }
  }
  
  full_lines . clear () ;
  tick_count = 0 ;
  return true ;
}



///////////////////////////////////////////////////////////////////////////////
/// \brief toggle pause state
/// \return true on success
///
bool GameController :: pause ()
{
  game_state . paused = !game_state . paused ;
  if ( game_state . game_over )
  {
    reset () ;
  }
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
      if ( !game_state . paused )
        game_state.active.tryMove(game_state.board, -1, 0, 0);
      break ;
    case EV_RIGHT :
      if ( !game_state . paused )
        game_state.active.tryMove(game_state.board, 1, 0, 0);
      break ;
    case EV_ROT_LEFT :
      if ( !game_state . paused )
        game_state.active.tryMove(game_state.board, 0, 0, -1);
      break ;
    case EV_ROT_RIGHT :
      if ( !game_state . paused )
        game_state.active.tryMove(game_state.board, 0, 0, 1);
      break ;
    case EV_DOWN :
      // Move down until failure
      if ( !game_state . paused )
      {
        while(game_state.active.tryMove(game_state.board, 0, -1, 0));
        game_state.active.place(game_state.board);
      }
      break ; 
    default :
      rt_printf ( "unknown event %d\n" , event ) ;    
  }
  return false ;
}


///////////////////////////////////////////////////////////////////////////////
/// \brief process all events and update game board state for each one
/// \return true on success
///
bool GameController :: processTick ()
{
  bool result = true ;
  int event = 0 ;
  pthread_mutex_lock ( &output_lock ) ;

  while ( mq_receive ( input_queue
                      , ( char* ) &event
                      , sizeof ( event )
                      , NULL ) != -1)
  {
    processEvent ( event ) ;
  }

  if ( game_state . paused || game_state . game_over )
    goto CLEANUP ;

  ++tick_count ;
  if ( !full_lines . empty () )
  {
    processFullLines () ;
    goto CLEANUP ;
  }
    
  if ( tick_count > ticks_til_drop )
  {
    downTick () ;
    tick_count = 0 ;
  }

CLEANUP:
  pthread_mutex_unlock ( &output_lock ) ;
  return result ;
}



///////////////////////////////////////////////////////////////////////////////
/// \brief function to be called every 60 Hz.
/// \return always NULL
///
void* GameController :: periodicFunc ( void* in_thread_obj )
{
  GameController* object = ( GameController* ) in_thread_obj ;
  object -> processTick () ;

  return NULL ;
}



///////////////////////////////////////////////////////////////////////////////
/// \brief thread loop for systems without periodic timers. Calls periodicFunc
///  and waits 16.66 milliseconds
/// \return will never return
///
void* GameController :: threadFunc ( void* in_thread_obj )
{
  #ifndef NOXENOMAI
  	pthread_make_periodic_np ( pthread_self () , gethrtime () , 16666666 ) ;
  #endif

  while ( 1 )
  {
    GameController :: periodicFunc ( in_thread_obj ) ;
    #ifdef NOXENOMAI
      usleep ( 16666 ) ;
    #else
      pthread_wait_np () ;
    #endif
  }

  return NULL ;
}




