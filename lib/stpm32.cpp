/***************************************************
RMS voltage, RMS current, Active Energy, Active Power
****************************************************/

#include "stpm32.h"
#include <SPI.h>

SPISettings spiSettings(12000000, MSBFIRST, SPI_MODE3);

stpm32::stpm32(int resetPin, int csPin, int synPin, int fnet) {
    RESET_PIN = resetPin;
    CS_PIN = csPin;
    SYN_PIN = synPin;
    netFreq = fnet;
    _autoLatch = false;
    _crcEnabled = true;
    for (uint8_t i = 0; i < 3; i++) {
        _calibration[i][0] = 1.0;
        _calibration[i][1] = 1.0;
    }
}

stpm32::stpm32(int resetPin, int csPin) {
    stpm32(resetPin, csPin, -1, 50);
}

bool stpm32::init() {
    pinMode(CS_PIN, OUTPUT);
    if (SYN_PIN != -1) pinMode(SYN_PIN, OUTPUT);
    pinMode(RESET_PIN, OUTPUT);
    digitalWrite(CS_PIN, LOW);
    digitalWrite(RESET_PIN, LOW);
    delay(35);
    digitalWrite(RESET_PIN, HIGH);
    delay(35);
    digitalWrite(CS_PIN, HIGH);
    delay(2);
    if (SYN_PIN != -1) {
        for (size_t i = 0; i < 3; i++) {
            digitalWrite(SYN_PIN, LOW);
            delay(2);
            digitalWrite(SYN_PIN, HIGH);
            delay(2);
        }
    }
    delay(2);
    digitalWrite(CS_PIN, LOW);
    delay(5);
    digitalWrite(CS_PIN, HIGH);
    SPI.begin();
    return true;
}

void stpm32::readRMSVoltageAndCurrent(uint8_t channel, float* voltage, float* current) {
    uint8_t address;
    if (channel == 1) {
        address = C1_RMS_Data_Address;
    } else if (channel == 2) {
        address = C2_RMS_Data_Address;
    } else {
        return;
    }
    if (!_autoLatch) latch();
    readFrame(address, readBuffer);
    *voltage = calcVolt((uint16_t)buffer0to14(readBuffer)) * _calibration[channel][_V];
    *current = calcCurrent((uint32_t)buffer15to32(readBuffer)) * _calibration[channel][_I];
}

float stpm32::readRMSVoltage(uint8_t channel) {
    uint8_t address;
    if (channel == 1) {
        address = C1_RMS_Data_Address;
    } else if (channel == 2) {
        address = C2_RMS_Data_Address;
    } else {
        return -1;
    }
    if (!_autoLatch) latch();
    readFrame(address, readBuffer);
    return calcVolt((uint16_t)buffer0to14(readBuffer)) * _calibration[channel][_V];
}

float stpm32::readRMSCurrent(uint8_t channel) {
    uint8_t address;
    if (channel == 1) {
        address = C1_RMS_Data_Address;
    } else if (channel == 2) {
        address = C2_RMS_Data_Address;
    } else {
        return -1;
    }
    if (!_autoLatch) latch();
    readFrame(address, readBuffer);
    return calcCurrent((uint32_t)buffer15to32(readBuffer)) * _calibration[channel][_I];
}

double stpm32::readActiveEnergy(uint8_t channel) {
    if (channel > 2) return -1;
    
    uint8_t address;
    if (channel == 0) address = Tot_Active_Energy_Address;
    else if (channel == 1) address = PH1_Active_Energy_Address;
    else address = PH2_Active_Energy_Address;
    SPI.beginTransaction(spiSettings);
    digitalWrite(CS_PIN, LOW);
    SPI.transfer(address);
    SPI.transfer(0xff);
    SPI.transfer(0xff);
    SPI.transfer(0xff);
    digitalWrite(CS_PIN, HIGH);
    delayMicroseconds(4);
    digitalWrite(CS_PIN, LOW);
    uint8_t buf[4];
    for (int i = 0; i < 4; i++) {
        buf[0] = SPI.transfer(0xff);
        buf[1] = SPI.transfer(0xff);
        buf[2] = SPI.transfer(0xff);
        buf[3] = SPI.transfer(0xff);
    }
    digitalWrite(CS_PIN, HIGH);
    SPI.endTransaction();
    uint32_t value = (((buf[3] << 24) | (buf[2] << 16)) | (buf[1] << 8)) | buf[0];
    return calcEnergy(value);
}

float stpm32::readActivePower(uint8_t channel) {
    uint8_t address;
    if (channel == 1) address = PH1_Active_Power_Address;
    else if (channel == 2) address = PH2_Active_Power_Address;
    else return -1;
    if (!_autoLatch) latch();
    readFrame(address, readBuffer);
    return calcPower(buffer0to28(readBuffer)) * _calibration[channel][_V] * _calibration[channel][_I];
}

void stpm32::latch() {
#ifdef SPI_LATCH
    DSP_CR301bits.SW_Latch1 = 1;
    DSP_CR301bits.SW_Latch2 = 1;
    sendFrame(0x05, 0x05, DSP_CR301bits.LSB, DSP_CR301bits.MSB);
#else
    digitalWrite(SYN_PIN, LOW);
    delayMicroseconds(4);
    digitalWrite(SYN_PIN, HIGH);
    delayMicroseconds(4);
#endif
}

void stpm32::readFrame(uint8_t address, uint8_t *buffer) {
    sendFrame(address, 0xff, 0xff, 0xff);
    SPI.beginTransaction(spiSettings);
    digitalWrite(CS_PIN, LOW);
    for (uint8_t i = 0; i < 4; i++) {
        buffer[i] = SPI.transfer(0xff);
    }
    digitalWrite(CS_PIN, HIGH);
    SPI.endTransaction();
}

void stpm32::sendFrame(uint8_t readAdd, uint8_t writeAdd, uint8_t dataLSB, uint8_t dataMSB) {
    SPI.beginTransaction(spiSettings);
    digitalWrite(CS_PIN, LOW);
    SPI.transfer(readAdd);
    SPI.transfer(writeAdd);
    SPI.transfer(dataLSB);
    SPI.transfer(dataMSB);
    digitalWrite(CS_PIN, HIGH);
    SPI.endTransaction();
}

// conversion
inline float stpm32::calcVolt(uint16_t value) {
    return ((float)value) * 0.0354840440;
}
inline float stpm32::calcCurrent(uint32_t value) {
    return (float)value * 0.2143;
}
inline double stpm32::calcEnergy(uint32_t value) {
    return (double)value * ENERGY_LSB;
}
inline float stpm32::calcPower(int32_t value) {
    return value * 0.0001217;
}
inline int32_t stpm32::buffer0to28(uint8_t *buffer) {
    return (((buffer[3] << 24) | (buffer[2] << 16)) | (buffer[1] << 8)) | buffer[0];
}
inline uint16_t stpm32::buffer0to14(uint8_t *buffer) {
    return ((buffer[1] & 0x7f) << 8) | buffer[0];
}
inline uint32_t stpm32::buffer15to32(uint8_t *buffer) {
    return (((buffer[3] << 16) | (buffer[2] << 8)) | buffer[1]) >> 7;
}
