#ifndef COMMON_H_
#define COMMON_H_

// This should be the first include in every source file.

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

#endif
