#ifndef STPM32_H
#define STPM32_H

#include <Arduino.h>
#include <SPI.h>
#include "stpm32_define.h"

#define _V 0
#define _I 1

class stpm32 {
public:
    stpm32(int resetPin, int csPin, int synPin, int fnet = 50);
    stpm32(int resetPin, int csPin);

    bool init();

    // Main measurement
    void readRMSVoltageAndCurrent(uint8_t channel, float* voltage, float* current);
    float readRMSVoltage(uint8_t channel);
    float readRMSCurrent(uint8_t channel);
    double readActiveEnergy(uint8_t channel);
    float readActivePower(uint8_t channel);

    // Calibration
    void setCalibration(float calV, float calI) {
        for (uint8_t i = 0; i < 3; i++) {
            _calibration[i][_V] = calV;
            _calibration[i][_I] = calI;
        }
    }

    // Latch mode
    void autoLatch(bool enabled) { _autoLatch = enabled; }
    // CRC mode
    void CRC(bool enabled) { _crcEnabled = enabled; }

private:
    int RESET_PIN;
    int CS_PIN;
    int SYN_PIN;
    int netFreq;

    bool _autoLatch;
    bool _crcEnabled;

    float _calibration[3][2];

    uint8_t readBuffer[4];

    // SPI helpers
    void latch();
    void readFrame(uint8_t address, uint8_t* buffer);
    void sendFrame(uint8_t readAdd, uint8_t writeAdd, uint8_t dataLSB, uint8_t dataMSB);

    // Conversion helpers
    inline float calcVolt(uint16_t value);
    inline float calcCurrent(uint32_t value);
    inline double calcEnergy(uint32_t value);
    inline float calcPower(int32_t value);

    inline int32_t buffer0to28(uint8_t* buffer);
    inline uint16_t buffer0to14(uint8_t* buffer);
    inline uint32_t buffer15to32(uint8_t* buffer);
};

#endif
