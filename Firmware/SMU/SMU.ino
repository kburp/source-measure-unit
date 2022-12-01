#include <SPI.h>

bool setCS(5);
bool setSCK(6);
bool setTX(7);
const uint8_t LDAC = 8;
const uint8_t SHDN = 9;

void setup() {
  SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
}

void loop() {
}
