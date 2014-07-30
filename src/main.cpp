#include <iostream>
#include <sys/time.h>
#include <fstream>
#include <string>
#include <unistd.h>
#include <libgen.h>

#ifndef NOXENOMAI
#include <rtdk.h>
#endif

#include "InputHandler.hpp"
#include "GameController.hpp"
#include "DisplayHandler.hpp"

//#include <posix.h>
//#include <native/task.h>
//#include <native/queue.h>

#define TASK_PRIO  99 /* Highest RT priority */
#define TASK_MODE  0  /* No flags */
#define TASK_STKSZ 0  /* Stack size (use default one) */

using namespace std ;

int main () {

  // Attempt to change into the directory with the executable to ensure access to resources
  char exe_path[1024];
  ssize_t exe_path_len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);

  if(exe_path_len >= 0) {
    exe_path[exe_path_len] = '\0';
    chdir(dirname(exe_path));
  }

#ifndef NOXENOMAI
  rt_print_auto_init ( 1 ) ;
#endif

  // Kick off input and controller threads
  InputHandler input ;
  input . start () ;

  GameController controller;
  controller.start();

  // Run display loop in main thread
  DisplayHandler(controller);
  return 0;
}
