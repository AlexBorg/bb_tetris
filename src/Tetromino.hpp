#ifndef TETROMINO_H
#define TETROMINO_H

#include "BBTdefines.hpp"

class Tetromino {
public:
  static const int num_rotations = 4;
  static const int width = 4;
  static const int height = 4;

private:
  static int values[BlockData::num_colors][height][num_rotations][width];
  static bool getValue(int block, int rotation, int x, int y);

  BlockData block_data;

public:
  int pos_x, pos_y;
  int pos_rotation;

  Tetromino();

  BlockData getBlock(int x, int y);
  void move(int dx, int dy, int dr = 0);
  void reinitialize();
  bool wouldIntersect(BoardState & board, int dx, int dy, int dr);
  bool tryMove(BoardState & board, int dx, int dy, int dr);
  void place(BoardState & board);
};

#endif
