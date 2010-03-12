#ifndef EERTOSHAL_H
#   define EERTOSHAL_H
#   include <HAL.h>

#   define STATUS_REG 			SREG
#   define Interrupt_Flag		SREG_I

//RTOS Config
#   ifdef __AVR_ATtiny2313__
#      define RTOS_ISR  			TIMER0_COMPA_vect
#   endif
       /*  */

#   define	TASK_QUEUE_SIZE		5
#   define MAIN_TIMER_QUEUE_SIZE  5
extern void     runRTOS (void);

#endif /*  */
