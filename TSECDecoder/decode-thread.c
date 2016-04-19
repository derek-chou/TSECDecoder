//
//  decode-thread.c
//  TSECDecoder
//
//  Created by Derek Chou on 2016/4/11.
//  Copyright © 2016年 Derek Chou. All rights reserved.
//

#include <string.h>
#include <stdlib.h>
#include "decode-thread.h"
#include "ringbuf.h"
#include <unistd.h>
#include "redis-thread.h"
#include "tsec-decoder.h"
#include "fur-decoder.h"

ringbuf_t g_ringbuf;
uv_rwlock_t g_rwlock;
uint8_t decode_atomic;

void find_tail_pos(ringbuf_t rb, size_t *tail1, size_t *tail2) {
  *tail1 = ringbuf_findchr(rb, PROTOCOL_TAIL_1, 0);
  *tail2 = ringbuf_findchr(rb, PROTOCOL_TAIL_2, 0);
  size_t rb_used = ringbuf_bytes_used(g_ringbuf);
  
  while (*tail1 != rb_used && *tail2 != rb_used && *tail2 - *tail1 != 1) {
    *tail1 = ringbuf_findchr(rb, PROTOCOL_TAIL_1, *tail1+1);
    *tail2 = ringbuf_findchr(rb, PROTOCOL_TAIL_2, *tail1+1);
  }
}

void decode_thread(void *arg) {
  size_t rb_used, head_pos, tail_pos1, tail_pos2;
  
  while (1) {
    rb_used = ringbuf_bytes_used(g_ringbuf);
    if (rb_used == 0) {
      usleep(200);
      continue;
    }
    
    head_pos = ringbuf_findchr(g_ringbuf, PROTOCOL_HEAD, 0);
    find_tail_pos(g_ringbuf, &tail_pos1, &tail_pos2);
    while (head_pos != rb_used && tail_pos1 != rb_used &&
           tail_pos2 != rb_used && tail_pos2 - tail_pos1 == 1) {
      if (head_pos > 0) {
        printf("drop garbage\n");
        uint8_t *garbage_buf = malloc(head_pos+1);
        uv_rwlock_rdlock(&g_rwlock);
        ringbuf_memcpy_from(garbage_buf, g_ringbuf, head_pos);
        uv_rwlock_rdunlock(&g_rwlock);
        free(garbage_buf);

        break;
      }
      
      uint8_t *buf = malloc(tail_pos2+2); //include head & tail
      int *ret = 0;
      uv_rwlock_rdlock(&g_rwlock);
      ret = ringbuf_memcpy_from(buf, g_ringbuf, tail_pos2+1);
      uv_rwlock_rdunlock(&g_rwlock);
      
      
      if (buf[4] == 6) {
        decode_format_6(buf, tail_pos2);
      } else if (buf[1] == '1' && buf[2] == '1') {
        //I010 Last Price
        decode_I010(buf, tail_pos2);
      } else if (buf[1] == '2' && buf[2] == '1') {
        //I020 Tick
        decode_I020(buf, tail_pos2);
      } else if (buf[1] == '2' && buf[2] == '2') {
        //I080 Ask/Bid
        decode_I080(buf, tail_pos2);
      }
      
      free(buf);
      
      rb_used = ringbuf_bytes_used(g_ringbuf);
      head_pos = ringbuf_findchr(g_ringbuf, PROTOCOL_HEAD, 0);
      find_tail_pos(g_ringbuf, &tail_pos1, &tail_pos2);
    } //while (rb_used > 0 && start_pos != rb_used...
    usleep(100);
  } //while 1
}

