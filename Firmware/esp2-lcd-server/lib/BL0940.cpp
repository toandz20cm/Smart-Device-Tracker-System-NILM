//raw
#include <Arduino.h>
#include "BL0940.h"
#include <SPI.h>
#include <math.h>

#define BL0940_DEBUG 1
#if BL0940_DEBUG
#define DBG(...) \
  { Serial.println(__VA_ARGS__); }
#define ERR(...) \
  { Serial.println(__VA_ARGS__); }
#else
#define DBG(...)
#define ERR(...)
#endif /* BL0940_DBG */

float KI = 0.52 ; 
float KU = 5.25;

#define BL0940_CS 5 // Chọn chân CS của BL0940
#define MOSI_PIN 23 // Chọn chân MOSI (Master Out Slave In)
#define MISO_PIN 19 // Chọn chân MISO (Master In Slave Out)
#define SCLK_PIN 18 // Chọn chân SCLK (Clock)

SPIClass mySPI(SPI);

BL0940::BL0940() {}

void BL0940::begin() {
  // Initialize SPI interface and BL0940
  pinMode(BL0940_CS, OUTPUT);
  mySPI.begin(SCLK_PIN, MISO_PIN, MOSI_PIN, BL0940_CS);
  mySPI.beginTransaction(SPISettings(900000, MSBFIRST, SPI_MODE1));
  digitalWrite(BL0940_CS, HIGH);
  Reset();
  setFrequency(50);
  setUpdateRate(400);
  setNoLoadThreshold(0x1D);
}

bool BL0940::transferSPI(uint8_t *sendData, uint8_t *receivedData, int dataSize) 
{
  digitalWrite(BL0940_CS, LOW); // Kích hoạt chip BL0940

  for (int i = 0; i < dataSize; i++) {
    receivedData[i] = mySPI.transfer(sendData[i]);
  }

  digitalWrite(BL0940_CS, HIGH); // Tắt chip BL0940
  
  return true;
}

uint8_t BL0940::calculateChecksum(uint8_t *rxData, uint8_t state, uint8_t address) 
{
  uint8_t STATE = state;
  uint8_t ADDR = address;
  uint8_t DATA_H = rxData[2];
  uint8_t DATA_M = rxData[3];
  uint8_t DATA_L = rxData[4];

  uint8_t checksum = STATE + ADDR + DATA_H + DATA_M + DATA_L;
  checksum &= 0xFF; // Ensure it's an 8-bit value
  checksum = ~checksum; // Bitwise invert the checksum

  return checksum;
}

bool BL0940::writeRegister(uint8_t address, uint32_t data) {

  //Register Unlock
  uint8_t unlockTxData[6] = { 0xA8, 0x1A, 0, 0, 0x55, 0 };
  uint8_t RxData[6] = {0x00};
  unlockTxData[5] = calculateChecksum(unlockTxData, 0xA8, 0x1A);
  //Write Register
  uint8_t txData[6] = { 0xA8, address, (uint8_t)(data >> 16), (uint8_t)(data >> 8), (uint8_t)(data), 0};
  txData[5] = calculateChecksum(txData, 0xA8, address);
  transferSPI(unlockTxData, RxData, sizeof(unlockTxData));
  transferSPI(txData, RxData, sizeof(txData));

  return true;
}

bool BL0940::readRegister(uint8_t address, uint32_t *data) {
  uint8_t txData[6] = { 0x58, address, 0, 0, 0, 0};
  uint8_t rxData[6] = { 0, 0, 0, 0, 0, 0};

  transferSPI(txData, rxData, sizeof(txData));

  uint8_t checksum = calculateChecksum(rxData, 0x58, address);

  if (rxData[5] != checksum) {
    return false;
  }

  *data = ((uint32_t)rxData[2] << 16) | ((uint32_t)rxData[3] << 8) | (uint32_t)rxData[4];
  return true;
}

bool BL0940::setNoLoadThreshold(uint8_t value) {
  return writeRegister(0x17, value); 
  }

bool BL0940::getCurrentWaveform(float *currentWaveform) {
  uint32_t data;

  if(false == readRegister(0x01, &data)) {
    return false;
  }

  int32_t rowActivePower = (int32_t)(data << 8);

  *currentWaveform = (float)rowActivePower * Vref/ ((324004.0 * R5 * 10000.0) / Rt)/KI;
  return true;
}

bool BL0940::getVoltageWaveform(float *voltageWaveform) {
  uint32_t data;

  if(false == readRegister(0x03, &data)) {
  return false;
  }
 
  int32_t rowActivePower = (int32_t)(data << 8);

  *voltageWaveform = (float)(rowActivePower * Vref * (R8 + R9 + R10 + R11 + R12) / (79931.0 * R7))/KU;
  return true;
}

bool BL0940::getCurrent(float *current) {
  uint32_t data;
  if (false == readRegister(0x04, &data)) {
    return false;
  }

  *current = (float)data * Vref / ((324004.0 * R5 * 1000.0) / Rt);
  return true;
}

bool BL0940::getVoltage(float *voltage) {
  uint32_t data;
  if (false == readRegister(0x06, &data)) {
    return false;
  }
  *voltage = (float)data * Vref * (R8 + R9 + R10 + R11 + R12) / (79931.0 * R7);
  return true;
}

