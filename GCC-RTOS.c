#include <HAL.h>
#include <EERTOS.h>
 
//RTOS Interrupt
  ISR (RTOS_ISR) 
{
  TimerService ();
} 

// Прототипы задач ============================================================
void Task1 (void);
void Task2 (void);
void Task3 (void);

//============================================================================
//Область задач
//============================================================================
  void
Task1 (void) 
{
  SetTimerTask (Task2, 100);
  LED_PORT ^= 1 << LED1;
} void

Task2 (void) 
{
  SetTimerTask (Task1, 100);
  LED_PORT &= ~(1 << LED1);
} 

//==============================================================================
void  __attribute__ ((naked))  main (void) 
{
  InitAll ();			// Инициализируем периферию
  InitRTOS ();			// Инициализируем ядро
  RunRTOS ();			// Старт ядра. 
  
// Запуск фоновых задач.
    SetTask (Task1);
  while (1)			// Главный цикл диспетчера
    {
      wdt_reset ();		// Сброс собачьего таймера
      TaskManager ();		// Вызов диспетчера
    }
}


