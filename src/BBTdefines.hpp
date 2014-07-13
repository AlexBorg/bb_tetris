///////////////////////////////////////////////////////////////////////////////
/// \file define commonly used items for the entire project
///////////////////////////////////////////////////////////////////////////////

#ifndef BBT_DEFINES
#define BBT_DEFINES 1

// instead of passing queue structure into contructors, have the objects create
// the queue based on this name 
#define BBT_EVENT_QUEUE_NAME "BBT_EVENT_INPUT_QUEUE"
#define BBT_EVENT_MSG_SIZE 4
#define BBT_EVENT_QUEUE_SIZE 300

////////////////////
// events from the InputHandler to the Processing Handler
enum bbtEvents {
    EV_NONE
  , EV_LEFT
  , EV_RIGHT
  , EV_ROT_LEFT
  , EV_ROT_RIGHT } ;

typedef struct {
  // TODO: Borg: if we store this struct in a 2D array, do we even need xy? 
  int x ;
  int y ;
  // TODO: Borg: can we replace colors with color def enumerations?
  int r ;
  int b ;
  int g ;
} blockData ;


#endif //BBT_DEFINES
