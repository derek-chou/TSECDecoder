//
//  tcp-client.h
//  TSECDecoder
//
//  Created by Derek Chou on 2016/4/13.
//  Copyright © 2016年 Derek Chou. All rights reserved.
//

#ifndef tcp_client_h
#define tcp_client_h

#include <stdio.h>
#include <uv.h>

extern uv_tcp_t tcp_handle;
extern uv_loop_t* loop;
extern uint64_t g_receive_count;


void connect_to_server();
void connect_cb(uv_connect_t* req, int status);

#endif /* tcp_client_h */
