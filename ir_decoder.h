#ifndef DECODER_H_
#define DECODER_H_

#include "ir_proto.h"

int decode(const unsigned char * buffer,
           unsigned long buffer_sz,
           ir_proto * proto);

unsigned long decode_sequence(const unsigned char * buffer,
                              unsigned long buffer_sz,
                              unsigned long offset,
                              unsigned char expect);

int decode_samsung(const unsigned char * buffer,
                   unsigned long buffer_sz,
                   ir_proto * proto);

int decode_sirc_12(const unsigned char * buffer,
                   unsigned long buffer_sz,
                   ir_proto * proto);

#endif
