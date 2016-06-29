//
//  tsec-decoder.c
//  TSECDecoder
//
//  Created by Derek Chou on 2016/4/19.
//  Copyright © 2016年 Derek Chou. All rights reserved.
//

#include "tsec-decoder.h"
#include "redis-thread.h"
#include "decode-thread.h"
#include <iconv.h>
#include <assert.h>
#include "log.h"

//格式一：集中市場普通股個股基本資料
void decode_format_1(uint8_t *buf, size_t size) {
  if (size < 103)
    return;
  
  char msg[MAX_INFO_LEN] = {0x00};
  char tmp[32];

  snprintf(&msg[strlen(msg)], 300-strlen(msg), "{");
  
  //流水序號
  char serial_no[9] = {0x00};
  snprintf(serial_no, 9, "%02x%02x%02x%02x#", buf[6], buf[7], buf[8], buf[9]);
  snprintf(tmp, 32, "%02x%02x%02x%02x", buf[6], buf[7], buf[8], buf[9]);
  snprintf(&msg[strlen(msg)], MAX_INFO_LEN-strlen(msg), "\"seq\":%d,", atoi(tmp));
  
  //股票代號
  char prod_id[7] = {0x00};
  memcpy(prod_id, &buf[10], 6);
  char *find_ptr = strchr(prod_id, ' ');
  if (find_ptr != NULL)
    *find_ptr = 0x00;
  
  //股票中文簡稱
  char prod_name[17] = {0x00};
  memcpy(prod_name, &buf[16], 16);
    char *find_name_ptr = strchr(prod_name, ' ');
    if (find_name_ptr != NULL)
      *find_name_ptr = 0x00;
  
  size_t in_size = sizeof(prod_name);
  char out[2*in_size+1];
  size_t out_size = sizeof(out);
  char *in_ptr = prod_name;
  char *out_ptr = out;

  iconv_t cd = iconv_open("utf-8", "big5-2003");
  size_t iconv_ret = iconv(cd, &in_ptr, &in_size, &out_ptr, &out_size);
  if (iconv_ret != 0)
    LOG_ERROR(g_log, "iconv fail prod_id = %s", prod_id);
  iconv_close(cd);
  
  snprintf(&msg[strlen(msg)], MAX_INFO_LEN-strlen(msg), "\"chname\":\"%s\",", out);

  //今日參考價
  snprintf(tmp, 32, "%02x%02x.%02x", buf[39], buf[40], buf[41]);
  snprintf(&msg[strlen(msg)], MAX_INFO_LEN-strlen(msg), "\"refPrice\":%.2f}",
           atof(tmp));
//  snprintf(&msg[strlen(msg)-1], MAX_INFO_LEN-strlen(msg), "}");

  uv_rwlock_wrlock(&g_redis_rwlock);
  msg_packet *mp = malloc(sizeof(msg_packet));
  mp->type = MARKET_TYPE_STOCK_BASIC;
  snprintf(mp->prod_id, 20, "%s", prod_id);
  snprintf(mp->serial_no, 8, "%s", serial_no);
  snprintf(mp->msg, MAX_INFO_LEN, "%s", msg);
  
  queue_push_left(g_redis_queue, (void*)mp);
  uv_rwlock_wrunlock(&g_redis_rwlock);
}

//格式三：集中市場普通股競價交易指數統計資料
void decode_format_3(uint8_t *buf, size_t size) {
  if (size < 188)
    return;
  
  char msg[MAX_INFO_LEN] = {0x00};
  char tmp[32];
  
  snprintf(&msg[strlen(msg)], 300-strlen(msg), "{");
  
  //傳輸序號
  char serial_no[9] = {0x00};
  snprintf(serial_no, 9, "%02x%02x%02x%02x#", buf[6], buf[7], buf[8], buf[9]);
  snprintf(tmp, 32, "%02x%02x%02x%02x", buf[6], buf[7], buf[8], buf[9]);
  snprintf(&msg[strlen(msg)], MAX_INFO_LEN-strlen(msg), "\"seq\":%d,", atoi(tmp));

  //指數時間(hh:mm:ss)
  snprintf(&msg[strlen(msg)], MAX_INFO_LEN-strlen(msg), "\"time\":\"%02x:%02x:%02x\",",
           buf[10], buf[11], buf[12]);

  //加權指數
  snprintf(tmp, 32, "%02x%02x%02x.%02x", buf[14], buf[15], buf[16], buf[17]);
  snprintf(&msg[strlen(msg)], MAX_INFO_LEN-strlen(msg), "\"POW00\":%.2f,",
           atof(tmp));

  snprintf(&msg[strlen(msg)-1], MAX_INFO_LEN-strlen(msg), "}");
  
  uv_rwlock_wrlock(&g_redis_rwlock);
  msg_packet *mp = malloc(sizeof(msg_packet));
  mp->type = MARKET_TYPE_STOCK_TICK;
  snprintf(mp->prod_id, 20, "POW00");
  snprintf(mp->serial_no, 8, "%s", serial_no);
  snprintf(mp->msg, MAX_INFO_LEN, "%s", msg);
  
  queue_push_left(g_redis_queue, (void*)mp);
  uv_rwlock_wrunlock(&g_redis_rwlock);
}

