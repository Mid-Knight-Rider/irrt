#include "common.h"
#include "ir_remote.h"
#include "uart_menu.h"

extern ir_remote ir_remotes[IR_REMOTES_MAX];
extern bool recv_buffer_ready;
extern unsigned long recv_buffer_sz;
extern uint8_t recv_buffer[];
extern systick_settings_t systick_settings;
extern bool use_eeprom;

static bool strequ(const char * a, const char * b);

void uart_menu_main(void)
{
    UARTprintf(" -------IRRT-------\n");
    UARTprintf("| a) Add Remote    |\n");
    UARTprintf("| d) Delete Remote |\n");
    UARTprintf("| l) List Remotes  |\n");
    UARTprintf("| u) Use Remote    |\n");
    UARTprintf("| r) Reset EEPROM  |\n");
    UARTprintf(" ------------------\n");
    UARTprintf(">");
    unsigned char command = UARTgetc();
    UARTprintf("\n");
    switch (command) {
        case 'a':
            uart_menu_remote_add();
            break;
        case 'd':
            uart_menu_remote_delete();
            break;
        case 'l':
            uart_menu_remotes_list();
            break;
        case 'u':
            uart_menu_remote_use();
            break;
        case 'r':
            uart_menu_reset();
            break;
        default:
            UARTprintf("Invalid command.\n");
            break;
    }
}

void uart_menu_remote_add(void)
{
    ir_remote * remote = 0;
    for (unsigned long i = 0; i < IR_REMOTES_MAX; ++i) {
        if (!ir_remotes[i].registered) {
            remote = &(ir_remotes[i]);
            remote->registered = true;
            remote->name[0] = '\0';
            remote->num_buttons = 0;
            break;
        }
    }
    if (0 == remote) {
        UARTprintf("Out of memory. You must delete an existing remote first.\n");
    }
    int name_size = 0;
    while (name_size < 1 || name_size > IR_REMOTE_NAME_SIZE_MAX - 1) {
        UARTprintf("Name your new remote (1-%u characters).\n", IR_REMOTE_NAME_SIZE_MAX - 1);
        UARTprintf(">");
        name_size = UARTgets(remote->name, IR_REMOTE_NAME_SIZE_MAX);
    }
    UARTprintf("Named remote '%s'.\n", remote->name);
    UARTprintf("Great! Now let's add some buttons. Point your physical remote\n");
    UARTprintf("at the receiver and press a button to clone it. You may press 'q'\n");
    UARTprintf("at any time to stop.\n");
    bool add_buttons = true;
    while (add_buttons) {
        // Setup receiver.
        systick_settings.mode = SYSTICK_MODE_RX;
        recv_buffer_ready = false;
        recv_buffer_sz = 0;
        MAP_GPIOIntClear(GPIO_PORTE_BASE, GPIO_PIN_4);
        MAP_IntPendClear(INT_GPIOE);
        MAP_IntEnable(INT_GPIOE);
        // Wait until the receiver has captured data.
        UARTprintf("Waiting for a button press (press 'q' to quit)...\n");
        while (!recv_buffer_ready) {
            if (UARTCharsAvail(UART0_BASE) && 'q' == UARTCharGet(UART0_BASE)) {
                UARTprintf("Finished adding buttons.\n");
                MAP_IntDisable(INT_GPIOE);
                MAP_SysTickIntDisable();
                while (UARTCharsAvail(UART0_BASE)) {
                    (void) UARTCharGet(UART0_BASE);
                }
                // Save the remote to EEPROM.
                if (use_eeprom) {
                    if (0 != EEPROMProgram((uint32_t *) &ir_remotes, sizeof(uint32_t), sizeof(ir_remotes))) {
                        UARTprintf("Could not save remote to EEPROM.\n");
                    }
                }
                return;
            }
        }
        ir_proto proto;
        if (ir_proto_decode(&proto, recv_buffer, recv_buffer_sz)) {
            switch (proto.type) {
                case IR_PROTO_SAMSUNG: {
                    ir_proto_samsung * myproto = (ir_proto_samsung *) &proto;
                    UARTprintf("Got: Samsung: custom = 0x%02x, data = 0x%02x\n", myproto->custom, myproto->data);
                    break;
                }
                case IR_PROTO_SIRC_12: {
                    ir_proto_sirc_12 * myproto = (ir_proto_sirc_12 *) &proto;
                    UARTprintf("Got: Sony SIRC 12-bit: address = 0x%02x, command = 0x%02x\n", myproto->address, myproto->command);
                    break;
                }
                case IR_PROTO_NEC: {
                    ir_proto_nec * myproto = (ir_proto_nec *) &proto;
                    UARTprintf("Got: NEC: address = 0x%02x, address_high = 0x%02x, command = 0x%02x\n",
                            myproto->address, myproto->address_high, myproto->command);
                    break;
                }
                default: {
                    break;
                }
            }
            // Name the button.
            int btn_name_size = 0;
            UARTprintf("Name this button (1-%u characters, leave empty to discard).\n", IR_REMOTE_BUTTON_NAME_SIZE_MAX - 1);
            UARTprintf(">");
            btn_name_size = UARTgets(remote->buttons[remote->num_buttons].name, IR_REMOTE_BUTTON_NAME_SIZE_MAX);
            if (btn_name_size > 0) {
                UARTprintf("Named button '%s'.\n", remote->buttons[remote->num_buttons].name);
                remote->buttons[remote->num_buttons].proto = proto;
                remote->num_buttons++;
                
                if (remote->num_buttons > IR_REMOTE_BUTTONS_MAX - 1) {
                    UARTprintf("No more memory for additional buttons.\n");
                    add_buttons = false;
                }
            } else {
                UARTprintf("Discarding button...\n");
            }
        } else {
            UARTprintf("Got a button, but couldn't decode it.\n");
        }
    }
}

