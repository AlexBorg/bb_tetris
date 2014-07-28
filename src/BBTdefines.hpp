///////////////////////////////////////////////////////////////////////////////
/// \file define commonly used items for the entire project
///////////////////////////////////////////////////////////////////////////////

#ifndef BBT_DEFINES
#define BBT_DEFINES 1

#include <array>

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

class BlockData {
public:
  static const int num_colors = 7;

  int id;    // Unique identifier for this particular block piece
  int color; // Color of block (1 .. 7)
  
  BlockData() { BlockData(0, 0); };
  BlockData(int _id, int _color) : id(_id), color(_color) {};

  bool operator==(const BlockData & bd) {
    return id == bd.id && color == bd.color;
  }
};

const int BOARD_WIDTH = 10;
const int BOARD_HEIGHT = 20;
typedef std::array<std::array<BlockData, BOARD_HEIGHT>, BOARD_WIDTH> BoardState;

#endif //BBT_DEFINES
