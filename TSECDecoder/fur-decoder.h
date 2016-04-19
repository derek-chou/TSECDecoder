//
//  fur-decoder.h
//  TSECDecoder
//
//  Created by Derek Chou on 2016/4/19.
//  Copyright © 2016年 Derek Chou. All rights reserved.
//

#ifndef fur_decoder_h
#define fur_decoder_h

#include <stdio.h>

void decode_I010(uint8_t *buf, size_t size);
void decode_I020(uint8_t *buf, size_t size);
void decode_I080(uint8_t *buf, size_t size);

#endif /* fur_decoder_h */
