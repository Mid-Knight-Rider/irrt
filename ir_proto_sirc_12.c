#include "common.h"
#include "ir_proto.h"

/**
 * Sony SIRC 12-bits
 *
 * Frequency: 40 kHz
 * Duty Cycle: 1/4
 *
 * Packet:
 * 2.4 ms of modulation (0)
 * 0.6ms of silence (1)
 * 7-bit command (LSB first)
 * 5-bit address (LSB first)
 * TODO: potentially more bits of higher SIRC
 *
 * 0 Bit:
 * 0.6 ms of modulation (0)
 * 0.6 ms of silence (1)
 *
 * 1 Bit:
 * 1.2 ms of modulation (0)
 * 0.6 ms of silence (1)
 */

#define CYCLES_LEADER_MOD 91
#define CYCLES_LEADER_SPACE 23
#define CYCLES_BIT_PREFIX_ZERO 23
#define CYCLES_BIT_PREFIX_ONE 46
#define CYCLES_BIT_SUFFIX 23
#define CYCLES_DELTA 4
#define COMMAND_NUM_BITS 7
#define ADDRESS_NUM_BITS 5

static unsigned long decode_bits(const uint8_t * buffer,
                                 unsigned long buffer_sz,
                                 unsigned long offset,
                                 unsigned char * byte,
                                 unsigned char num_bits,
                                 unsigned char last_seq)
{
    *byte = 0;
    unsigned long sequence_length = 0;
    for (unsigned int i = 0; i < num_bits; ++i) {
        unsigned long zero_len = decode_sequence(buffer, buffer_sz, offset + sequence_length, 0);
        if (zero_len > CYCLES_BIT_PREFIX_ZERO - CYCLES_DELTA && zero_len < CYCLES_BIT_PREFIX_ZERO + CYCLES_DELTA) {
            // *byte |= (0 << i); // This is a NOP.
            sequence_length += zero_len;
        } else if (zero_len > CYCLES_BIT_PREFIX_ONE - CYCLES_DELTA && zero_len < CYCLES_BIT_PREFIX_ONE + CYCLES_DELTA) {
            *byte |= (1 << i);
            sequence_length += zero_len;
        } else {
            return 0;
        }
        
        unsigned long one_len = decode_sequence(buffer, buffer_sz, offset + sequence_length, 1);
        if (one_len < CYCLES_BIT_SUFFIX - CYCLES_DELTA) {
            return 0;
        } else if (one_len > CYCLES_BIT_SUFFIX + CYCLES_DELTA && !last_seq && num_bits != i + 1) {
            // Special case on last bit of last sequence since there's no space at the end of the packet.
            return 0;
        } else {
            sequence_length += one_len;
        }
    }
    return sequence_length;
}

bool ir_proto_decode_sirc_12(ir_proto * proto,
                             const uint8_t * buffer,
                             unsigned long buffer_sz)
{
    proto->type = IR_PROTO_SIRC_12;
    ir_proto_sirc_12 * myproto = (ir_proto_sirc_12 *) proto;
    myproto->command = 0;
    myproto->address = 0;

    unsigned long offset = 0;
    unsigned long sequence_length = 0;
    
    // Leader modulated.
    sequence_length = decode_sequence(buffer, buffer_sz, offset, 0);
    if (sequence_length < CYCLES_LEADER_MOD - CYCLES_DELTA || sequence_length > CYCLES_LEADER_MOD + CYCLES_DELTA) {
        return false;
    }
    
    offset += sequence_length;
    
    // Leader silent.
    sequence_length = decode_sequence(buffer, buffer_sz, offset, 1);
    if (sequence_length < CYCLES_LEADER_SPACE - CYCLES_DELTA || sequence_length > CYCLES_LEADER_SPACE + CYCLES_DELTA) {
        return false;
    }
    
    offset += sequence_length;
    
    // Command.
    sequence_length = decode_bits(buffer, buffer_sz, offset, &myproto->command, COMMAND_NUM_BITS, 0);
    if (0 == sequence_length) {
        return false;
    }
    
    offset += sequence_length;
    
    // Address.
    sequence_length = decode_bits(buffer, buffer_sz, offset, &myproto->address, ADDRESS_NUM_BITS, 1);
    if (0 == sequence_length) {
        return false;
    }
    
    return true;
}
