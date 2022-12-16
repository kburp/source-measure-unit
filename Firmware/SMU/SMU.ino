#include <SPI.h>

// define GPIO pins
const uint8_t CS = 5;
const uint8_t CLOCK = 6;
const uint8_t SDI = 7;
const uint8_t LDAC = 8;

const uint8_t SVMC = 9;
const uint8_t SCMV = 10;

const uint8_t LED = 11;

const uint8_t TWO_HUNDRED_OHM = 13;
const uint8_t TEN_K_OHM = 15;
const uint8_t ONE_HUNDRED_K_OHM = 12;
const uint8_t ONE_MILLION_OHM = 14;

const uint8_t DUT_VOLTAGE_SELECT = 16;
const uint8_t RSENSE_VOLTAGE_SELECT = 17;

const uint8_t RSENSE_TO_ADC = 24;
const uint8_t RSENSE_TO_FEEDBACK = 23;
const uint8_t DUT_TO_FEEDBACK = 22;
const uint8_t DUT_TO_ADC = 25;

const uint8_t ADC = A0;
const uint8_t POT = A1;

// configure DAC
const uint16_t BUF = 1; // 0 for unbuffered, 1 for buffered
const uint16_t GA = 1; // 0 for 2X gain, 1 for 1X gain
const uint16_t SHDN_BIT = 1; // 0 for shutdown, 1 for normal operation
const uint8_t DAC_SETTINGS = (BUF << 6) | (GA << 5) | (SHDN_BIT << 4);
uint8_t byte0, byte1;
uint16_t pot_value;
uint16_t dac_input;

uint8_t smu_mode; // Mode 0 is SVMC, Mode 1 is SCMV
unsigned long time_last_mc, time_last_mv; // Keep track of the time of last measurement


/* 
 * Given a digital DAC input, set the analog DAC output
 * 
 * Args:
 *  input: 12 bit digital input of the DAC (0 to 4095)
 */
void set_dac_output(uint16_t input) {
  digitalWrite(LED, HIGH);
  SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
  digitalWrite(CS, LOW);
  byte0 = (DAC_SETTINGS | (input >> 8));
  byte1 = (input & 0xFF);
  SPI.transfer(byte0);
  SPI.transfer(byte1);
  digitalWrite(CS, HIGH);
  delay(5);
  digitalWrite(LDAC, LOW);
  delay(5);
  digitalWrite(LDAC, HIGH);
  SPI.endTransaction();
  delay(50);
  digitalWrite(LED, LOW);
}


/* 
 * Set up the SMU to source voltage and measure current
 */
void set_svmc() {
  smu_mode = 0;
  pot_value = 0;
  digitalWrite(RSENSE_TO_FEEDBACK, LOW);
  digitalWrite(DUT_TO_ADC, LOW);
  digitalWrite(RSENSE_VOLTAGE_SELECT, LOW);
  digitalWrite(DUT_VOLTAGE_SELECT, HIGH);
  digitalWrite(TEN_K_OHM, LOW);
  digitalWrite(ONE_HUNDRED_K_OHM, LOW);
  digitalWrite(ONE_MILLION_OHM, LOW);
  digitalWrite(TWO_HUNDRED_OHM, HIGH);
  digitalWrite(DUT_TO_FEEDBACK, HIGH);
  digitalWrite(RSENSE_TO_ADC, HIGH);
  Serial.println("Mode Changed: Source Voltage, Measure Current");
 }


/*
 * Set up the SMU to source current and measure voltage
 */
void set_scmv() {
  smu_mode = 1;
  pot_value = 0;
  digitalWrite(DUT_TO_FEEDBACK, LOW);
  digitalWrite(RSENSE_TO_ADC, LOW);
  digitalWrite(DUT_VOLTAGE_SELECT, LOW);
  digitalWrite(RSENSE_VOLTAGE_SELECT, HIGH);
  digitalWrite(TWO_HUNDRED_OHM, LOW);
  digitalWrite(RSENSE_TO_FEEDBACK, HIGH);
  digitalWrite(DUT_TO_ADC, HIGH);
  Serial.println("Mode Changed: Source Current, Measure Voltage");
 }


