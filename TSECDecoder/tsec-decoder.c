//
//  tsec-decoder.c
//  TSECDecoder
//
//  Created by Derek Chou on 2016/4/19.
//  Copyright © 2016年 Derek Chou. All rights reserved.
//

#include "tsec-decoder.h"
#include "redis-thread.h"

//格式六:集中市場普通股競價交易即時行情資訊
void decode_format_6(uint8_t *buf, size_t size) {
  if (size < 77)
    return;
  
  char msg[300] = {0x00};
  
  //流水序號
  char serial_no[9] = {0x00};
  snprintf(serial_no, 9, "%02x%02x%02x%02x#", buf[6], buf[7], buf[8], buf[9]);
  snprintf(&msg[strlen(msg)], 300-strlen(msg), "%02x%02x%02x%02x#", buf[6], buf[7], buf[8], buf[9]);
  
  //商品代號
  char prod_id[7] = {0x00};
  memcpy(prod_id, &buf[10], 6);
  char *find_ptr = strchr(prod_id, ' ');
  if (find_ptr != NULL)
    *find_ptr = 0x00;
  
  while (__sync_fetch_and_add(&redis_atomic, 1) <= 0) {
    msg_packet *mp = malloc(sizeof(msg_packet));
    mp->type = 'L';
    snprintf(mp->prod_id, 20, "%s", prod_id);
    snprintf(mp->serial_no, 8, "%s", serial_no);
    snprintf(mp->msg, 300, "%s", msg);
    
    queue_push_left(g_redis_queue, (void*)mp);
    break;
  }
}
