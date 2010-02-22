#include <HAL.h>
#include <EERTOS.h>

//RTOS Interrupt
ISR (TIMER0_COMPA_vect) {
//ISR (RTOS_ISR) {
  TimerService ();
}

// Глобальные переменные ====================================================

#define SEG_ITEM(x) (x)
//#   define SEG_ITEM(x) (x^255)

unsigned char digit2segments[11] = {
//             gfedcba
  SEG_ITEM (0b00111111),	//0
  SEG_ITEM (0b00000110),	//1
  SEG_ITEM (0b01011011),	//2
  SEG_ITEM (0b01001111),	//3
  SEG_ITEM (0b01100110),	//4
  SEG_ITEM (0b01101101),	//5
  SEG_ITEM (0b01111101),	//6
  SEG_ITEM (0b00000111),	//7
  SEG_ITEM (0b01111111),	//8
  SEG_ITEM (0b01101111),	//9
  SEG_ITEM (0b01000000),	//-
};


volatile struct Buttons {
  unsigned        plusButtonPressed:1;
  unsigned        plusButtonHolded:1;
  unsigned        minusButtonPressed:1;
  unsigned        minusButtonHolded:1;
} buttons;

volatile int    number = 1;

volatile int    digits[3];

volatile unsigned char currentIndicatorDigit = 0;

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
number2digits (void);

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
    SetTimerTask (checkDebouncedButtonsOff, DEBOUNCE_DELAY);
  } else {
    SetTimerTask (checkButtonsOff, KEYSCAN_DELAY);
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
    if(number <1) number = 1;
    if(number > 999) number = 999;
    number2digits ();
  }
}

void
updateIndicator (void) {
  if (currentIndicatorDigit != 2 && digits[currentIndicatorDigit] == 0) {
  } else {
    INDICATOR_SEGMENTS_PORT = digit2segments[digits[currentIndicatorDigit]];
    INDICATOR_DIGITS_PORT = (INDICATOR_DIGITS_PORT & (255 - 7) ) | 1 << (2 - currentIndicatorDigit);
  }
  currentIndicatorDigit++;
  if (currentIndicatorDigit > 2)
    currentIndicatorDigit = 0;

  SetTimerTask (updateIndicator, 5);
}

void
number2digits () {
  unsigned int    num = number;

  digits[0] = num / 100;
  num = num % 100;
  digits[1] = num / 10;
  digits[2] = num % 10;
}
//==============================================================================
void __attribute__ ((naked)) main (void) {

  buttons.plusButtonPressed = 0;
  buttons.minusButtonPressed = 0;
  buttons.plusButtonHolded = 0;
  buttons.minusButtonHolded = 0;
  number2digits();

  InitAll ();			// Инициализируем периферию
  InitRTOS ();			// Инициализируем ядро
  RunRTOS ();			// Старт ядра

// Запуск фоновых задач.
  SetTimerTask (checkButtonsOn, 50);
  SetTimerTask (updateIndicator, 5);

  while (1)			// Главный цикл диспетчера
  {
    wdt_reset ();		// Сброс собачьего таймера
    TaskManager ();		// Вызов диспетчера
  }
}
