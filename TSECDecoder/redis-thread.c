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

queue_t *g_redis_queue;
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
    while (__sync_fetch_and_add(&redis_atomic, 0) > 0) {
      msg_packet *mp = queue_pop_right(g_redis_queue);
      __sync_fetch_and_add(&redis_atomic, -1);
      
      if (mp != NULL) {
        char key[100] = {0x00};
        snprintf(key, 100, "%s@%02x", mp->prod_id, mp->type);
        redisReply *reply;
        reply = redisCommand(context, "LPUSH %s %s", key, mp->msg);
        freeReplyObject(reply);
        reply = redisCommand(context, "PUBLISH %s %s", key, mp->msg);
        freeReplyObject(reply);
        
//        printf("%s %c %s\n", mp->prod_id, mp->type, mp->msg);
        free(mp);
      } else {
//        printf("mp is NULL\n");
//        usleep(200);
      }
      break;
    }
    usleep(100);
  } //while 1
}

