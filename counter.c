#include <HAL.h>
#include <EERTOS.h>
#include <avr/eeprom.h>


// Глобальные переменные ====================================================

//#define SEG_ITEM(x) (x)
#   define SEG_ITEM(x) (x^255)

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

 volatile unsigned int    number = 1;
 unsigned int saved_number EEMEM = 1;

 volatile unsigned char    digits[3];

 volatile unsigned char currentIndicatorDigit = 0;

void debugOn(void){
	PORTD |= _BV(PD5);
}

void debugOff(void){
  PORTD &= ~_BV(PD5);
}
void debug(void){
	debugOn();
	debugOff();
}
void readNumber(){
	number = eeprom_read_word(&saved_number);
}

void saveNumber(){
	eeprom_write_word(&saved_number,number);
}

//RTOS Interrupt
ISR (RTOS_ISR) {
  timerService ();
}
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
inline void resetButtons(void);
//============================================================================
//Область задач
//============================================================================

void
checkButtons (void) {
  if (buttons.plusButtonPressed || buttons.minusButtonPressed) {
    setTimerTask (checkButtonsOff, KEYSCAN_DELAY);
  } else {
    setTimerTask (checkButtonsOn, KEYSCAN_DELAY);
  }
}

void
checkButtonsOn (void) {
  buttons.plusButtonPressed = bit_is_clear(BUTTONS_PIN,BUTTON_PLUS) ? 1 : 0;
  buttons.minusButtonPressed =  bit_is_clear(BUTTONS_PIN,BUTTON_MINUS) ? 1 : 0;
  if (buttons.plusButtonPressed || buttons.minusButtonPressed) {
    setTimerTask (checkButtonsHold, HOLD_KEY_DELAY);
    setTimerTask (checkButtonsOff, KEYSCAN_DELAY);
  } else {
    setTimerTask (checkButtonsOn, KEYSCAN_DELAY);
  }
}

void
checkButtonsOff (void) {
  if(buttons.plusButtonPressed && bit_is_set(BUTTONS_PIN,BUTTON_PLUS) 
	|| buttons.minusButtonPressed && bit_is_set(BUTTONS_PIN, BUTTON_MINUS )){
    processButtons();
  }
  buttons.plusButtonPressed = bit_is_clear(BUTTONS_PIN,BUTTON_PLUS) ? 1 : 0;
  buttons.minusButtonPressed = bit_is_clear(BUTTONS_PIN,BUTTON_MINUS) ? 1 : 0;


  if (!buttons.plusButtonPressed)
    buttons.plusButtonHolded = 0;
  if (!buttons.minusButtonPressed)
    buttons.minusButtonHolded = 0;

  checkButtons ();
}

void
checkButtonsHold (void) {
  buttons.plusButtonHolded = buttons.plusButtonPressed =
     bit_is_clear(BUTTONS_PIN,BUTTON_PLUS) ? 1 : 0;
  buttons.minusButtonHolded = buttons.minusButtonPressed =
     bit_is_clear(BUTTONS_PIN,BUTTON_MINUS) ? 1 : 0;

  if (buttons.plusButtonHolded || buttons.minusButtonHolded) {
	processButtons();
    setTimerTask (checkButtonsHold, HOLD_KEY_DELAY);
  }

  checkButtons ();
}

void
processButtons (void) {
	unsigned char   updated = 0;
	if(buttons.plusButtonHolded && buttons.minusButtonHolded) {
		number = 1;
		updated = 1;
	}else{
		if (buttons.plusButtonPressed) {
			if (buttons.plusButtonHolded && number < 990) {
				number += 10;
			} else {
				number++;
			}
			updated = 1;
		}
		if (buttons.minusButtonPressed) {
			if (buttons.minusButtonHolded && number > 10) {
				number -= 10;
			} else {
				number--;
			}
			updated = 1;
		}
	}
	if (updated) {
		if(number < 1) number = 1;
		if(number > 999) number = 999;
		number2digits ();
		setTimerTask(saveNumber,5000);
	}
}

void
updateIndicator (void) {
  //если текущий разряд == 0 и цифра == 0, то не показываем
  //если текущий разряд == 1 и цифра == 0 и цифра в разряде 0 == 0 то не показываем

  if (currentIndicatorDigit == 0 && digits[0] == 0
		  || currentIndicatorDigit == 1 && digits[0] == 0 && digits[1] == 0 ) {
  } else {
	  INDICATOR_SEGMENTS_PORT = digit2segments[digits[currentIndicatorDigit]];
	  INDICATOR_DIGITS_PORT = (INDICATOR_DIGITS_PORT & (0xFF - 0b111) ) | 1 << (2 - currentIndicatorDigit);
  }
  currentIndicatorDigit++;
  if (currentIndicatorDigit > 2)
    currentIndicatorDigit = 0;

  setTimerTask (updateIndicator, INDICATOR_DELAY);
}

void
number2digits () {
	unsigned int    num = number;
	if(num > 999){
		digits[0] = 10;
		digits[1] = 10;
		digits[2] = 10;
	} else {
		digits[0] = num / 100;
		num = num % 100;
		digits[1] = num / 10;
		digits[2] = num % 10;
	}
}
void resetButtons(void){
  buttons.plusButtonPressed = 0;
  buttons.minusButtonPressed = 0;
  buttons.plusButtonHolded = 0;
  buttons.minusButtonHolded = 0;
}
//==============================================================================
void __attribute__ ((naked)) main (void) {
	readNumber();
	number2digits();
	resetButtons();

	initAll ();			// Инициализируем периферию
	initRTOS ();			// Инициализируем ядро
	runRTOS ();			// Старт ядра

	// Запуск фоновых задач.
	setTimerTask (checkButtonsOn, KEYSCAN_DELAY);
	setTimerTask (updateIndicator, INDICATOR_DELAY);

	while (1)			// Главный цикл диспетчера
	{
		wdt_reset ();		// Сброс собачьего таймера
		taskManager ();		// Вызов диспетчера
	}
}
