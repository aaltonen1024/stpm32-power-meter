#ifndef STPM32_DEFINE_H
#define STPM32_DEFINE_H

// SPI frame and CRC
#define STPM3x_FRAME_LEN 5
#define CRC_8 (0x07)

// Register addresses
#define C1_RMS_Data_Address 0x48
#define C2_RMS_Data_Address 0x4A
#define PH1_Active_Power_Address 0x5C
#define PH2_Active_Power_Address 0x74
#define PH1_Active_Energy_Address 0x54
#define PH2_Active_Energy_Address 0x6C
#define Tot_Active_Energy_Address 0x84

// Calibration and conversion constants
#define ENERGY_LSB 0.00000000886162

#endif
