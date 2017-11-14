#if false
#ifndef MMIO_H_
#define MMIO_H_

// SYSCTL
#define SYSCTL_RCGCGPIO_R (*((volatile unsigned long *) 0x400FE608))
#define SYSCTL_PRGPIO_R   (*((volatile unsigned long *) 0x400FEA08))
#define SYSCTL_RCC_R      (*((volatile unsigned long *) 0x400FE060))
#define SYSCTL_RIS_R      (*((volatile unsigned long *) 0x400FE050))
#define SYSCTL_RCC2_R     (*((volatile unsigned long *) 0x400FE070))

// NVIC
#define NVIC_ST_CTRL_R    (*((volatile unsigned long *) 0xE000E010))
#define NVIC_ST_RELOAD_R  (*((volatile unsigned long *) 0xE000E014))
#define NVIC_ST_CURRENT_R (*((volatile unsigned long *) 0xE000E018))
#define NVIC_SYS_PRI3_R   (*((volatile unsigned long *) 0xE000ED20))
#define NVIC_PRI1_R       (*((volatile unsigned long *) 0xE000E404))
#define NVIC_EN0_R        (*((volatile unsigned long *) 0xE000E100))
#define NVIC_DIS0_R       (*((volatile unsigned long *) 0xE000E180))

// PORT A
#define GPIO_PORTA_DATA_R  (*((volatile unsigned long *) 0x400043FC))
#define GPIO_PORTA_DIR_R   (*((volatile unsigned long *) 0x40004400))
#define GPIO_PORTA_AFSEL_R (*((volatile unsigned long *) 0x40004420))
#define GPIO_PORTA_PUR_R   (*((volatile unsigned long *) 0x40004510))
#define GPIO_PORTA_DEN_R   (*((volatile unsigned long *) 0x4000451C))
#define GPIO_PORTA_CR_R    (*((volatile unsigned long *) 0x40004524))
#define GPIO_PORTA_AMSEL_R (*((volatile unsigned long *) 0x40004528))

// PORT B
#define GPIO_PORTB_DATA_R  (*((volatile unsigned long *) 0x400053FC))
#define GPIO_PORTB_DIR_R   (*((volatile unsigned long *) 0x40005400))
#define GPIO_PORTB_AFSEL_R (*((volatile unsigned long *) 0x40005420))
#define GPIO_PORTB_PUR_R   (*((volatile unsigned long *) 0x40005510))
#define GPIO_PORTB_DEN_R   (*((volatile unsigned long *) 0x4000551C))
#define GPIO_PORTB_CR_R    (*((volatile unsigned long *) 0x40005524))
#define GPIO_PORTB_AMSEL_R (*((volatile unsigned long *) 0x40005528))

// PORT C
#define GPIO_PORTC_DATA_R  (*((volatile unsigned long *) 0x400063FC))
#define GPIO_PORTC_DIR_R   (*((volatile unsigned long *) 0x40006400))
#define GPIO_PORTC_AFSEL_R (*((volatile unsigned long *) 0x40006420))
#define GPIO_PORTC_PUR_R   (*((volatile unsigned long *) 0x40006510))
#define GPIO_PORTC_DEN_R   (*((volatile unsigned long *) 0x4000651C))
#define GPIO_PORTC_CR_R    (*((volatile unsigned long *) 0x40006524))
#define GPIO_PORTC_AMSEL_R (*((volatile unsigned long *) 0x40006528))

// PORT D
#define GPIO_PORTD_DATA_R  (*((volatile unsigned long *) 0x400073FC))
#define GPIO_PORTD_DIR_R   (*((volatile unsigned long *) 0x40007400))
#define GPIO_PORTD_AFSEL_R (*((volatile unsigned long *) 0x40007420))
#define GPIO_PORTD_PUR_R   (*((volatile unsigned long *) 0x40007510))
#define GPIO_PORTD_DEN_R   (*((volatile unsigned long *) 0x4000751C))
#define GPIO_PORTD_CR_R    (*((volatile unsigned long *) 0x40007524))
#define GPIO_PORTD_AMSEL_R (*((volatile unsigned long *) 0x40007528))

// PORT E
#define GPIO_PORTE_DATA_R  (*((volatile unsigned long *) 0x400243FC))
#define GPIO_PORTE_DIR_R   (*((volatile unsigned long *) 0x40024400))
#define GPIO_PORTE_AFSEL_R (*((volatile unsigned long *) 0x40024420))
#define GPIO_PORTE_PUR_R   (*((volatile unsigned long *) 0x40024510))
#define GPIO_PORTE_DEN_R   (*((volatile unsigned long *) 0x4002451C))
#define GPIO_PORTE_CR_R    (*((volatile unsigned long *) 0x40024524))
#define GPIO_PORTE_AMSEL_R (*((volatile unsigned long *) 0x40024528))
#define GPIO_PORTE_IS_R    (*((volatile unsigned long *) 0x40024404))
#define GPIO_PORTE_IBE_R   (*((volatile unsigned long *) 0x40024408))
#define GPIO_PORTE_IEV_R   (*((volatile unsigned long *) 0x4002440C))
#define GPIO_PORTE_IM_R    (*((volatile unsigned long *) 0x40024410))
#define GPIO_PORTE_ICR_R   (*((volatile unsigned long *) 0x4002441C))

// PORT F
#define GPIO_PORTF_DATA_R  (*((volatile unsigned long *) 0x400253FC))
#define GPIO_PORTF_DIR_R   (*((volatile unsigned long *) 0x40025400))
#define GPIO_PORTF_AFSEL_R (*((volatile unsigned long *) 0x40025420))
#define GPIO_PORTF_PUR_R   (*((volatile unsigned long *) 0x40025510))
#define GPIO_PORTF_DEN_R   (*((volatile unsigned long *) 0x4002551C))
#define GPIO_PORTF_CR_R    (*((volatile unsigned long *) 0x40025524))
#define GPIO_PORTF_AMSEL_R (*((volatile unsigned long *) 0x40025528))

// PCTL
#define GPIO_PORTA_PCTL_R (*((volatile unsigned long *) 0x4000452C))
#define GPIO_PORTB_PCTL_R (*((volatile unsigned long *) 0x4000552C))
#define GPIO_PORTC_PCTL_R (*((volatile unsigned long *) 0x4000652C))
#define GPIO_PORTD_PCTL_R (*((volatile unsigned long *) 0x4000752C))
#define GPIO_PORTE_PCTL_R (*((volatile unsigned long *) 0x4002452C))
#define GPIO_PORTF_PCTL_R (*((volatile unsigned long *) 0x4002552C))

// LOCK (write 0x4C4F434B to unlock, other locks)
#define GPIO_PORTC_LOCK_R (*((volatile unsigned long *) 0x40006520))
#define GPIO_PORTD_LOCK_R (*((volatile unsigned long *) 0x40007520))
#define GPIO_PORTF_LOCK_R (*((volatile unsigned long *) 0x40025520))

#endif
#endif
