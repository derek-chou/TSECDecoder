//
//  tsec-decoder.h
//  TSECDecoder
//
//  Created by Derek Chou on 2016/4/19.
//  Copyright © 2016年 Derek Chou. All rights reserved.
//

#ifndef tsec_decoder_h
#define tsec_decoder_h

#include <stdio.h>
#include <stdint.h>

void decode_format_6(uint8_t *buf, size_t size);
void decode_format_1(uint8_t *buf, size_t size);
void decode_format_3(uint8_t *buf, size_t size);

#endif /* tsec_decoder_h */
