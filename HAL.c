#include <HAL.h>
inline void
InitAll (void) {

//InitPort

  INDICATOR_DIGITS_DDR = 0b111;
  INDICATOR_DIGITS_PORT = 0b11000;
  INDICATOR_SEGMENTS_DDR = 0xFF;

}