bool BL0940::getActivePower(float *activePower) {
  uint32_t data;
  if (false == readRegister(0x08, &data)) {
    return false;
  }

  int32_t rowActivePower = (int32_t)(data << 8) / 256;
  if (rowActivePower < 0)
    rowActivePower = -rowActivePower;
  *activePower = (float)rowActivePower * Vref * Vref * (R8 + R9 + R10 + R11 + R12) / (4046.0 * (R5 * 1000.0 / Rt) * R7);
  return true;
}

bool BL0940::getActiveEnergy(float *activeEnergy) {

  uint32_t data;
  if (false == readRegister(0x0A, &data)) {
    return false;
  }

  int32_t rowCF_CNT = (int32_t)(data << 8) / 256;
  if (rowCF_CNT < 0)
    rowCF_CNT = -rowCF_CNT;
  *activeEnergy = (float)rowCF_CNT * 1638.4 * 256.0 * Vref * Vref * (R8 + R9 + R10 + R11 + R12) / (3600000.0 * 4046.0 * (R5 * 1000.0 / Rt) * R7);

  return true;
}

bool BL0940::getPowerFactor(float *powerFactor) {
  uint32_t data;
  if (false == readRegister(0x0C, &data)) {
    return false;
  }

  float rowPowerFactor = cos(2.0 * 3.1415926535 * (float)data * (float)Hz / 1000000.0) * 100.0;
  if (rowPowerFactor < 0)
    rowPowerFactor = -rowPowerFactor;
  *powerFactor = rowPowerFactor;

  return true;
}

bool BL0940::getTemperature(float *temperature) {
  uint32_t data;
  if (false == readRegister(0x0E, &data)) {
    return false;
  }

  int16_t rowTemperature = (int16_t)(data << 6) / 64
  ;
  *temperature = (170.0 / 448.0) * (rowTemperature / 2.0 - 32.0) - 45;
  return true;
}

bool BL0940::setFrequency(uint32_t Hz) {
  uint32_t data = 0;
  if (false == readRegister(0x18, &data)) {
    return false;
  }
  uint16_t mask = 0b0000001000000000;  //9bit
  if (Hz == 50)
    data &= ~mask;
  else
    data |= mask;
  
  if (false == writeRegister(0x18, data)) {
    return false;
  }
  if (false == readRegister(0x18, &data)) {
    return false;
  }
  if ((data & mask) == 0) {
    Hz = 50;
    // DBG("Set frequency:50Hz");
  } else {
    Hz = 60;
    // DBG("Set frequency:60Hz");
  }
  return true;
}

bool BL0940::setUpdateRate(uint32_t rate) {
  uint32_t data;
  if (false == readRegister(0x18, &data)) {
    return false;
  }

  uint16_t mask = 0b0000000100000000;  //8bit
  if (rate == 400)
    data &= ~mask;
  else
    data |= mask;

  if (false == writeRegister(0x18, data)) {
    return false;
  }

  if (false == readRegister(0x18, &data)) {
    return false;
  }

  if ((data & mask) == 0) {
    updateRate = 400;
    // DBG("Set update rate:400ms.");
  } else {
    updateRate = 800;
    // DBG("Set update rate:800ms.");
  }
  return true;
}

bool BL0940::setOverCurrentDetection(float detectionCurrent) {
  const float magicNumber = 0.72;  // I_FAST_RMS = 0.72 * I_RMS (Values obtained by experiments in the case of resistance load)

  //MODE[12] CF_UNABLE set 1 : alarm, enable by TPS_CTRL[14] configured
  uint32_t data;
  if (false == readRegister(0x18, &data)) {
    return false;
  }
  data |= 0b0001000000000000;  //12bit
  if (false == writeRegister(0x18, data)) {
    return false;
  }

  //TPS_CTRL[14] Alarm switch set 1 : Over-current and leakage alarm on
  if (false == readRegister(0x1B, &data)) {
    return false;
  }
  data |= 0b0100000000000000;  //14bit  0b0100000000000000
  if (false == writeRegister(0x1B, data)) {
    return false;
  }

  //Set detectionCurrent I_FAST_RMS_CTRL
  data = (uint32_t)(detectionCurrent * magicNumber / Vref * ((324004.0 * R5 * 1000.0) / Rt));
  data >>= 9;
  data &= 0x007FFF;
  float actualDetectionCurrent = (float)(data << 9) * Vref / ((324004.0 * R5 * 1000.0) / Rt);
  data |= 0b1000000000000000;  //15bit=1 Fast RMS refresh time is every cycle
  data &= 0x00000000FFFFFFFF;
  if (false == writeRegister(0x10, data)) {
    return false;
  }
  char massage[128];
  sprintf(massage, "Set Current Detection:%.1fA.", actualDetectionCurrent);
  DBG(massage);

  return true;
}

bool BL0940::setCFOutputMode() {
  //MODE[12] CF_UNABLE set 0 : alarm, enable by TPS_CTRL[14] configured
  uint32_t data;
  if (false == readRegister(0x18, &data)) {
    return false;
  }
  data &= ~0b0001000000000000;  //12bit
  if (false == writeRegister(0x18, data)) {
    return false;
  }
  return true;
}

bool BL0940::Reset() {
  if (false == writeRegister(0x19, 0x5A5A5A)) {
    return false;
  }
  delay(500);
  return true;
}
