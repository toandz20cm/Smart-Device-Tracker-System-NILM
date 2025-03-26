//raw
#ifndef BL0940_h
#define BL0940_h

class BL0940 {
public:
  BL0940();

    void begin();
    bool getCurrentWaveform(float *currentWaveform);
    bool getVoltageWaveform(float *voltageWaveform);
    bool getCurrent(float *current);
    bool getVoltage(float *voltage);
    bool getActivePower(float *activePower);
    bool getActiveEnergy(float *activeEnergy);
    bool getPowerFactor(float *powerFactor);
    bool getTemperature(float *temperature);
    bool setFrequency(uint32_t Hz);
    bool setUpdateRate(uint32_t rate);
    bool setOverCurrentDetection( float detectionCurrent);  //[A] CF pin is high if current is larger than detectionCurrent
    bool setCFOutputMode();
    bool setNoLoadThreshold(uint8_t value);
    bool Reset();

private:
  // Add private variables and helper functions here if needed
    const uint16_t timeout = 1000;  //Serial timeout[ms]
    
    const float Vref = 1.218; //[V]
    const float R5 = 3.3;   //[Ohm]
    const float Rt = 2000.0;  //n:1 
  
    const float R8 = 20.0;  //[kOhm]
    const float R9 = 20.0;  //[kOhm]
    const float R10 = 20.0;  //[kOhm]
    const float R11 = 20.0;  //[kOhm]
    const float R12 = 20.0;  //[kOhm]

    const float R7 = 23.888888;  //[Ohm]
    uint16_t Hz = 50;   //[Hz]
    uint16_t updateRate = 400; //[ms]

    bool transferSPI(uint8_t *sendData, uint8_t *receivedData, int dataSize) ;
    uint8_t calculateChecksum(uint8_t *rxData, uint8_t state, uint8_t address);
    bool writeRegister(uint8_t address, uint32_t data);
    bool readRegister(uint8_t address, uint32_t *data);
};

#endif
