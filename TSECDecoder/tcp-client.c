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

uv_tcp_t tcp_handle;
char read_buf[MAX_PACKET_SIZE];
uv_loop_t* loop;
uv_connect_t connect_req;

void read_cb(uv_stream_t *server, ssize_t nread, uv_buf_t buf);

void alloc_callback(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
  buf->base = read_buf;
  buf->len = MAX_PACKET_SIZE;
}

void connect_cb(uv_connect_t* req, int status) {
  assert(0 == status);
  assert(0 == uv_read_start((uv_stream_t*) &tcp_handle,
                            (uv_alloc_cb) alloc_callback,
                            (uv_read_cb) read_cb));
  printf("connected!!\n");
}

void connect_to_server() {
  struct sockaddr_in addr;

  loop = uv_default_loop();
  
  if (uv_ip4_addr("192.168.10.214", 2100, &addr) != 0)
    fprintf(stderr, "uv_ip4_addr fail");
  if (uv_tcp_init(loop, &tcp_handle) != 0)
    fprintf(stderr, "uv_tcp_init fail");
  if (uv_tcp_connect(&connect_req, &tcp_handle, (const struct sockaddr*) &addr, connect_cb) != 0)
    fprintf(stderr, "uv_tcp_connect fail");
}


void read_cb(uv_stream_t *server, ssize_t nread, uv_buf_t buf) {
  static int read_count = 0;

  if (nread < 0) {
    fprintf(stderr, "error read\n");
    connect_to_server();
  }
  
  static size_t max_used = 0;
  if (nread > 0) {
    if (read_count % 500 == 0) {
      //            printf("result: read %ld bytes.\n", nread);
      size_t used = ringbuf_bytes_used(g_ringbuf);
      if (used > max_used)
        max_used = used;
      
      printf("ringbuf used = %ld, max = %ld\n", used, max_used);
      printf("queue count = %ld\n", queue_count(g_redis_queue));
    }
    
    uv_rwlock_wrlock(&g_rwlock);
    ringbuf_memcpy_into(g_ringbuf, read_buf, nread);
    uv_rwlock_wrunlock(&g_rwlock);
    
    
    //    if (nread == MAX_PACKET_SIZE)
    //      printf("too slow\n");
  }
  
  read_count++;
}
