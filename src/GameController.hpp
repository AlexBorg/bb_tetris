///////////////////////////////////////////////////////////////////////////////
/// \file define the GameController class
///////////////////////////////////////////////////////////////////////////////

#ifndef TETRIS_GAME_CONTROLLER_H
#define TETRIS_GAME_CONTROLLER_H 1


// external includes
#include <mqueue.h>
#include <vector>

// local includes
#include "BBTdefines.hpp"
#include "GameState.hpp"

///////////////////////////////////////////////////////////////////////////////
/// \class accepts input from the input handler and manages changes to the 
/// game board. It allows the display handler to pull game board data from 
/// this class.
///////////////////////////////////////////////////////////////////////////////
class GameController
{
public :
    GameController () ;
    ~GameController () ;
  void start () ;
  
  bool getGameState ( GameState &out_state ) ;
  

private :
  void reset () ; 
  int getFullLines () ;
  bool processFullLines () ;
  bool removeFullLines () ;
  
  pthread_t thread ;
  pthread_mutex_t output_lock ;
  mqd_t input_queue ;
                    
  struct GameState game_state ;
  unsigned int ticks_til_drop ;
  unsigned int tick_count ;
  std :: vector < unsigned int > full_lines ;
  
  
  bool pause () ;
  bool downTick () ;
  bool processEvent ( int event ) ;
  bool processTick () ;
  static void* periodicFunc ( void* in_thread_obj ) ;
  #ifdef NOXENOMAI
  static void* threadFunc ( void* in_thread_obj ) ;
  #else
  static void threadFunc ( void* in_thread_obj ) ;
  #endif
  void* exec () ;
} ; 


#endif // TETRIS_GAME_CONTROLLER_H

