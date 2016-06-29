//
//  common.h
//  TSECDecoder
//
//  Created by Derek Chou on 2016/4/11.
//  Copyright © 2016年 Derek Chou. All rights reserved.
//


#ifndef common_h
#define common_h

#include <sys/types.h>
#include <libconfig.h>

void get_now_string(char *buf, size_t size);
int get_now_sec();
void get_conf_string (char *key, char *value);
void get_conf_int (char *key, int *value);

extern char g_server_ip[20];
extern int g_server_port;
#endif /* common_h */