void uart_menu_remote_delete(void)
{
    char remote_name[IR_REMOTE_NAME_SIZE_MAX];
    UARTprintf("Enter the name of a remote to delete.\n");
    UARTprintf(">");
    UARTgets(remote_name, IR_REMOTE_NAME_SIZE_MAX);
    bool found_remote = false;
    unsigned long i = 0;
    for (i = 0; i < IR_REMOTES_MAX; ++i) {
        if (ir_remotes[i].registered && strequ(ir_remotes[i].name, remote_name)) {
            found_remote = true;
            break;
        }
    }
    if (!found_remote) {
        UARTprintf("There is no remote registered by that name.\n");
        return;
    }
    UARTprintf("Deleted remote '%s'.\n", ir_remotes[i].name);
    ir_remotes[i].registered = false;
    // Update EEPROM.
    if (use_eeprom) {
        if (0 != EEPROMProgram((uint32_t *) &ir_remotes, sizeof(uint32_t), sizeof(ir_remotes))) {
            UARTprintf("Could not remove remote from EEPROM.\n");
        }
    }
}

void uart_menu_remotes_list(void)
{
    UARTprintf("Listing your remotes...\n");
    bool found_remote = false;
    for (unsigned long i = 0; i < IR_REMOTES_MAX; ++i) {
        if (ir_remotes[i].registered) {
            found_remote = true;
            ir_remote * remote = &(ir_remotes[i]);
            UARTprintf("%s (%u buttons)\n", remote->name, remote->num_buttons);
        }
    }
    if (!found_remote) {
        UARTprintf("You don't have any remotes. Add one from the main menu.\n");
    }
}

void uart_menu_remote_use(void)
{
    char remote_name[IR_REMOTE_NAME_SIZE_MAX];
    UARTprintf("Enter the name of a remote to use.\n");
    UARTprintf(">");
    UARTgets(remote_name, IR_REMOTE_NAME_SIZE_MAX);
    bool found_remote = false;
    unsigned long i = 0;
    for (i = 0; i < IR_REMOTES_MAX; ++i) {
        if (ir_remotes[i].registered && strequ(ir_remotes[i].name, remote_name)) {
            found_remote = true;
            break;
        }
    }
    if (!found_remote) {
        UARTprintf("There is no remote registered by that name.\n");
        return;
    }
    unsigned char command;
    do {
        UARTprintf("Listing remote '%s' buttons\n", ir_remotes[i].name);
        for (unsigned j = 0; j < ir_remotes[i].num_buttons; ++j) {
            UARTprintf("%c) %s\n", ((char) (j + 97)), ir_remotes[i].buttons[j].name);
        }
        UARTprintf("Enter a button code to transmit (q to quit)...\n");
        UARTprintf(">");
        command = UARTgetc();
        unsigned char normalized_command = command - 97;
        if (normalized_command >= ir_remotes[i].num_buttons) {
            UARTprintf("Invalid button code. Check the list.\n");
        } else {
            UARTprintf("Transmitting button '%s'...\n", ir_remotes[i].buttons[normalized_command].name);
            ir_proto_encode(&(ir_remotes[i].buttons[normalized_command].proto));
        }
    } while ('q' != command);
}

void uart_menu_reset(void)
{
    char yorn[4];
    UARTprintf("Resetting will destroy all EEPROM. Continue?\n");
    UARTprintf("Type \"yes\" or \"no\".\n");
    UARTprintf(">");
    UARTgets(yorn, 4);
    if (strequ("yes", yorn)) {
        UARTprintf("Resetting EEPROM...\n");
        if (0 != EEPROMMassErase()) {
            UARTprintf("Failed to reset EEPROM.\n");
        } else {
            SysCtlReset();
        }
    } else {
        UARTprintf("Aborting... Did not reset.\n");
    }
}

static bool strequ(const char * a, const char * b)
{
    unsigned long i = 0;
    while (1) {
        if (a[i] == '\0') {
            return b[i] == '\0';
        } else if (b[i] == '\0') {
            return a[i] == '\0';
        } else if (a[i] != b[i]) {
            return false;
        }
        i++;
    }
}
