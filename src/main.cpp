#include <iostream>
#include <sys/time.h>
#include <fstream>
#include <string>
#include <unistd.h>

//#include <posix.h>
//#include <native/task.h>
//#include <native/queue.h>

#define TASK_PRIO  99 /* Highest RT priority */
#define TASK_MODE  0  /* No flags */
#define TASK_STKSZ 0  /* Stack size (use default one) */

using namespace std ;

int main () {
  cout << "hello world" << endl ;
  int err ;


  return 0 ;
}


