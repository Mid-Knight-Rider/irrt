#include "common.h"
#include "ir_proto.h"

/**
 * Samsung
 *
 * Frequency: 37.9 kHz
 * Duty Cycle: 1/3
 *
 * Packet:
 * 4.5 ms of modulation (0)
 * 4.5 ms of silence (1)
 * 8 bits of custom
 * 8 bits of custom (repeat)
 * 8 bits of data
 * 8 bits of data inversed
 * 0 bit to end packet
 *
 * 0 Bit:
 * 0.56 ms of modulation (0)
 * 0.56 ms of silence (1)
 *
 * 1 Bit:
 * 0.56 ms of modulation (0)
 * 1.69 ms of silence (1)
 */

#define CYCLES_LEADER 171
#define CYCLES_BIT_PREFIX 21
#define CYCLES_BIT_SUFFIX_ZERO 21
#define CYCLES_BIT_SUFFIX_ONE 64
#define CYCLES_DELTA 6
#define BITS_PER_BYTE 8

static unsigned long decode_byte(const uint8_t * buffer,
                                 unsigned long buffer_sz,
                                 unsigned long offset,
                                 unsigned char * byte)
{
    *byte = 0;
    unsigned long sequence_length = 0;
    for (unsigned int i = 0; i < BITS_PER_BYTE; ++i) {
        unsigned long zero_len = decode_sequence(buffer, buffer_sz, offset + sequence_length, 0);
        if (zero_len < CYCLES_BIT_PREFIX - CYCLES_DELTA || zero_len > CYCLES_BIT_PREFIX + CYCLES_DELTA) {
            return 0;
        }
        sequence_length += zero_len;
        unsigned long one_len = decode_sequence(buffer, buffer_sz, offset + sequence_length, 1);
        if (one_len > CYCLES_BIT_SUFFIX_ZERO - CYCLES_DELTA && one_len < CYCLES_BIT_SUFFIX_ZERO + CYCLES_DELTA) {
            // *byte |= (0 << i); // This is a NOP.
            sequence_length += one_len;
        } else if (one_len > CYCLES_BIT_SUFFIX_ONE - CYCLES_DELTA && one_len < CYCLES_BIT_SUFFIX_ONE + CYCLES_DELTA) {
            *byte |= (1 << i);
            sequence_length += one_len;
        } else {
            return 0;
        }
    }
    return sequence_length;
}

bool ir_proto_decode_samsung(ir_proto * proto,
                             const uint8_t * buffer,
                             unsigned long buffer_sz)
{
    proto->type = IR_PROTO_SAMSUNG;
    ir_proto_samsung * myproto = (ir_proto_samsung *) proto;
    myproto->custom = 0;
    myproto->data = 0;

    unsigned long offset = 0;
    unsigned long sequence_length = 0;
    
    // Leader modulated.
    sequence_length = decode_sequence(buffer, buffer_sz, offset, 0);
    if (sequence_length < CYCLES_LEADER - CYCLES_DELTA || sequence_length > CYCLES_LEADER + CYCLES_DELTA) {
        return false;
    }
    
    offset += sequence_length;
    
    // Leader silent.
    sequence_length = decode_sequence(buffer, buffer_sz, offset, 1);
    if (sequence_length < CYCLES_LEADER - CYCLES_DELTA || sequence_length > CYCLES_LEADER + CYCLES_DELTA) {
        return false;
    }
    
    offset += sequence_length;
    
    // Custom (first time).
    sequence_length = decode_byte(buffer, buffer_sz, offset, &myproto->custom);
    if (0 == sequence_length) {
        return false;
    }
    
    offset += sequence_length;
    
    // Custom (second time).
    unsigned char custom;
    sequence_length = decode_byte(buffer, buffer_sz, offset, &custom);
    if (0 == sequence_length || custom != myproto->custom) {
        return false;
    }
    
    offset += sequence_length;
    
    // Data.
    sequence_length = decode_byte(buffer, buffer_sz, offset, &myproto->data);
    if (0 == sequence_length) {
        return false;
    }
    
    offset += sequence_length;
    
    // Data.
    unsigned char data;
    sequence_length = decode_byte(buffer, buffer_sz, offset, &data);
    data = ~data;
    if (0 == sequence_length || data != myproto->data) {
        return false;
    }
    
    return true;
}