void setup() {
  Serial.begin(9600);
  Serial.println("Starting SMU");

  time_last_mc = millis();
  time_last_mv = millis();
  
  SPI.setCS(CS);
  SPI.setSCK(CLOCK);
  SPI.setTX(SDI);

  pinMode(CS, OUTPUT);
  pinMode(CLOCK, OUTPUT);
  pinMode(SDI, OUTPUT);
  pinMode(LDAC, OUTPUT);
  pinMode(SVMC, INPUT);
  pinMode(SCMV, INPUT);
  pinMode(LED, OUTPUT);
  pinMode(TWO_HUNDRED_OHM, OUTPUT);
  pinMode(TEN_K_OHM, OUTPUT);
  pinMode(ONE_HUNDRED_K_OHM, OUTPUT);
  pinMode(ONE_MILLION_OHM, OUTPUT);
  pinMode(DUT_VOLTAGE_SELECT, OUTPUT);
  pinMode(RSENSE_VOLTAGE_SELECT, OUTPUT);
  pinMode(RSENSE_TO_ADC, OUTPUT);
  pinMode(RSENSE_TO_FEEDBACK, OUTPUT);
  pinMode(DUT_TO_FEEDBACK, OUTPUT);
  pinMode(DUT_TO_ADC, OUTPUT);
  pinMode(ADC, INPUT);
  pinMode(POT, INPUT);

  analogReadResolution(12);
  
  digitalWrite(LDAC, HIGH);

  SPI.begin();

  set_svmc();

  delay(1001);
}


void loop() {
  if (digitalRead(SVMC) == LOW && smu_mode == 1) {
    set_svmc();
  }
  else if (digitalRead(SCMV) == LOW && smu_mode == 0) {
    set_scmv();
  }

  if (abs(pot_value - analogRead(POT)) > 4) {
    pot_value = analogRead(POT);
    if (smu_mode == 0) {
      dac_input = pot_value;
      Serial.print("Supply Voltage: ");
      Serial.print(3.3 * dac_input / 4096.0 * ((12700 + 4750) / 4750.0));
      Serial.println(" V");
    }
    else {
      Serial.print("Supply Current: ");
      if (pot_value < 1024) {
        dac_input = pot_value * 4;
        digitalWrite(TWO_HUNDRED_OHM, LOW);
        digitalWrite(TEN_K_OHM, LOW);
        digitalWrite(ONE_HUNDRED_K_OHM, LOW);
        digitalWrite(ONE_MILLION_OHM, HIGH);
        Serial.print(dac_input * 3.3 / 4096.0 * ((12700 + 4750) / 4750.0) / 1000.0);
      }
      else if (pot_value < 2048) {
        dac_input = (pot_value - 1024) * 4;
        digitalWrite(TWO_HUNDRED_OHM, LOW);
        digitalWrite(TEN_K_OHM, LOW);
        digitalWrite(ONE_MILLION_OHM, LOW);
        digitalWrite(ONE_HUNDRED_K_OHM, HIGH);
        Serial.print(dac_input * 3.3 / 4096.0 * ((12700 + 4750) / 4750.0) / 100.0);
      }
      else if (pot_value < 3072) {
        dac_input = (pot_value - 2048) * 4;
        digitalWrite(TWO_HUNDRED_OHM, LOW);
        digitalWrite(ONE_HUNDRED_K_OHM, LOW);
        digitalWrite(ONE_MILLION_OHM, LOW);
        digitalWrite(TEN_K_OHM, HIGH);
        Serial.print(dac_input * 3.3 / 4096.0 * ((12700 + 4750) / 4750.0) / 10.0);
      }
      else {
        dac_input = (pot_value - 3072) * 4;
        digitalWrite(TEN_K_OHM, LOW);
        digitalWrite(ONE_HUNDRED_K_OHM, LOW);
        digitalWrite(ONE_MILLION_OHM, LOW);
        digitalWrite(TWO_HUNDRED_OHM, HIGH); 
        Serial.print(dac_input * 3.3 / 4096.0 * ((12700 + 4750) / 4750.0) * 5.0);
      }
      Serial.println(" mA");
    }
    set_dac_output(dac_input);
  }

  if (smu_mode == 0 && (millis() - time_last_mc > 1000)) {
    time_last_mc = millis();
    Serial.print("Current Measured: ");
    Serial.print(analogRead(ADC) * (3.3 / 4096.0) * ((12700 + 4750) / 4750.0) / 200.0 * 1000.0);
    Serial.println(" mA");
  } 
  else if (smu_mode == 1 && (millis() - time_last_mv > 1000)) {
    time_last_mv = millis();
    Serial.print("Voltage Measured: ");
    Serial.print(analogRead(ADC) * (3.3 / 4096.0) * ((12700 + 4750) / 4750.0));
    Serial.println(" V");
  }
}
