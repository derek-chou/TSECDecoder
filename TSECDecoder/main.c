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
#include "log.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

char g_server_ip[20] = {0x00};
int g_server_port = 0;
char g_stock_type[20] = {0x00};

int main(int argc, const char * argv[]) {
  uv_thread_t decode_thread_id;
  uv_thread_t redis_thread_id;
  
  struct stat st = {0};
  if (stat("./log", &st) == -1) {
    mkdir("./log", 0700);
  }
  log4c_init();
  g_log = log4c_category_get ("log.std");
  
  get_conf_string("server_ip", g_server_ip);
  if (strcmp(g_server_ip, "") == 0) {
    LOG_ERROR(g_log, "server_ip isn't set");
    return -1;
  }
  LOG_INFO(g_log, "server_ip = %s", g_server_ip);
  
  get_conf_int("server_port", &g_server_port);
  if (g_server_port == 0) {
    LOG_ERROR(g_log, "server_port isn't set");
    return -1;
  }
  LOG_INFO(g_log, "server_port = %d", g_server_port);

  get_conf_string("stock_type", g_stock_type);
  if (strcmp(g_stock_type, "") == 0) {
    sprintf(g_stock_type, "stock");
    LOG_INFO(g_log, "stock_type isn't set. default is \"stock\"");
  }
  LOG_INFO(g_log, "stock_type = %s", g_stock_type);

  LOG_INFO(g_log, "system start\n");

  g_ringbuf = ringbuf_new(RINGBUF_SIZE - 1);
  g_redis_queue = queue_create();
  redis_atomic = 0;
  decode_atomic = 0;

  uv_rwlock_init(&g_rwlock);
  uv_rwlock_init(&g_redis_rwlock);
  connect_to_server();
  
  uv_thread_create(&decode_thread_id, decode_thread, (void*)g_stock_type);
  //uv_thread_create(&redis_thread_id, redis_thread, (void*)NULL);
  
  uv_run(loop, UV_RUN_DEFAULT);
  
  uv_rwlock_destroy(&g_rwlock);
  uv_rwlock_destroy(&g_redis_rwlock);
  log4c_fini();
}
