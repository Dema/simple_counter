#include <HAL.h>
inline void
InitAll (void) {

//InitPort
  BUTTONS_DDR &= 0 << BUTTON_PLUS | 0 << BUTTON_MINUS;
  BUTTONS_PORT |= 1 << BUTTON_PLUS | 1 << BUTTON_MINUS;

  INDICATOR_DDR = 0xFF;

}
