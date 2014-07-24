///////////////////////////////////////////////////////////////////////////////
/// \file define commonly used items for the entire project
///////////////////////////////////////////////////////////////////////////////

#ifndef BBT_DEFINES
#define BBT_DEFINES 1

// instead of passing queue structure into contructors, have the objects create
// the queue based on this name 
#define BBT_EVENT_QUEUE_NAME "/BBT_EVENT_INPUT_QUEUE"
#define BBT_EVENT_MSG_SIZE 4
#define BBT_EVENT_QUEUE_SIZE 32

#ifdef NOXENOMAI
#define rt_printf printf
#else
#include <rtdk.h>
#endif

////////////////////
// events from the InputHandler to the Processing Handler
enum bbtEvents {
    EV_NONE
  , EV_LEFT
  , EV_RIGHT
  , EV_ROT_LEFT
  , EV_ROT_RIGHT
  , EV_DOWN
  , EV_PAUSE } ;

typedef struct {
  int color;
} blockData ;

const int BOARD_WIDTH = 10;
const int BOARD_HEIGHT = 20;

class Tetromino {
public:
   static const int width = 4;
   static const int height = 4;

   blockData blocks[width][height]; // Grid containing the blockData for pieces
   int x, y;                        // Bottom-left position of 4x4 grid

   //void rotateLeft();
   //void rotateRight();
   //void randomize();
};

struct GameState {
   blockData board[BOARD_WIDTH][BOARD_HEIGHT];
   Tetromino active, next;
};

#endif //BBT_DEFINES
