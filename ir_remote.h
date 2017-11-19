#ifndef IR_REMOTE_H
#define IR_REMOTE_H

#include "ir_proto.h"
#include <stdint.h>
#include <stdbool.h>

#define IR_REMOTE_NAME_SIZE_MAX 16
#define IR_REMOTE_BUTTONS_MAX 16
#define IR_REMOTE_BUTTON_NAME_SIZE_MAX 8
#define IR_REMOTES_MAX 8

typedef struct ir_remote {
    char name[IR_REMOTE_NAME_SIZE_MAX];
    struct {
        char name[IR_REMOTE_BUTTON_NAME_SIZE_MAX];
        ir_proto proto;
    } buttons[IR_REMOTE_BUTTONS_MAX];
    uint8_t num_buttons;
    bool registered;
} ir_remote;

#endif
