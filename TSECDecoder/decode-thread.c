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

void decode_I010(uint8_t *buf, size_t size) {
  if (size < 77)
    return;
  
  char msg[300] = {0x00};
  //資料時間
  snprintf(msg, 300, "%02x:%02x:%02x.%02x#", buf[3], buf[4], buf[5], buf[6]);
  
  //流水序號
  char serial_no[9] = {0x00};
  snprintf(serial_no, 9, "%02x%02x%02x%02x#", buf[7], buf[8], buf[9], buf[10]);
  snprintf(&msg[strlen(msg)], 300-strlen(msg), "%02x%02x%02x%02x#", buf[7], buf[8], buf[9], buf[10]);
  
  //商品代號
  char prod_id[11] = {0x00};
  memcpy(prod_id, &buf[14], 10);
  char *find_ptr = strchr(prod_id, ' ');
  if (find_ptr != NULL)
    *find_ptr = 0x00;
  
  //第一漲停價
  snprintf(&msg[strlen(msg)], 300-strlen(msg), "%02x%02x%02x%02x%02x#", buf[24], buf[25], buf[26],
           buf[27], buf[28]);

  //參考價
  snprintf(&msg[strlen(msg)], 300-strlen(msg), "%02x%02x%02x%02x%02x#", buf[29], buf[30], buf[31],
           buf[32], buf[33]);

  //第一跌停價
  snprintf(&msg[strlen(msg)], 300-strlen(msg), "%02x%02x%02x%02x%02x#", buf[34], buf[35], buf[36],
           buf[37], buf[38]);
  
  //第二漲停價
  snprintf(&msg[strlen(msg)], 300-strlen(msg), "%02x%02x%02x%02x%02x#", buf[39], buf[40], buf[41],
           buf[42], buf[43]);
  
  //第二跌停價
  snprintf(&msg[strlen(msg)], 300-strlen(msg), "%02x%02x%02x%02x%02x#", buf[44], buf[45], buf[46],
           buf[47], buf[48]);
  
  //第三漲停價
  snprintf(&msg[strlen(msg)], 300-strlen(msg), "%02x%02x%02x%02x%02x#", buf[49], buf[50], buf[51],
           buf[52], buf[53]);
  
  //第三跌停價
  snprintf(&msg[strlen(msg)], 300-strlen(msg), "%02x%02x%02x%02x%02x#", buf[54], buf[55], buf[56],
           buf[57], buf[58]);
  
  //契約種類
  snprintf(&msg[strlen(msg)], 300-strlen(msg), "%c#", buf[59]);
  
  //價格小數位
  snprintf(&msg[strlen(msg)], 300-strlen(msg), "%d#", buf[60]);
  
  //履約價小數位
  snprintf(&msg[strlen(msg)], 300-strlen(msg), "%d#", buf[61]);
  
  //上市日期
  snprintf(&msg[strlen(msg)], 300-strlen(msg), "%02x%02x%02x%02x#", buf[62], buf[63], buf[64], buf[65]);
  
  //下市日期
  snprintf(&msg[strlen(msg)], 300-strlen(msg), "%02x%02x%02x%02x#", buf[66], buf[67], buf[68], buf[69]);
  
  
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

void decode_I020(uint8_t *buf, size_t size) {
  if (size < 64)
    return;
  
  char msg[300] = {0x00};
  //資料時間
  snprintf(msg, 300, "%02x:%02x:%02x.%02x#", buf[3], buf[4], buf[5], buf[6]);
  
  //流水序號
  char serial_no[9] = {0x00};
  snprintf(serial_no, 9, "%02x%02x%02x%02x#", buf[7], buf[8], buf[9], buf[10]);
  snprintf(&msg[strlen(msg)], 300-strlen(msg), "%02x%02x%02x%02x#", buf[7], buf[8], buf[9], buf[10]);
  
  //商品代號
  char prod_id[21] = {0x00};
  memcpy(prod_id, &buf[14], 20);
  char *find_ptr = strchr(prod_id, ' ');
  if (find_ptr != NULL)
    *find_ptr = 0x00;
  
  //  snprintf(&msg[strlen(msg)], 300-strlen(msg), "%s#", prod_id);
  
  //成交時間
  snprintf(&msg[strlen(msg)], 300-strlen(msg), "%02x:%02x:%02x.%02x#", buf[34], buf[35], buf[36], buf[37]);
  
  //價格正('0')負('-')號
  //第一成交價
  snprintf(&msg[strlen(msg)], 300-strlen(msg), "%c%02x%02x%02x%02x%02x#", buf[38], buf[39], buf[40], buf[41], buf[42], buf[43]);
  
  //第一成交量
  //  snprintf(&msg[strlen(msg)], 300-strlen(msg), "%02x%02x%02x%02x#", buf[44], buf[45], buf[46], buf[47]);
  
  //成交資料註記
  int match_count = buf[48] & 0x7f;
  //        for (int cnt = 0; cnt < match_count; cnt++) {
  //          //價格正('0')負('-')號
  //          printf("\t%c\n", buf[49 + cnt*8]);
  //          //成交價
  //          printf("\t%02x%02x%02x%02x%02x\n", buf[50 + cnt*8], buf[51 + cnt*8], buf[52 + cnt*8], buf[53 + cnt*8], buf[54 + cnt*8]);
  //          //成交量
  //          printf("\t%02x%02x\n", buf[55 + cnt*8], buf[56 + cnt*8]);
  //        }
  
  //成交總量
  snprintf(&msg[strlen(msg)], 300-strlen(msg), "%02x%02x%02x%02x", buf[49 + match_count*8],
           buf[50 + match_count*8], buf[51 + match_count*8],
           buf[52 + match_count*8]);
  
  //買進總筆數
  //  snprintf(&msg[strlen(msg)], 300-strlen(msg), "%02x%02x%02x%02x#", buf[53 + match_count*8],
  //           buf[54 + match_count*8], buf[55 + match_count*8],
  //           buf[56 + match_count*8]);
  
  //賣出總筆數
  //  snprintf(&msg[strlen(msg)], 300-strlen(msg), "%02x%02x%02x%02x#", buf[57 + match_count*8],
  //           buf[58 + match_count*8], buf[59 + match_count*8],
  //           buf[60 + match_count*8]);
  
  while (__sync_fetch_and_add(&redis_atomic, 1) <= 0) {
    msg_packet *mp = malloc(sizeof(msg_packet));
    mp->type = 'T';
    snprintf(mp->prod_id, 20, "%s", prod_id);
    snprintf(mp->serial_no, 8, "%s", serial_no);
    snprintf(mp->msg, 300, "%s", msg);
    
    queue_push_left(g_redis_queue, (void*)mp);
    break;
  }
}

void decode_I080(uint8_t *buf, size_t size) {
  if (size < 155)
    return;
  
  char msg[300] = {0x00};
  //資料時間
  snprintf(msg, 300, "%02x:%02x:%02x.%02x#", buf[3], buf[4], buf[5], buf[6]);
  
  //流水序號
  char serial_no[9] = {0x00};
  snprintf(serial_no, 9, "%02x%02x%02x%02x#", buf[7], buf[8], buf[9], buf[10]);
  snprintf(&msg[strlen(msg)], 300-strlen(msg), "%02x%02x%02x%02x#", buf[7], buf[8], buf[9], buf[10]);
  
  //商品代號
  char prod_id[21] = {0x00};
  memcpy(prod_id, &buf[14], 20);
  char *find_ptr = strchr(prod_id, ' ');
  if (find_ptr != NULL)
    *find_ptr = 0x00;
  
  //買進五檔
  for(int i = 0; i < 5; i++) {
    int offset = 34 + i * 10;
    snprintf(&msg[strlen(msg)], 300-strlen(msg), "%c%02x%02x%02x%02x%02x#",
             buf[offset], buf[offset + 1], buf[offset + 2], buf[offset + 3], buf[offset + 4],
             buf[offset + 5]);
    snprintf(&msg[strlen(msg)], 300-strlen(msg), "%02x%02x%02x%02x#",
             buf[offset + 6], buf[offset + 7], buf[offset + 8], buf[offset + 9]);
  }
  
  //賣出五檔
  for(int i = 0; i < 5; i++) {
    int offset = 84 + i * 10;
    snprintf(&msg[strlen(msg)], 300-strlen(msg), "%c%02x%02x%02x%02x%02x#",
             buf[offset], buf[offset + 1], buf[offset + 2], buf[offset + 3], buf[offset + 4],
             buf[offset + 5]);
    snprintf(&msg[strlen(msg)], 300-strlen(msg), "%02x%02x%02x%02x#",
             buf[offset + 6], buf[offset + 7], buf[offset + 8], buf[offset + 9]);
  }
  
  while (__sync_fetch_and_add(&redis_atomic, 1) <= 0) {
    msg_packet *mp = malloc(sizeof(msg_packet));
    mp->type = 'A';
    snprintf(mp->prod_id, 20, "%s", prod_id);
    snprintf(mp->serial_no, 8, "%s", serial_no);
    snprintf(mp->msg, 300, "%s", msg);
    
    queue_push_left(g_redis_queue, (void*)mp);
    break;
  }
}
