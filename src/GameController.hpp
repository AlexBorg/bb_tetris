///////////////////////////////////////////////////////////////////////////////
/// \file define the GameController class
///////////////////////////////////////////////////////////////////////////////

#ifndef TETRIS_GAME_CONTROLLER_H
#define TETRIS_GAME_CONTROLLER_H 1


// external includes
#include <mqueue.h>

// local includes
#include "BBTdefines.hpp"

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
  pthread_t thread ;
  pthread_mutex_t output_lock ;
  struct GameState game_state ;
  
  bool pause () ;
  bool left () ;
  bool right () ;
  bool rotateLeft () ;
  bool rotateRight () ;
  bool down () ;
  
  bool downTick () ;
  bool processEvent ( int event ) ;
  bool processTick ( mqd_t input ) ;
  static void* thread_func ( void* in_thread_obj ) ;
  void* exec () ;
} ; 


#endif // TETRIS_GAME_CONTROLLER_H
