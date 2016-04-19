//
//  redis-thread.h
//  TSECDecoder
//
//  Created by Derek Chou on 2016/4/13.
//  Copyright © 2016年 Derek Chou. All rights reserved.
//

#ifndef redis_thread_h
#define redis_thread_h

#include <stdio.h>
#include "queue.h"

typedef struct {
  char prod_id[21];
  char serial_no[9];
  char type;
  char msg[300];
} msg_packet;

extern queue_t *g_redis_queue;
extern uint8_t redis_atomic;

void redis_thread(void *arg);

#endif /* redis_thread_h */
