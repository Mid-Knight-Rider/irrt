#ifndef IR_PROTO_H
#define IR_PROTO_H

typedef enum ir_proto_type {
    IR_PROTO_UNKNOWN,
    IR_PROTO_SAMSUNG,
    IR_PROTO_SIRC_12
} ir_proto_type;

typedef struct ir_proto {
    ir_proto_type type;
    char data[2];
} ir_proto;

typedef struct ir_proto_samsung {
    ir_proto_type type;
    unsigned char custom;
    unsigned char data;
} ir_proto_samsung;

typedef struct ir_proto_sirc_12 {
    ir_proto_type type;
    unsigned char command;
    unsigned char address;
} ir_proto_sirc_12;

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
