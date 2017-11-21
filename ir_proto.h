#ifndef IR_PROTO_H
#define IR_PROTO_H

#include <stdbool.h>
#include <stdint.h>

typedef enum ir_proto_type {
    IR_PROTO_SAMSUNG = 0,
    IR_PROTO_SIRC_12 = 1,
    IR_PROTO_NEC = 2,
    
    // Meta protocols for iterating over the enum.
    IR_PROTO_LAST,
    IR_PROTO_FIRST = IR_PROTO_SAMSUNG,
    IR_PROTO_UNKNOWN = IR_PROTO_LAST
} ir_proto_type;

typedef struct ir_proto {
    ir_proto_type type;
    uint8_t data[3];
} ir_proto;

typedef struct ir_proto_samsung {
    ir_proto_type type;
    uint8_t custom;
    uint8_t data;
    uint8_t pad[1];
} ir_proto_samsung;

typedef struct ir_proto_sirc_12 {
    ir_proto_type type;
    uint8_t command;
    uint8_t address;
    uint8_t pad[1];
} ir_proto_sirc_12;

// Supports both regular and extended NEC.
// For regular NEC, address_high is address_bar.
typedef struct ir_proto_nec {
    ir_proto_type type;
    uint8_t address;
    uint8_t address_high;
    uint8_t command;
} ir_proto_nec;

typedef bool (*ir_decoder_fn)(ir_proto *, const uint8_t *, unsigned long);
typedef bool (*ir_encoder_fn)(const ir_proto *);

void ir_carrier(unsigned long duty_on,
                unsigned long duty_total,
                unsigned long num_cycles);

bool ir_proto_decode(ir_proto * proto,
                     const uint8_t * buffer,
                     unsigned long buffer_sz);

bool ir_proto_encode(const ir_proto * proto);

unsigned long decode_sequence(const uint8_t * buffer,
                              unsigned long buffer_sz,
                              unsigned long offset,
                              unsigned char expect);

bool ir_proto_decode_samsung(ir_proto * proto,
                             const uint8_t * buffer,
                             unsigned long buffer_sz);

bool ir_proto_decode_sirc_12(ir_proto * proto,
                             const uint8_t * buffer,
                             unsigned long buffer_sz);

bool ir_proto_decode_nec(ir_proto * proto,
                         const uint8_t * buffer,
                         unsigned long buffer_sz);

bool ir_proto_encode_samsung(const ir_proto * proto);

bool ir_proto_encode_sirc_12(const ir_proto * proto);

bool ir_proto_encode_nec(const ir_proto * proto);

#endif
