#include <EERTOS.h>

// Очереди задач, таймеров.
// Тип данных - указатель на функцию
volatile static TPTR TaskQueue[TaskQueueSize + 1];	// очередь указателей

volatile static struct {
  TPTR            GoToTask;	// Указатель перехода
  u16             Time;		// Выдержка в мс
} MainTimer[MainTimerQueueSize + 1];	// Очередь таймеров

// RTOS Подготовка. Очистка очередей
inline void
InitRTOS (void) {
  u08             index;

  for (index = 0; index != TaskQueueSize + 1; index++) {	// Во все позиции записываем Idle
    TaskQueue[index] = Idle;
  }
  for (index = 0; index != MainTimerQueueSize + 1; index++)	// Обнуляем все таймеры.
  {
    MainTimer[index].GoToTask = Idle;
    MainTimer[index].Time = 0;
  }
}


//Пустая процедура - простой ядра. 
void
Idle (void) {
}

// Функция установки задачи в очередь. Передаваемый параметр - указатель на функцию
// Отдаваемое значение - код ошибки.
void
SetTask (TPTR TS) {
  u08             index = 0;

  u08             nointerrupted = 0;

  if (STATUS_REG & (1 << Interrupt_Flag))	// Если прерывания разрешены, то запрещаем их.
  {
    cli ();
    nointerrupted = 1;		// И ставим флаг, что мы не в прерывании. 
  }
  while (TaskQueue[index] != Idle)	// Прочесываем очередь задач на предмет свободной ячейки
  {				// с значением Idle - конец очереди.
    index++;
    if (index == TaskQueueSize + 1)	// Если очередь переполнена то выходим не солоно хлебавши
    {
      if (nointerrupted)
	sei ();			// Если мы не в прерывании, то разрешаем прерывания
      return;			// Раньше функция возвращала код ошибки - очередь переполнена. Пока убрал.
    }
  }

  // Если нашли свободное место, то
  TaskQueue[index] = TS;	// Записываем в очередь задачу
  if (nointerrupted)
    sei ();			// И включаем прерывания если не в обработчике прерывания.
}

//Функция установки задачи по таймеру. Передаваемые параметры - указатель на функцию, 
// Время выдержки в тиках системного таймера. Возвращет код ошибки.
void
SetTimerTask (TPTR TS, u16 NewTime) {
  u08             index = 0;

  u08             nointerrupted = 0;

  if (STATUS_REG & (1 << Interrupt_Flag))	// Проверка запрета прерывания, аналогично функции выше
  {
    cli ();
    nointerrupted = 1;
  }
  for (index = 0; index != MainTimerQueueSize + 1; ++index)	//Прочесываем очередь таймеров
  {
    if (MainTimer[index].GoToTask == TS)	// Если уже есть запись с таким адресом
    {
      MainTimer[index].Time = NewTime;	// Перезаписываем ей выдержку
      if (nointerrupted)
	sei ();			// Разрешаем прерывания если не были запрещены.
      return;			// Выходим. Раньше был код успешной операции. Пока убрал
    }
  }
  for (index = 0; index != MainTimerQueueSize + 1; ++index)	// Если не находим похожий таймер, то ищем любой пустой      
  {
    if (MainTimer[index].GoToTask == Idle) {
      MainTimer[index].GoToTask = TS;	// Заполняем поле перехода задачи
      MainTimer[index].Time = NewTime;	// И поле выдержки времени
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
TaskManager (void) {
  u08             index = 0;

  TPTR            GoToTask = Idle;	// Инициализируем переменные

  cli ();			// Запрещаем прерывания!!!
  GoToTask = TaskQueue[0];	// Хватаем первое значение из очереди
  if (GoToTask == Idle)		// Если там пусто
  {
    sei ();			// Разрешаем прерывания
    (Idle) ();			// Переходим на обработку пустого цикла
  }

  else {
    for (index = 0; index != TaskQueueSize; index++)	// В противном случае сдвигаем всю очередь
    {
      TaskQueue[index] = TaskQueue[index + 1];
    }
    TaskQueue[TaskQueueSize] = Idle;	// В последнюю запись пихаем затычку
    sei ();			// Разрешаем прерывания
    (GoToTask) ();		// Переходим к задаче
  }
}

/*
Служба таймеров ядра. Должна вызываться из прерывания раз в 1мс. Хотя время можно варьировать в зависимости от задачи

To DO: Привести к возможности загружать произвольную очередь таймеров. Тогда можно будет создавать их целую прорву. 
А также использовать эту функцию произвольным образом. 
В этом случае не забыть добавить проверку прерывания. 
*/
inline void
TimerService (void) {
  u08             index;

  for (index = 0; index != MainTimerQueueSize + 1; index++)	// Прочесываем очередь таймеров
  {
    if (MainTimer[index].GoToTask == Idle)
      continue;			// Если нашли пустышку - щелкаем следующую итерацию
    if (MainTimer[index].Time != 1)	// Если таймер не выщелкал, то щелкаем еще раз. 
    {				// To Do: Вычислить по тактам, что лучше !=1 или !=0. 
      MainTimer[index].Time--;	// Уменьшаем число в ячейке если не конец.
    }

    else {
      SetTask (MainTimer[index].GoToTask);	// Дощелкали до нуля? Пихаем в очередь задачу
      MainTimer[index].GoToTask = Idle;	// А в ячейку пишем затычку
    }
  }
}
