#include <HAL.h>
#include <EERTOS.h>

//RTOS Interrupt
ISR (RTOS_ISR) {
  TimerService ();
}

// Глобальные переменные ====================================================

struct Buttons {
  unsigned        plusButtonPressed:1;
  unsigned        plusButtonHolded:1;
  unsigned        minusButtonPressed:1;
  unsigned        minusButtonHolded:1;
} buttons;

int             number = 0;

int             digits[3];

unsigned char   currentIndicatorDigit = 0;

// Прототипы задач ===========================================================
void
                checkButtons (void);

void
                checkButtonsOn (void);

void
                checkDebouncedButtonsOn (void);

void
                checkButtonsOff (void);

void            checkButtonsHold (void);

void            checkDebouncedButtonsOff (void);

void            processButtons (void);

void
number2digits (void) {
}

//============================================================================
//Область задач
//============================================================================

void
checkButtons (void) {
  if (buttons.plusButtonPressed || buttons.minusButtonPressed) {
    SetTimerTask (checkButtonsOff, KEYSCAN_DELAY);
  } else {
    SetTimerTask (checkButtonsOn, KEYSCAN_DELAY);
  }
}

void
checkButtonsOn (void) {
  if (BUTTONS_PIN & (1 << BUTTON_PLUS | 1 << BUTTON_MINUS)) {
    SetTimerTask (checkDebouncedButtonsOn, DEBOUNCE_DELAY);
  } else {
    SetTimerTask (checkButtonsOn, KEYSCAN_DELAY);
  }
}

void
checkDebouncedButtonsOn (void) {
  unsigned char   b = BUTTONS_PIN & (1 << BUTTON_PLUS | 1 << BUTTON_MINUS);

  buttons.plusButtonPressed = (b & (1 << BUTTON_PLUS)) ? 1 : 0;
  buttons.minusButtonPressed = (b & (1 << BUTTON_MINUS)) ? 1 : 0;

  if (b) {
    SetTask (processButtons);
    SetTimerTask (checkButtonsOff, KEYSCAN_DELAY);
    SetTimerTask (checkButtonsHold, HOLD_KEY_DELAY);
  } else {
    SetTimerTask (checkButtonsOn, KEYSCAN_DELAY);
  }
}

void
checkButtonsOff (void) {
  if (!(BUTTONS_PIN & (1 << BUTTON_PLUS | 1 << BUTTON_MINUS))) {
    SetTimerTask (checkDebouncedButtonsOff, 30);
  } else {
    SetTimerTask (checkButtonsOff, 50);
  }

}

void
checkDebouncedButtonsOff (void) {
  unsigned char   b = BUTTONS_PIN & (1 << BUTTON_PLUS | 1 << BUTTON_MINUS);

  buttons.plusButtonPressed = (b & (1 << BUTTON_PLUS)) ? 1 : 0;
  buttons.minusButtonPressed = (b & (1 << BUTTON_MINUS)) ? 1 : 0;

  if (!buttons.plusButtonPressed)
    buttons.plusButtonHolded = 0;
  if (!buttons.minusButtonPressed)
    buttons.minusButtonHolded = 0;

  checkButtons ();
}

void
checkButtonsHold (void) {
  unsigned char   b = BUTTONS_PIN & (1 << BUTTON_PLUS | 1 << BUTTON_MINUS);

  buttons.plusButtonHolded = buttons.plusButtonPressed =
    (b & (1 << BUTTON_PLUS)) ? 1 : 0;
  buttons.minusButtonHolded = buttons.minusButtonPressed =
    (b & (1 << BUTTON_MINUS)) ? 1 : 0;

  if (b) {
    SetTimerTask (checkButtonsHold, 500);
  }

  checkButtons ();
}

void
processButtons (void) {
  unsigned char   updated = 0;

  if (buttons.plusButtonPressed) {
    if (buttons.plusButtonHolded) {
      number += 10;
    } else {
      number++;
    }
    updated = 1;
  }
  if (buttons.minusButtonPressed) {
    if (buttons.minusButtonHolded) {
      number -= 10;
    } else {
      number--;
    }
    updated = 1;
  }
  if (updated) {
    number2digits ();
  }
}

void
updateIndicator (void) {
//INDICATOR_DIGITS_PORT = 
}

//==============================================================================
void __attribute__ ((naked)) main (void) {
  InitAll ();			// Инициализируем периферию
  InitRTOS ();			// Инициализируем ядро
  RunRTOS ();			// Старт ядра. 


  buttons.plusButtonPressed = 0;
  buttons.minusButtonPressed = 0;
  buttons.plusButtonHolded = 0;
  buttons.minusButtonHolded = 0;


// Запуск фоновых задач.
  SetTimerTask (checkButtonsOn, 50);
  SetTimerTask (updateIndicator, 5);
  while (1)			// Главный цикл диспетчера
  {
    wdt_reset ();		// Сброс собачьего таймера
    TaskManager ();		// Вызов диспетчера
  }
}
