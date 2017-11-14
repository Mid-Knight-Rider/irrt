#define PART_TM4C123GH6PM
#define TARGET_IS_TM4C123_RB1
#include <stdint.h>
#include <stdbool.h>
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/systick.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "ir_decoder.h"
#include "ir_proto.h"

#define BUFFER_SIZE 4096
unsigned char buffer[BUFFER_SIZE];
unsigned long buffer_sz = 0;
int buffer_ready = 0;
int mode_transmit = 0;

void ir_transmit(void)
{
    
}

void ir_receive(void)
{
    unsigned long next_bit = (0 == (GPIO_PORTE_DATA_R & 0x10UL)) ? 0 : 1;
    buffer[buffer_sz] = next_bit;
    buffer_sz++;
    if (buffer_sz >= BUFFER_SIZE) {
        buffer_ready = 1;
        MAP_SysTickIntDisable();
        buffer_sz = 0;
    }
}

void SysTick_Handler(void)
{
    if (mode_transmit) {
        ir_transmit();
    } else {
        ir_receive();
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
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    UARTStdioConfig(0, 9600, 16000000);
    // Enable interrupts globally.
    MAP_IntMasterEnable();
    // Main program loop.
    while (loop) {
        //UARTCharPut(UART0_BASE, 'a');
        UARTprintf("blah mannn\n");
        ir_proto proto;
        MAP_IntEnable(INT_GPIOE);
        // Wait until buffer is ready to parse.
        while (!buffer_ready);
        int decoded = decode(buffer, BUFFER_SIZE, &proto);
        if (0 == decoded) {
            GPIO_PORTC_DATA_R ^= 0x60UL;
        }
        buffer_ready = 0;
    }
    return 0;
}
