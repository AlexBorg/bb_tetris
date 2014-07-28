#include <stdexcept>
#include "Tetromino.hpp"

static BlockData empty_block = BlockData(0, 0);

// All rotation states for all tetromino types
int Tetromino::values[BlockData::num_colors][height][num_rotations][width] = {
  {
    {{1,1,1,1}, {0,0,1,0}, {0,0,0,0}, {0,1,0,0}},
    {{0,0,0,0}, {0,0,1,0}, {0,0,0,0}, {0,1,0,0}},
    {{0,0,0,0}, {0,0,1,0}, {1,1,1,1}, {0,1,0,0}},
    {{0,0,0,0}, {0,0,1,0}, {0,0,0,0}, {0,1,0,0}},
  }, {
    {{1,1,1,0}, {0,1,0,0}, {0,0,0,0}, {0,1,0,0}},
    {{0,1,0,0}, {0,1,1,0}, {1,1,1,0}, {1,1,0,0}},
    {{0,0,0,0}, {0,1,0,0}, {0,1,0,0}, {0,1,0,0}},
    {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  }, {
    {{1,1,0,0}, {1,1,0,0}, {1,1,0,0}, {1,1,0,0}},
    {{1,1,0,0}, {1,1,0,0}, {1,1,0,0}, {1,1,0,0}},
    {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
    {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  }, {
    {{1,1,1,0}, {1,1,0,0}, {0,0,1,0}, {1,0,0,0}},
    {{1,0,0,0}, {0,1,0,0}, {1,1,1,0}, {1,0,0,0}},
    {{0,0,0,0}, {0,1,0,0}, {0,0,0,0}, {1,1,0,0}},
    {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  }, {
    {{1,0,0,0}, {1,1,0,0}, {1,1,1,0}, {0,1,0,0}},
    {{1,1,1,0}, {1,0,0,0}, {0,0,1,0}, {0,1,0,0}},
    {{0,0,0,0}, {1,0,0,0}, {0,0,0,0}, {1,1,0,0}},
    {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  }, {
    {{1,1,0,0}, {0,1,0,0}, {0,1,1,0}, {0,1,0,0}},
    {{0,1,1,0}, {1,1,0,0}, {1,1,0,0}, {1,1,0,0}},
    {{0,0,0,0}, {1,0,0,0}, {0,0,0,0}, {1,0,0,0}},
    {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  }, {
    {{0,1,1,0}, {1,0,0,0}, {0,1,1,0}, {1,0,0,0}},
    {{1,1,0,0}, {1,1,0,0}, {1,1,0,0}, {1,1,0,0}},
    {{0,0,0,0}, {0,1,0,0}, {0,0,0,0}, {0,1,0,0}},
    {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
  }
};

////////////////////////////////////////////////////////////////////////////////
Tetromino::Tetromino() {
  reinitialize();
}

////////////////////////////////////////////////////////////////////////////////
bool Tetromino::getValue(int color, int rotation, int x, int y) {
  if(rotation < 0 || rotation >= num_rotations)  throw std::out_of_range("rotation");
  if(x < 0 || x > width)                        throw std::out_of_range("x");
  if(y < 0 || y > height)                       throw std::out_of_range("y");

  if(color < 1 || color > BlockData::num_colors) return false;

  return values[color - 1][height - 1 - y][rotation][x] != 0;
}

////////////////////////////////////////////////////////////////////////////////
/// \brief Get BlockData for a particular subposition of this tetromino
BlockData Tetromino::getBlock(int x, int y) {
  return getValue(block_data.color, pos_rotation, x, y) ? block_data : empty_block;
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Move this piece
/// 
void Tetromino::move(int dx, int dy, int dr) {
  pos_x += dx;
  pos_y += dy;
  pos_rotation = (pos_rotation + dr + num_rotations) % num_rotations;
}

////////////////////////////////////////////////////////////////////////////////
/// \brief Reinitialize our block data to a new random block at rotation 0
void Tetromino::reinitialize() {
  static bool rand_initialized = false;

  if(!rand_initialized) {
    time_t t;
    srandom(time(&t));

    rand_initialized = true;
  }

  block_data.id = random();
  block_data.color = (block_data.id % BlockData::num_colors) + 1;

  pos_x = BOARD_WIDTH / 2;
  pos_y = BOARD_HEIGHT;
  pos_rotation = 0;
}

////////////////////////////////////////////////////////////////////////////////
/// \brief Determine if a movement would result in an intersection with a block
///        on the board.
bool Tetromino::wouldIntersect(BoardState & board, int dx, int dy, int dr) {
  for(int x = 0; x < width; x++) {
    for(int y = 0; y < height; y++) {
      int new_rot = (pos_rotation + dr + num_rotations) % num_rotations;
      bool on_block = getValue(block_data.color, new_rot, x, y);

      if(!on_block) continue;

      // Apply offset and check ranges
      int new_x = pos_x + dx + x;
      int new_y = pos_y + dy + y;

      if(new_x < 0 || new_x >= (signed)board.size()) return true;
      if(new_y < 0) return true;

      // Allow to live off of the top end of the board
      if(new_y >= (signed)board[new_x].size()) continue;

      bool on_board = board[new_x][new_y].color != 0;

      if(on_board) return true;
    }
  }
  
  return false;
}

////////////////////////////////////////////////////////////////////////////////
/// \brief If a move can be performed without intersecting, apply it
/// \returns True if a move was performed, False if blocked
bool Tetromino::tryMove(BoardState & board, int dx, int dy, int dr) {
  if(wouldIntersect(board, dx, dy, dr)) return false;

  move(dx, dy, dr);
  return true;
}

////////////////////////////////////////////////////////////////////////////////
/// \brief Place this tetromino on the board at its current location
void Tetromino::place(BoardState & board) {
  for(int x = 0; x < width; x++) {
    for(int y = 0; y < height; y++) {
      bool on_block = getValue(block_data.color, pos_rotation, x, y);

      if(!on_block) continue;

      int new_x = pos_x + x;
      int new_y = pos_y + y;
      if(new_x < 0 || new_x >= (signed)board.size()) continue;
      if(new_y < 0 || new_y >= (signed)board[new_x].size()) continue;

      board[new_x][new_y] = block_data;
    }
  }
}
