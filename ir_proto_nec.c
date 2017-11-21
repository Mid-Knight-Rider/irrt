#include "common.h"
#include "ir_proto.h"

/**
 * NEC
 *
 * Frequency: 38 kHz
 * Duty Cycle: 1/4
 *
 * Packet:
 * 9 ms of modulation (0)
 * 4.5 ms of silence
 * 8 bits of address (LSB first)
 * 8 bits of address inverse (LSB first)
 * 8 bits of command (LSB first)
 * 8 bits of command inverse (LSB first)
 *
 * 0 Bit:
 * 560 us ms of modulation (0)
 * 560 us of silence (1)
 *
 * 1 Bit:
 * 560 us of modulation (0)
 * 1.69 ms of silence (1)
 */

#define CYCLES_LEADER_MOD 342
#define CYCLES_LEADER_SPACE 171
#define CYCLES_BIT_PREFIX 21
#define CYCLES_BIT_SUFFIX_ZERO 21
#define CYCLES_BIT_SUFFIX_ONE 64
#define CYCLES_DELTA 8
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

bool ir_proto_decode_nec(ir_proto * proto,
                         const uint8_t * buffer,
                         unsigned long buffer_sz)
{
    proto->type = IR_PROTO_NEC;
    ir_proto_nec * myproto = (ir_proto_nec *) proto;
    myproto->address = 0;
    myproto->address_high = 0;
    myproto->command = 0;

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
    
    // Address.
    sequence_length = decode_byte(buffer, buffer_sz, offset, &myproto->address);
    if (0 == sequence_length) {
        return false;
    }
    
    offset += sequence_length;
    
    // Address High or Address Bar.
    sequence_length = decode_byte(buffer, buffer_sz, offset, &myproto->address_high);
    if (0 == sequence_length) {
        return false;
    }
    
    offset += sequence_length;
    
    // Command.
    sequence_length = decode_byte(buffer, buffer_sz, offset, &myproto->command);
    if (0 == sequence_length) {
        return false;
    }
    
    offset += sequence_length;
    
    // Command Bar.
    uint8_t command_bar = 0;
    sequence_length = decode_byte(buffer, buffer_sz, offset, &command_bar);
    if (0 == sequence_length) {
        return false;
    }
    command_bar = ~command_bar;
    if (command_bar != myproto->command) {
        return false;
    }
    
    return true;
}

bool ir_proto_encode_nec(const ir_proto * proto)
{
    const ir_proto_nec * myproto = (const ir_proto_nec *) proto;
    uint8_t command_bar = ~(myproto->command);
    ir_carrier(0x01, 4, CYCLES_LEADER_MOD);
    ir_carrier(0x00, 4, CYCLES_LEADER_SPACE);
    for (unsigned i = 0; i < BITS_PER_BYTE; ++i) {
        if (0 != ((myproto->address >> i) & 0x01)) {
            // One
            ir_carrier(0x01, 4, CYCLES_BIT_PREFIX);
            ir_carrier(0x00, 4, CYCLES_BIT_SUFFIX_ONE);
        } else {
            // Zero
            ir_carrier(0x01, 4, CYCLES_BIT_PREFIX);
            ir_carrier(0x00, 4, CYCLES_BIT_SUFFIX_ZERO);
        }
    }
    for (unsigned i = 0; i < BITS_PER_BYTE; ++i) {
        if (0 != ((myproto->address_high >> i) & 0x01)) {
            // One
            ir_carrier(0x01, 4, CYCLES_BIT_PREFIX);
            ir_carrier(0x00, 4, CYCLES_BIT_SUFFIX_ONE);
        } else {
            // Zero
            ir_carrier(0x01, 4, CYCLES_BIT_PREFIX);
            ir_carrier(0x00, 4, CYCLES_BIT_SUFFIX_ZERO);
        }
    }
    for (unsigned i = 0; i < BITS_PER_BYTE; ++i) {
        if (0 != ((myproto->command >> i) & 0x01)) {
            // One
            ir_carrier(0x01, 4, CYCLES_BIT_PREFIX);
            ir_carrier(0x00, 4, CYCLES_BIT_SUFFIX_ONE);
        } else {
            // Zero
            ir_carrier(0x01, 4, CYCLES_BIT_PREFIX);
            ir_carrier(0x00, 4, CYCLES_BIT_SUFFIX_ZERO);
        }
    }
    for (unsigned i = 0; i < BITS_PER_BYTE; ++i) {
        if (0 != ((command_bar >> i) & 0x01)) {
            // One
            ir_carrier(0x01, 4, CYCLES_BIT_PREFIX);
            ir_carrier(0x00, 4, CYCLES_BIT_SUFFIX_ONE);
        } else {
            // Zero
            ir_carrier(0x01, 4, CYCLES_BIT_PREFIX);
            ir_carrier(0x00, 4, CYCLES_BIT_SUFFIX_ZERO);
        }
    }
    // Zero to end packet.
    // Zero to end packet
    ir_carrier(0x01, 4, CYCLES_BIT_PREFIX);
    ir_carrier(0x00, 4, CYCLES_BIT_SUFFIX_ZERO);
    return true;
}
