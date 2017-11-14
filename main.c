#include <stdint.h>
#include <stdbool.h>
#include "mmio.h"
#include "ir_decoder.h"
#include "ir_proto.h"

#include "inc/hw_memmap.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "inc/tm4c123gh6pm.h"

#define BUFFER_SIZE 4096
unsigned char buffer[BUFFER_SIZE];
unsigned long buffer_sz = 0;
int buffer_ready = 0;
int mode_transmit = 0;

// Set clock speed to (400 / (sysdiv + 1)) MHz.
// sysdiv is unsigned 7 bits
void clock_set(unsigned long sysdiv)
{
    // Use RCC2.
    SYSCTL_RCC2_R |= 0x80000000UL;
    // Bypass PLL while initializing.
    SYSCTL_RCC2_R |= 0x00000800UL;
    // Select the external oscillator at 16 MHz.
    SYSCTL_RCC_R = (SYSCTL_RCC_R & ~0x000007C0UL) + 0x00000540UL;
    // Activate the PLL.
    SYSCTL_RCC2_R &= ~0x00002000UL;
    // Setup sysdiv using 400 MHz PLL.
    SYSCTL_RCC2_R |= 0x40000000UL;
    SYSCTL_RCC2_R = (SYSCTL_RCC2_R & ~0x1FC00000UL) + (sysdiv << 22);
    // Wait for the PLL to lock.
    while (0 == (SYSCTL_RIS_R & 0x00000040UL));
    // Enable the PLL by clearing the bypass.
    SYSCTL_RCC2_R &= ~0x00000800UL;
}

void systick_init(unsigned long period)
{
    NVIC_ST_CTRL_R &= ~0x01UL;
    NVIC_ST_RELOAD_R = period - 1;
    NVIC_ST_CURRENT_R = 0;
    // Priority = 0 (highest)
    NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R & 0x00FFFFFFUL);// | 0x40000000UL;
    NVIC_ST_CTRL_R |= 0x06UL;
}

void gpio_init(void)
{
    // pe4
    SYSCTL_RCGCGPIO_R |= 0x10UL;
    while (0 == (SYSCTL_PRGPIO_R & 0x10UL));
    GPIO_PORTE_AMSEL_R &= ~0x10UL;
    GPIO_PORTE_DIR_R &= ~0x10UL;
    GPIO_PORTE_AFSEL_R &= ~0x10UL;
    GPIO_PORTE_DEN_R |= 0x10UL;
    // Edge trigger interrupt.
    GPIO_PORTE_IM_R &= ~0x10UL;
    GPIO_PORTE_IS_R &= ~0x10UL; // edge trigger
    GPIO_PORTE_IBE_R &= ~0x10UL; // single edge only
    GPIO_PORTE_IEV_R &= ~0x10UL; // falling edge
    GPIO_PORTE_ICR_R = 0x10UL;
    GPIO_PORTE_IM_R |= 0x10UL;
    NVIC_PRI1_R = (NVIC_PRI1_R & 0xFFFFFF00UL) | 0xE0UL; // lowest prio
    
    // pc5 & pc6
    SYSCTL_RCGCGPIO_R |= 0x04UL;
    while (0 == (SYSCTL_PRGPIO_R & 0x04UL));
    GPIO_PORTC_AMSEL_R &= ~0x60UL;
    GPIO_PORTC_DIR_R |= 0x60UL;
    GPIO_PORTC_AFSEL_R &= ~0x60UL;
    GPIO_PORTC_DEN_R |= 0x60UL;
}

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
        NVIC_ST_CTRL_R &= ~0x01UL;
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
    GPIO_PORTE_ICR_R = 0x10UL;
    //GPIO_PORTC_DATA_R ^= 0x20UL;
    
    // Disable edge trigger until SysTick finishes.
    NVIC_DIS0_R = 0x10UL;
    
    // Enable SysTick to begin decoding incoming signal.
    NVIC_ST_CURRENT_R = 0;
    NVIC_ST_CTRL_R |= 0x01UL;
}

void InitConsole(void)
{
    // Enable GPIO port A which is used for UART0 pins.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // Configure the pin muxing for UART0 functions on port A0 and A1.
    GPIOPinConfigure(GPIO_PCTL_PA0_U0RX);
    GPIOPinConfigure(GPIO_PCTL_PA1_U0TX);

    // Enable UART0 so that we can configure the clock.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    // Use the internal 16MHz oscillator as the UART clock source.
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
   

		// Select the alternate (UART) function for these pins.   
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Initialize the UART for console I/O. 9600 BAUD
    //UARTStdioConfig(0, 9600, 16000000);
    UARTEnable(UART0_BASE);
}

int main(void)
{
    int running = 1;
    clock_set(86); // ~4.6 MHz
    systick_init(121); // 120.99
    gpio_init();
    //InitConsole();
    while (running) {
        //UARTCharPut(UART0_BASE, 'a');
        //UARTprintf("EE383 Lab 4: Introduction to C Programming, ADC, UART\n");
        ir_proto proto;
        // Enable edge trigger on IR receiver.
        NVIC_EN0_R = 0x10UL;
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
