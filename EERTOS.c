#include<avr/sleep.h>
#include <EERTOS.h>

// Очереди задач, таймеров.
// Тип данных - указатель на функцию
volatile static TPTR taskQueue[TASK_QUEUE_SIZE + 1];	// очередь указателей

volatile static struct {
  TPTR            goToTask;	// Указатель перехода
  u16             time;		// Выдержка в мс
} mainTimer[MAIN_TIMER_QUEUE_SIZE + 1];	// Очередь таймеров

// RTOS Подготовка. Очистка очередей
inline void
initRTOS (void) {
  u08             index;

  for (index = 0; index != TASK_QUEUE_SIZE + 1; index++) {	// Во все позиции записываем idle
    taskQueue[index] = idle;
  }
  for (index = 0; index != MAIN_TIMER_QUEUE_SIZE + 1; index++)	// Обнуляем все таймеры.
  {
    mainTimer[index].goToTask = idle;
    mainTimer[index].time = 0;
  }
  set_sleep_mode (SLEEP_MODE_IDLE);
}


//Пустая процедура - простой ядра. 
void
idle (void) {
  sleep_mode ();
}

// Функция установки задачи в очередь. Передаваемый параметр - указатель на функцию
// Отдаваемое значение - код ошибки.
void
setTask (TPTR task) {
  u08             index = 0;

  u08             nointerrupted = 0;

  if (STATUS_REG & (1 << Interrupt_Flag))	// Если прерывания разрешены, то запрещаем их.
  {
    cli ();
    nointerrupted = 1;		// И ставим флаг, что мы не в прерывании. 
  }
  while (taskQueue[index] != idle)	// Прочесываем очередь задач на предмет свободной ячейки
  {				// с значением idle - конец очереди.
    index++;
    if (index == TASK_QUEUE_SIZE + 1)	// Если очередь переполнена то выходим не солоно хлебавши
    {
      if (nointerrupted)
	sei ();			// Если мы не в прерывании, то разрешаем прерывания
      return;			// Раньше функция возвращала код ошибки - очередь переполнена. Пока убрал.
    }
  }

  // Если нашли свободное место, то
  taskQueue[index] = task;	// Записываем в очередь задачу
  if (nointerrupted)
    sei ();			// И включаем прерывания если не в обработчике прерывания.
}

//Функция установки задачи по таймеру. Передаваемые параметры - указатель на функцию, 
// Время выдержки в тиках системного таймера. Возвращет код ошибки.
void
setTimerTask (TPTR task, u16 newTime) {
  u08             index = 0;

  u08             nointerrupted = 0;

  if (STATUS_REG & (1 << Interrupt_Flag))	// Проверка запрета прерывания, аналогично функции выше
  {
    cli ();
    nointerrupted = 1;
  }
  for (index = 0; index != MAIN_TIMER_QUEUE_SIZE + 1; ++index)	//Прочесываем очередь таймеров
  {
    if (mainTimer[index].goToTask == task)	// Если уже есть запись с таким адресом
    {
      mainTimer[index].time = newTime;	// Перезаписываем ей выдержку
      if (nointerrupted)
	sei ();			// Разрешаем прерывания если не были запрещены.
      return;			// Выходим. Раньше был код успешной операции. Пока убрал
    }
  }
  for (index = 0; index != MAIN_TIMER_QUEUE_SIZE + 1; ++index)	// Если не находим похожий таймер, то ищем любой пустой      
  {
    if (mainTimer[index].goToTask == idle) {
      mainTimer[index].goToTask = task;	// Заполняем поле перехода задачи
      mainTimer[index].time = newTime;	// И поле выдержки времени
      if (nointerrupted)
	sei ();			// Разрешаем прерывания
      return;			// Выход. 
    }
  }				// тут можно сделать return c кодом ошибки - нет свободных таймеров
}

/*=================================================================================
Диспетчер задач ОС. Выбирает из очереди задачи и отправляет на выполнение.
*/
inline void
taskManager (void) {
  u08             index = 0;

  TPTR            goToTask = idle;	// Инициализируем переменные

  cli ();			// Запрещаем прерывания!!!
  goToTask = taskQueue[0];	// Хватаем первое значение из очереди
  if (goToTask == idle)		// Если там пусто
  {
    sei ();			// Разрешаем прерывания
    (idle) ();			// Переходим на обработку пустого цикла
  }

  else {
    for (index = 0; index != TASK_QUEUE_SIZE; index++)	// В противном случае сдвигаем всю очередь
    {
      taskQueue[index] = taskQueue[index + 1];
    }
    taskQueue[TASK_QUEUE_SIZE] = idle;	// В последнюю запись пихаем затычку
    sei ();			// Разрешаем прерывания
    (goToTask) ();		// Переходим к задаче
  }
}

/*
Служба таймеров ядра. Должна вызываться из прерывания раз в 1мс. Хотя время можно варьировать в зависимости от задачи

To DO: Привести к возможности загружать произвольную очередь таймеров. Тогда можно будет создавать их целую прорву. 
А также использовать эту функцию произвольным образом. 
В этом случае не забыть добавить проверку прерывания. 
*/
inline void
timerService (void) {
  u08             index;

  for (index = 0; index != MAIN_TIMER_QUEUE_SIZE + 1; index++)	// Прочесываем очередь таймеров
  {
    if (mainTimer[index].goToTask == idle)
      continue;			// Если нашли пустышку - щелкаем следующую итерацию
    if (mainTimer[index].time != 1)	// Если таймер не выщелкал, то щелкаем еще раз. 
    {				// To Do: Вычислить по тактам, что лучше !=1 или !=0. 
      mainTimer[index].time--;	// Уменьшаем число в ячейке если не конец.
    }

    else {
      setTask (mainTimer[index].goToTask);	// Дощелкали до нуля? Пихаем в очередь задачу
      mainTimer[index].goToTask = idle;	// А в ячейку пишем затычку
    }
  }
}
