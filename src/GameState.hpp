#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "BBTdefines.hpp"
#include "Tetromino.hpp"

class GameState {
public:
  BoardState board;
  Tetromino active, next;
  unsigned int score;
  unsigned int level;
  unsigned int lines_cleared;
  bool paused ;
  bool game_over ;

  GameState() { reset () ; }
  
  void reset ()
  {
    score = 0 ;
    level = 1 ;
    lines_cleared = 0 ;
    paused = true ;
    game_over = false ;
    
    for(unsigned int x = 0; x < board.size(); x++) {
      for(unsigned int y = 0; y < board[x].size(); y++) {
        board[x][y] = BlockData(0, 0);
      }
    }
  }
};

#endif
