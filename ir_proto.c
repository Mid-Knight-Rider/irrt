#include "common.h"
#include "ir_proto.h"

/**
 * An array of structures of information about each supported
 * IR protocol. The index into this array is an ir_proto_type
 * value.
 */
static struct {
    ir_decoder_fn decode;
    ir_encoder_fn encode;
} ir_protos[] = {
        { .decode = ir_proto_decode_samsung, .encode = 0 },
        { .decode = ir_proto_decode_sirc_12, .encode = 0 }
};

/**
 * A generic decoder that tries each supported decoder before
 * giving up. Returns true on success or false on failure. On success,
 * proto points to a valid protocol with decoded fields. On failure,
 * proto->type = IR_PROTO_UNKNOWN.
 */
bool ir_proto_decode(ir_proto * proto,
                    const uint8_t * buffer,
                    unsigned long buffer_sz)
{
    for (int i = IR_PROTO_FIRST; i < IR_PROTO_LAST; ++i) {
        if (ir_protos[i].decode(proto, buffer, buffer_sz)) {
            return true;
        }
    }
    proto->type = IR_PROTO_UNKNOWN;
    return false;
}

/**
 * Returns the length of the sequence of 'expect' bits in 'buffer'
 * beginning at 'offset' and ending at 'buffer_sz' or the first ~'expect'
 * bit, whichever comes first.
 */
unsigned long decode_sequence(const uint8_t * buffer,
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
