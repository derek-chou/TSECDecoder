//
//  tcp-client.c
//  TSECDecoder
//
//  Created by Derek Chou on 2016/4/13.
//  Copyright © 2016年 Derek Chou. All rights reserved.
//

#include <assert.h>
#include "tcp-client.h"
#include "decode-thread.h"
#include "redis-thread.h"
#include <unistd.h>
#include "log.h"
#include "common.h"

uv_tcp_t tcp_handle;
char read_buf[MAX_PACKET_SIZE];
uv_loop_t* loop;
uv_connect_t connect_req;
uint64_t g_receive_count = 0;

void read_cb(uv_stream_t *server, ssize_t nread, uv_buf_t buf);

void alloc_callback(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
  buf->base = read_buf;
  buf->len = MAX_PACKET_SIZE;
}

void connect_cb(uv_connect_t* req, int status) {
  if (status != 0) {
    LOG_ERROR(g_log, "tcp server connect fail!!");
    usleep(1000 * 1000);
    connect_to_server();
//    return;
  } else {
    LOG_INFO(g_log, "tcp server connected");
    
    if (uv_read_start((uv_stream_t*) &tcp_handle, (uv_alloc_cb) alloc_callback,
                      (uv_read_cb) read_cb) != 0) {
      LOG_ERROR(g_log, "uv_read_start fail!!");
      usleep(1000 * 1000);
      connect_to_server();
    }
  }
}

void connect_to_server() {
  struct sockaddr_in addr;

  loop = uv_default_loop();
  
  //if (uv_ip4_addr("192.168.12.208", 2100, &addr) != 0)
  if (uv_ip4_addr(g_server_ip, g_server_port, &addr) != 0)
    LOG_ERROR(g_log, "uv_ip4_addr fail");
  
  if (uv_tcp_init(loop, &tcp_handle) != 0)
    LOG_ERROR(g_log, "uv_tcp_init fail");
  
  if (uv_tcp_connect(&connect_req, &tcp_handle, (const struct sockaddr*) &addr,
                     connect_cb) != 0) {
    LOG_ERROR(g_log, "uv_tcp_connect fail");
    usleep(1000 * 1000);
    connect_to_server();
  }
}

void read_cb(uv_stream_t *server, ssize_t nread, uv_buf_t buf) {
  if (nread < 0) {
    LOG_ERROR(g_log, "tcp read error");
    connect_to_server();
  }
  
  if (nread > 0) {
    uv_rwlock_wrlock(&g_rwlock);
    ringbuf_memcpy_into(g_ringbuf, read_buf, nread);
    uv_rwlock_wrunlock(&g_rwlock);
    
    //    if (nread == MAX_PACKET_SIZE)
    //      printf("too slow\n");
    g_receive_count += nread;
  }
  
}
