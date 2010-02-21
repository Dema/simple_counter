#ifndef EERTOSHAL_H
#   define EERTOSHAL_H
#   include <HAL.h>

#   define STATUS_REG 			SREG
#   define Interrupt_Flag		SREG_I

//RTOS Config
#   ifdef VR_ATtiny2313__
#      define RTOS_ISR  			TIMER0_COMPA_vect
#   endif
       /*  */

#   define	TaskQueueSize		5
#   define MainTimerQueueSize  10
extern void     RunRTOS (void);

#endif /*  */
