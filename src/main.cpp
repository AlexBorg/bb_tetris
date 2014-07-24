#include <iostream>
#include <sys/time.h>
#include <fstream>
#include <string>
#include <unistd.h>

#ifndef NOXENOMAI
#include <rtdk.h>
#endif

#include "InputHandler.hpp"
#include "DisplayHandler.hpp"

//#include <posix.h>
//#include <native/task.h>
//#include <native/queue.h>

#define TASK_PRIO  99 /* Highest RT priority */
#define TASK_MODE  0  /* No flags */
#define TASK_STKSZ 0  /* Stack size (use default one) */

using namespace std ;

int main () {
#ifndef NOXENOMAI
  rt_print_auto_init ( 1 ) ;
#endif

  InputHandler input ;
  input . start () ;

  //while ( 1 )
    //sleep ( 1000 ) ;

  DisplayHandler();
  return 0 ;
}
