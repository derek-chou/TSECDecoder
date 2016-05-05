//
//  redis-thread.c
//  TSECDecoder
//
//  Created by Derek Chou on 2016/4/13.
//  Copyright © 2016年 Derek Chou. All rights reserved.
//

#include "redis-thread.h"
#include <unistd.h>
#include "hiredis.h"
#include "decode-thread.h"

queue_t *g_redis_queue;
uv_rwlock_t g_redis_rwlock;
uint8_t redis_atomic;

void redis_thread(void *arg) {
  redisContext *context = redisConnect("127.0.0.1", 6379);
  redisReply *reply= redisCommand(context, "AUTH %s", "apex@tw");
  if (reply == NULL || reply->type == REDIS_REPLY_ERROR) {
    fprintf(stderr, "redis connect fail\n");
  }
  
  if (reply != NULL)
    freeReplyObject(reply);

  while (1) {
    while (queue_count(g_redis_queue) > 0) {
      uv_rwlock_wrlock(&g_redis_rwlock);
      msg_packet *mp = queue_pop_right(g_redis_queue);
      uv_rwlock_wrunlock(&g_redis_rwlock);
    
      if (mp != NULL) {
        char key[100] = {0x00};
        snprintf(key, 100, "%s@%02x", mp->prod_id, mp->type);
        redisReply *reply = NULL;
        if (mp->type & MARKET_TYPE_STOCK_TICK) {
          reply = redisCommand(context, "LPUSH %s %s", mp->prod_id, mp->msg);
        } else if (mp->type & MARKET_TYPE_STOCK_BASIC) {
          reply = redisCommand(context, "SET %s %s", key, mp->msg);
        }
        if (reply)
          freeReplyObject(reply);
        
        reply = redisCommand(context, "PUBLISH %s %s", key, mp->msg);
        freeReplyObject(reply);
        
        free(mp);
      } else {
//        printf("mp is NULL\n");
//        usleep(200);
      }
    }
    usleep(100);
  } //while 1
}

