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
#include "uv.h"

#define MAX_INFO_LEN 1024

typedef struct {
  char prod_id[21];
  char serial_no[9];
  uint8_t type;
  char msg[MAX_INFO_LEN];
} msg_packet;

extern queue_t *g_redis_queue;
extern uint8_t redis_atomic;
extern uv_rwlock_t g_redis_rwlock;


void redis_thread(void *arg);

#endif /* redis_thread_h */
