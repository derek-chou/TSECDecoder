//
//  decode-thread.h
//  TSECDecoder
//
//  Created by Derek Chou on 2016/4/11.
//  Copyright © 2016年 Derek Chou. All rights reserved.
//

#ifndef decode_thread_h
#define decode_thread_h

#include <stdio.h>
#include "ringbuf.h"
#include "uv.h"

#define MAX_PACKET_SIZE 2048
#define RINGBUF_SIZE MAX_PACKET_SIZE * 1024
#define PROTOCOL_HEAD 0x1B
#define PROTOCOL_TAIL_1 0x0D
#define PROTOCOL_TAIL_2 0x0A

extern ringbuf_t g_ringbuf;
extern uv_rwlock_t g_rwlock;
extern uint8_t decode_atomic;

void decode_thread(void *arg);

#endif /* decode_thread_h */
