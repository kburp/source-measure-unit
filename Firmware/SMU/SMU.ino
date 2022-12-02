#include <SPI.h>

const uint8_t CS = 5;
const uint8_t CLOCK = 6;
const uint8_t SDI = 7;
const uint8_t LDAC = 8;
const uint8_t SHDN = 9;
const uint8_t LED = 25;

const uint8_t POWER = 22;

const uint16_t BUF = 1; // 0 for unbuffered, 1 for buffered
const uint16_t GA = 1; // 0 for 2X gain, 1 for 1X gain
const uint16_t SHDN_BIT = 1; // 0 for shutdown, 1 for normal operation
const uint8_t DAC_SETTINGS = (BUF << 6) | (GA << 5) | (SHDN_BIT << 4);
uint8_t byte0, byte1;

void setup() {
  Serial.begin(9600);
  SPI.setCS(CS);
  SPI.setSCK(CLOCK);
  SPI.setTX(SDI);
  pinMode(LDAC, OUTPUT);
  pinMode(SHDN, OUTPUT);
  pinMode(LED, OUTPUT);

  pinMode(POWER, OUTPUT);
  digitalWrite(POWER, HIGH);
  
  digitalWrite(LDAC, HIGH);
  digitalWrite(SHDN, HIGH);
  SPI.begin();
}

void loop() {
  SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
  static uint16_t dac_output = 1365; // 0 to 4095
  digitalWrite(CS, LOW);
  delay(50);
  byte0 = (DAC_SETTINGS | (dac_output >> 8));
  byte1 = (dac_output & 0xFF);
  SPI.transfer(byte0);
  SPI.transfer(byte1);
  Serial.print(byte0);
  Serial.print("  ");
  Serial.println(byte1);
  delay(50);
  digitalWrite(CS, HIGH);
  digitalWrite(LDAC, LOW);
  delay(50);
  digitalWrite(LDAC, HIGH);
  SPI.endTransaction();
  digitalWrite(LED, HIGH);
  delay(200);
  digitalWrite(LED, LOW);
  delay(50);
  
}
