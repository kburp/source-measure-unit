#include <SPI.h>

const uint8_t CS = 5;
const uint8_t CLOCK = 6;
const uint8_t SDI = 7;
const uint8_t LDAC = 8;
const uint8_t SHDN = 9;

const uint16_t AB = 0; // 0 for A, 1 for B
const uint16_t BUF = 0; // 0 for unbuffered, 1 for buffered
const uint16_t GA = 1; // 0 for 2X gain, 1 for 1X gain
const uint16_t SHDN_BIT = 1; // 0 for shutdown, 1 for normal operation
const uint16_t DAC_SETTINGS = (AB << 15) | (BUF << 14) | (GA << 13) | (SHDN_BIT << 12);

void setup() {
  Serial.begin(9600);
  SPI.setCS(CS);
  SPI.setSCK(CLOCK);
  SPI.setTX(SDI);
  SPI.begin();
  pinMode(LDAC, OUTPUT);
  pinMode(SHDN, OUTPUT);
  digitalWrite(LDAC, HIGH);
  digitalWrite(SHDN, HIGH);
}

void loop() {
  SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
  static uint16_t dac_output = 2000; // 0 to 4096
  digitalWrite(CS, LOW);
  SPI.transfer16(DAC_SETTINGS | dac_output);
  digitalWrite(CS, HIGH);
  digitalWrite(LDAC, LOW);
  digitalWrite(LDAC, HIGH);
  SPI.endTransaction();
 
  
}
