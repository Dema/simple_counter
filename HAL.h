#ifndef HAL_H
#   define HAL_H

#   include <avr/io.h>
#   include <avr/interrupt.h>
#   include <avrlibtypes.h>
#   include "avrlibdefs.h"
#   include "avrlibtypes.h"
#   include "avr/pgmspace.h"
#   include <avr/wdt.h>

//System Timer Config
#   define Prescaler	  		64
#   define TIMER_TICK_MILLIS       1
#   define	TimerDivider  		((F_CPU/Prescaler + TIMER_TICK_MILLIS*1000 / 2)/(TIMER_TICK_MILLIS*1000))	// 1 mS

//USART Config
#   define BAUDRATE 9600L


#   define BAUD_DIVIDER ((F_CPU + BAUDRATE * 8)/(16 * BAUDRATE)-1)
#   define REAL_BAUDRATE  ((F_CPU + (BAUD_DIVIDER + 1) * 8) / ((BAUD_DIVIDER + 1)*16))
#   define BAUDRATE_ERROR ((100L * (BAUDRATE - REAL_BAUDRATE) + BAUDRATE/2) / (BAUDRATE))

#   if BAUDRATE_ERROR < -2 || BAUDRATE_ERROR > 2
#      warning "Неправильно задана константа BAUDRATE"
#   endif

#   define HI(x) ((x)>>8)
#   define LO(x) ((x)& 0xFF)

//PORT Defines
#   define BUTTONS_PORT    PORTD
#   define BUTTONS_DDR     DDRD
#   define BUTTONS_PIN    PIND
#   define BUTTON_PLUS     4
#   define BUTTON_MINUS    5
#   define INDICATOR_DIGITS_PORT  PORTD
#   define INDICATOR_DIGITS_DDR  DDRD
#   define INDICATOR_SEGMENTS_PORT  PORTB
#   define INDICATOR_SEGMENTS_DDR  DDRB


// Задержки

#   define DEBOUNCE_DELAY  30/TIMER_TICK_MILLIS
#   define INDICATOR_DELAY 5/TIMER_TICK_MILLIS
#   define KEYSCAN_DELAY   50/TIMER_TICK_MILLIS
#   define HOLD_KEY_DELAY 500/TIMER_TICK_MILLIS


extern void     InitAll (void);

#endif /*  */
