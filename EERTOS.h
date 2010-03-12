#ifndef EERTOS_H
#   define EERTOS_H

#   include <HAL.h>
#   include <EERTOSHAL.h>
extern void     initRTOS (void);

extern void     idle (void);

typedef void    (*TPTR) (void);

extern void     setTask (TPTR task);

extern void     setTimerTask (TPTR task, u16 newTime);

extern void     taskManager (void);

extern void     timerService (void);

//RTOS Errors Пока не используются.
#   define TaskSetOk			 'A'
#   define TaskQueueOverflow	 'B'
#   define TimerUpdated		 'C'
#   define TimerSetOk			 'D'
#   define TimerOverflow		 'E'

#endif /*  */
