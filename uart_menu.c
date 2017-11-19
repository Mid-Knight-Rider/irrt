#include "common.h"
#include "ir_remote.h"
#include "uart_menu.h"

extern ir_remote ir_remotes[];
extern bool recv_buffer_ready;
extern unsigned long recv_buffer_sz;
extern uint8_t recv_buffer[];

void uart_menu_main(void)
{
    UARTprintf(" -------IRRT-------\n");
    UARTprintf("| a) Add Remote    |\n");
    UARTprintf("| d) Delete Remote |\n");
    UARTprintf("| l) List Remotes  |\n");
    UARTprintf("| u) Use Remote    |\n");
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
    
}
