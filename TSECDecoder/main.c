//
//  main.c
//  TSECDecoder
//
//  Created by Derek Chou on 2016/4/6.
//  Copyright © 2016年 Derek Chou. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ringbuf.h"
#include "common.h"
#include "decode-thread.h"
#include "tcp-client.h"
#include "redis-thread.h"

int main(int argc, const char * argv[]) {
  uv_thread_t decode_thread_id;
  uv_thread_t redis_thread_id;
  
  g_ringbuf = ringbuf_new(RINGBUF_SIZE - 1);
  g_redis_queue = queue_create();
  redis_atomic = 0;
  decode_atomic = 0;

  uv_rwlock_init(&g_rwlock);
  connect_to_server();
  uv_thread_create(&decode_thread_id, decode_thread, (void*)NULL);
  uv_thread_create(&redis_thread_id, redis_thread, (void*)NULL);
  uv_run(loop, UV_RUN_DEFAULT);
  
  uv_rwlock_destroy(&g_rwlock);
  
}
