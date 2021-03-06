///////////////////////////////////////////////////////////////////////////////
// \file test InputHandler by printing the BBT events generated by keyboard
// or controller key presses

#include "InputHandler.hpp"
#include "BBTdefines.hpp"
#include "errno.h"

#include <stdio.h>

int main ( int argc , char** argv )
{
  printf ( "input test start\n" ) ;

#ifndef NOXENOMAI
  rt_print_auto_init ( 1 ) ;
#endif

  InputHandler in_hand ;
  mqd_t input = mq_open ( BBT_EVENT_QUEUE_NAME
                      , O_RDONLY ) ;

  if ( input < 0 )
  {
    printf ( "failed to open queue %d\n" , errno ) ;
    return 0 ;
  }

  in_hand . start () ;

  int msg = 0 ;
  while ( 1 )
  {
    size_t result = mq_receive ( input
                               , ( char* ) &msg
                               , sizeof ( msg )
                               , NULL ) ;
    if ( result < 1 )
      continue ;

    switch ( msg )
    {
      case EV_NONE :
        printf ( "received EV_NONE msg\n" ) ;
        break ;
      case EV_PAUSE :
        printf ( "received EV_PAUSE msg\n" ) ;
        break ;
      case EV_START_LEFT :
        printf ( "received EV_START_LEFT msg\n" ) ;
        break ;
      case EV_STOP_LEFT :
        printf ( "received EV_STOP_LEFT msg\n" ) ;
        break ;
      case EV_ROT_LEFT :
        printf ( "received EV_ROT_LEFT msg\n" ) ;
        break ;
      case EV_START_RIGHT :
        printf ( "received EV_START_RIGHT msg\n" ) ;
        break ;
      case EV_STOP_RIGHT :
        printf ( "received EV_STOP_RIGHT msg\n" ) ;
        break ;
      case EV_ROT_RIGHT :
        printf ( "received EV_ROT_RIGHT msg\n" ) ;
        break ;
      case EV_START_DOWN :
        printf ( "received EV_START_DOWN msg\n" ) ;
        break ;
      case EV_STOP_DOWN :
        printf ( "received EV_STOP_DOWN msg\n" ) ;
        break ;
    }

  }

}
