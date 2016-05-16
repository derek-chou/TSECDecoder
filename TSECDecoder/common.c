//
//  common.c
//  TSECDecoder
//
//  Created by Derek Chou on 2016/4/11.
//  Copyright © 2016年 Derek Chou. All rights reserved.
//

#include "common.h"
#include <time.h>
#include <sys/time.h>
#include <stdio.h>

void get_now_string(char *buf, size_t size) {
  time_t now;
  struct tm *timeinfo;
  struct timeval tv;
  
  gettimeofday (&tv, NULL);
  time (&now);
  timeinfo = localtime (&now);
  snprintf(buf, size, "%04d/%02d/%02d %02d:%02d:%02d.%06d",
           timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday,
           (int)timeinfo->tm_hour, (int)timeinfo->tm_min, (int)timeinfo->tm_sec, (int)tv.tv_usec);
}
