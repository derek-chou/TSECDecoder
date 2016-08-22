//
//  common.c
//  TSECDecoder
//
//  Created by Derek Chou on 2016/4/11.
//  Copyright © 2016年 Derek Chou. All rights reserved.
//

#include "common.h"
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include "log.h"
#include <stdlib.h>

log4c_category_t *g_log;

void get_now_string(char *buf, size_t size) {
  time_t now;
  struct tm *timeinfo;
  struct timeval tv;
  
  gettimeofday (&tv, NULL);
  time (&now);
  timeinfo = localtime (&now);
  snprintf(buf, size, "%04d/%02d/%02d %02d:%02d:%02d.%06d",
           timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday,
           (int)timeinfo->tm_hour, (int)timeinfo->tm_min, (int)timeinfo->tm_sec,
           (int)tv.tv_usec);
}

int get_now_sec() {
  struct tm *timeinfo;
  struct timeval tv;
  time_t now;
  char buf[64];
  
  gettimeofday (&tv, NULL);
  time (&now);
  timeinfo = localtime (&now);
  snprintf(buf, sizeof(buf), "%06d",
           (int)timeinfo->tm_hour * 3600 +
           (int)timeinfo->tm_min * 60 +
           (int)timeinfo->tm_sec);
  return atoi(buf);
}

void get_conf_string (char *key, char *value) {
  config_t *conf = &(config_t) {};
  config_init (conf);
  config_read_file (conf, "TSECDecoder.conf");
  char *tmp;
  if (config_lookup (conf, key) == NULL)
    return;
  config_lookup_string (conf, key, (const char **)&tmp);
  if (tmp == NULL)
    return;
  memcpy (value, tmp, strlen (tmp));
  config_destroy (conf);
}

void get_conf_int (char *key, int *value) {
  config_t *conf = &(config_t) {};
  config_init (conf);
  config_read_file (conf, "TSECDecoder.conf");
  config_lookup_int(conf, key, value);
  config_destroy (conf);
}