//格式六:集中市場普通股競價交易即時行情資訊
void decode_format_6(uint8_t *buf, size_t size) {
  if (size < 32)
    return;
  
  char msg[MAX_INFO_LEN] = {0x00};
  char tmp[32];
  uint8_t info_type = 0;
  
  snprintf(&msg[strlen(msg)], 300-strlen(msg), "{");
  
  //流水序號
  char serial_no[9] = {0x00};
  snprintf(serial_no, 9, "%02x%02x%02x%02x#", buf[6], buf[7], buf[8], buf[9]);
  snprintf(tmp, 32, "%02x%02x%02x%02x", buf[6], buf[7], buf[8], buf[9]);
  snprintf(&msg[strlen(msg)], MAX_INFO_LEN-strlen(msg), "\"seq\":%d,", atoi(tmp));
  
  //股票代號
  char prod_id[7] = {0x00};
  memcpy(prod_id, &buf[10], 6);
  char *find_ptr = strchr(prod_id, ' ');
  if (find_ptr != NULL)
    *find_ptr = 0x00;
  
  //撮合時間
  snprintf(&msg[strlen(msg)], MAX_INFO_LEN-strlen(msg), "\"time\":\"%02x:%02x:%02x.%02x%02x%02x\",",
           buf[16], buf[17], buf[18], buf[19], buf[20], buf[21]);
  
  //揭示項目註記
  uint8_t show_flag = buf[22];
  //bit 0 是否僅揭示成交價量
  uint8_t only_show_make = show_flag & 0x01;
  //bit 1,2,3 賣出價量檔數
  uint8_t show_sell_count = (show_flag >> 1) & 0x07;
  //bit 4,5,6 買進價量檔數
  uint8_t show_buy_count = (show_flag >> 4) & 0x07;
  //bit 7 有無成交價量
  uint8_t have_make = show_flag & 0x80;
  
  //漲跌停註記 buf[23]
  
  //狀態註記 buf[24]
  //bit 7 是否為試算揭示
  uint8_t is_fake_trade = buf[24] & 0x80;
  
  //累計成交數量
  snprintf(tmp, 32, "%02x%02x%02x%02x", buf[25], buf[26], buf[27], buf[28]);
  snprintf(&msg[strlen(msg)], MAX_INFO_LEN-strlen(msg), "\"total_amount\":%d,",
           atoi(tmp));

  //狀態為一般揭示時才取資料
  if (!is_fake_trade) {
    int offset = 29;
    
    //成交價量
    if (have_make) {
      snprintf(tmp, 32, "%02x%02x.%02x", buf[29], buf[30], buf[31]);
      snprintf(&msg[strlen(msg)], MAX_INFO_LEN-strlen(msg), "\"make_price\":%.2f,",
               atof(tmp));
      
      snprintf(tmp, 32, "%02x%02x%02x%02x", buf[32], buf[33], buf[34], buf[35]);
      snprintf(&msg[strlen(msg)], MAX_INFO_LEN-strlen(msg), "\"make_amount\":%d,",
               atoi(tmp));
    
      offset += 7;
      info_type |= MARKET_TYPE_STOCK_TICK;
    }
    
    if (!only_show_make) {
      //最佳x檔買進價量
      if (show_buy_count > 0)
        snprintf(&msg[strlen(msg)], MAX_INFO_LEN-strlen(msg), "\"bid\":[");
      for (int i = 0; i < show_buy_count; i++) {
        snprintf(tmp, 32, "%02x%02x.%02x", buf[offset], buf[offset + 1], buf[offset + 2]);
        snprintf(&msg[strlen(msg)], MAX_INFO_LEN-strlen(msg), "{\"price\":%.2f,", atof(tmp));
        snprintf(tmp, 32, "%02x%02x%02x%02x",
                 buf[offset + 3], buf[offset + 4], buf[offset + 5], buf[offset + 6]);
        snprintf(&msg[strlen(msg)], MAX_INFO_LEN-strlen(msg), "\"amount\":%d}", atoi(tmp));
        if (i < show_buy_count -1)
          snprintf(&msg[strlen(msg)], MAX_INFO_LEN-strlen(msg), ",");
        
        offset += 7;
      }
      if (show_buy_count > 0)
        snprintf(&msg[strlen(msg)], MAX_INFO_LEN-strlen(msg), "],");

      //最佳x檔賣出價量
      if (show_sell_count > 0)
        snprintf(&msg[strlen(msg)], MAX_INFO_LEN-strlen(msg), "\"ask\":[");
      for (int i = 0; i < show_sell_count; i++) {
        snprintf(tmp, 32, "%02x%02x.%02x", buf[offset], buf[offset + 1], buf[offset + 2]);
        snprintf(&msg[strlen(msg)], MAX_INFO_LEN-strlen(msg), "{\"price\":%.2f,", atof(tmp));
        snprintf(tmp, 32, "%02x%02x%02x%02x",
                 buf[offset + 3], buf[offset + 4], buf[offset + 5], buf[offset + 6]);
        snprintf(&msg[strlen(msg)], MAX_INFO_LEN-strlen(msg), "\"amount\":%d}", atoi(tmp));
        if (i < show_sell_count -1)
          snprintf(&msg[strlen(msg)], MAX_INFO_LEN-strlen(msg), ",");
        
        offset += 7;
      }
      if (show_sell_count > 0)
        snprintf(&msg[strlen(msg)], MAX_INFO_LEN-strlen(msg), "],");

      info_type |= MARKET_TYPE_STOCK_ASK_BID;
    }
  }
  snprintf(&msg[strlen(msg)-1], MAX_INFO_LEN-strlen(msg), "}");

//  while (__sync_fetch_and_add(&redis_atomic, 1) <= 0) {
  uv_rwlock_wrlock(&g_redis_rwlock);
    msg_packet *mp = malloc(sizeof(msg_packet));
    mp->type = info_type;
    snprintf(mp->prod_id, 20, "%s", prod_id);
    snprintf(mp->serial_no, 8, "%s", serial_no);
    snprintf(mp->msg, MAX_INFO_LEN, "%s", msg);
    
    queue_push_left(g_redis_queue, (void*)mp);
//    break;
  uv_rwlock_wrunlock(&g_redis_rwlock);
//  }
}
