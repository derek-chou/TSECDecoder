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
#include "log.h"
#include <stdlib.h>
#include "cJSON.h"

queue_t *g_redis_queue;
uv_rwlock_t g_redis_rwlock;
uint8_t redis_atomic;

void *execute_redis(redisContext *ctx, const char *format, ...) {
  va_list ap;
  void *reply = NULL;
  va_start(ap,format);
  reply = redisvCommand(ctx, format, ap);
  va_end(ap);
  if (reply) freeReplyObject(reply);
  return reply;
}

void update_quote(redisContext *ctx, msg_packet *mp) {
  cJSON *json;
  json = cJSON_Parse(mp->msg);
  if(json) {
    cJSON *json_find;
    json_find = cJSON_GetObjectItem(json, "make_price");
    if(json_find) {
      double make_price = json_find->valuedouble;
      
      redisReply *reply = NULL;
      double open_price, high_price, low_price;
      reply = redisCommand(ctx, "HMGET TW:Quote:%s.TW Open High Low", mp->prod_id);
//      for (int i = 0; i < reply->elements; i++)
//        printf( "reply %s\n", reply->element[i]->str);
      open_price = reply->element[0]->str ? atof(reply->element[0]->str) : 0;
      high_price = reply->element[1]->str ? atof(reply->element[1]->str) : 0;
      low_price = reply->element[2]->str ? atof(reply->element[2]->str) : 0;
      
      //開盤價
      if (open_price == 0)
        execute_redis(ctx, "HSET TW:Quote:%s.TW Open %.2f",
                             mp->prod_id, make_price);
      //最高價
      if (make_price > high_price || high_price == 0)
        execute_redis(ctx, "HSET TW:Quote:%s.TW High %.2f",
                             mp->prod_id, make_price);
      //最低價
      if (make_price < low_price || low_price == 0)
        execute_redis(ctx, "HSET TW:Quote:%s.TW Low %.2f",
                             mp->prod_id, make_price);
      //成交價
      execute_redis(ctx, "HSET TW:Quote:%s.TW Price %.2f",
                           mp->prod_id, make_price);
      //tick數量
      execute_redis(ctx, "HINCRBY TW:Quote:%s.TW PTRid 1", mp->prod_id);

    }
    cJSON_Delete(json);
  }

}

void redis_thread(void *arg) {
  redisContext *context = redisConnect("127.0.0.1", 6379);
  redisReply *reply= redisCommand(context, "AUTH %s", "apex@tw");
  if (reply == NULL || reply->type == REDIS_REPLY_ERROR) {
    LOG_ERROR(g_log, "redis connect fail!!");
    exit(-1);
//    return;
  } else
    LOG_INFO(g_log, "redis connected");
  
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
        
        if (mp->type & MARKET_TYPE_STOCK_TICK || mp->type & MARKET_TYPE_FUR_TICK) {
          execute_redis(context, "LPUSH %s %s", mp->prod_id, mp->msg);

          update_quote(context, mp);
        } else if (mp->type & MARKET_TYPE_STOCK_BASIC) {
          execute_redis(context, "SET %s %s", key, mp->msg);
        }
        
        
        execute_redis(context, "PUBLISH %s %s", key, mp->msg);
        
        free(mp);
      } else {
//        printf("mp is NULL\n");
//        usleep(200);
      }
    }
    usleep(100);
  } //while 1
}

