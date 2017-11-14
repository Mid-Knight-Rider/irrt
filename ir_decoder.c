#include "ir_decoder.h"

int decode(const unsigned char * buffer,
           unsigned long buffer_sz,
           ir_proto * proto)
{
    if (0 == decode_samsung(buffer, buffer_sz, proto)) {
        return 0;
    } else if (0 == decode_sirc_12(buffer, buffer_sz, proto)) {
        return 0;
    } else {
        proto->type = IR_PROTO_UNKNOWN;
        return 1;
    }
}

/**
 * Returns the length of the sequence of 'expect' bits in 'buffer'
 * beginning at 'offset' and ending at 'buffer_sz' or the first ~'expect'
 * bit, whichever comes first.
 */
unsigned long decode_sequence(const unsigned char * buffer,
                              unsigned long buffer_sz,
                              unsigned long offset,
                              unsigned char expect)
{
    unsigned long i = 0;
    for (i = offset; i < buffer_sz; ++i) {
        if (expect != buffer[i]) {
            break;
        }
    }
    return (i - offset);
}
