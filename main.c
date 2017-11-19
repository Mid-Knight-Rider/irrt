#include "common.h"
#include "ir_proto.h"
#include "uart_menu.h"
#include "ir_remote.h"

#define RECV_BUFFER_SIZE 4096
uint8_t recv_buffer[RECV_BUFFER_SIZE];
unsigned long recv_buffer_sz = 0;
bool recv_buffer_ready = false;
ir_remote ir_remotes[IR_REMOTES_MAX];

void SysTick_Handler(void)
{
    uint8_t next_bit = (0 != MAP_GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_4));
    recv_buffer[recv_buffer_sz] = next_bit;
    recv_buffer_sz++;
    if (recv_buffer_sz >= RECV_BUFFER_SIZE) {
        MAP_SysTickIntDisable();
        recv_buffer_ready = true;
    }
}

void GPIOE_Handler(void)
{
    // Acknowledge interrupt.
    MAP_GPIOIntClear(GPIO_PORTE_BASE, GPIO_INT_PIN_4);
    // Disable IR trigger until IR decoding has finished.
    MAP_IntDisable(INT_GPIOE);
    // Enable SysTick to begin decoding IR signal.
    NVIC_ST_CURRENT_R = 0;
    MAP_SysTickIntEnable();
}

int main(void)
{
    bool loop = true;
    // Disable interrupts until we've initialized.
    MAP_IntMasterDisable();
    // Set clock to roughly 4.65 MHz.
    MAP_SysCtlClockSet(SYSCTL_SYSDIV_43 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);
    // Configure SysTick for 38 kHz frequency.
    MAP_SysTickPeriodSet((MAP_SysCtlClockGet() * 26) / 1000000);
    MAP_SysTickEnable();
    // Configure PE4 to be a negative-edge triggered input pin for IR decoding.
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    while (!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE));
    MAP_GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_4);
    MAP_GPIOIntTypeSet(GPIO_PORTE_BASE, GPIO_PIN_4, GPIO_FALLING_EDGE);
    MAP_GPIOIntClear(GPIO_PORTE_BASE, GPIO_INT_PIN_4);
    MAP_IntPrioritySet(INT_GPIOE, 0x07);
    MAP_GPIOIntEnable(GPIO_PORTE_BASE, GPIO_INT_PIN_4);
    // Configure PC5-PC6 as generic output pins.
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    while (!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOC));
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_5 | GPIO_PIN_6);
    // Configure UART.
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    while (!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    while (!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_UART0));
    MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    MAP_GPIOPinConfigure(GPIO_PA0_U0RX);
    MAP_GPIOPinConfigure(GPIO_PA1_U0TX);
    MAP_UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    UARTStdioConfig(0, 9600, 16000000);
    // Initialize all remotes.
    for (unsigned long i = 0; i < IR_REMOTES_MAX; ++i) {
        ir_remotes[i].registered = false;
    }
    // Enable interrupts globally.
    MAP_IntMasterEnable();
    // Main program loop.
    while (loop) {
        uart_menu_main();
    }
    return 0;
}
