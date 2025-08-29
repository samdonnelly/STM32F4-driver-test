/**
 * @file driver_test_config.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Driver test configuration interface 
 * 
 * @details The definition of these driver test configs are left to the user since each 
 *          case may be different. Users should make a "driver_test_config.c" file in the 
 *          same folder as this file (which will not be tracked) to define what's here. 
 * 
 * @version 0.1
 * @date 2025-05-29
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef _DRIVER_TEST_CONFIG_H_ 
#define _DRIVER_TEST_CONFIG_H_ 

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes 

#include "stm32f411xe.h"
#include "lsm303agr_driver.h" 
#include "nrf24l01_driver.h" 

//=======================================================================================


//=======================================================================================
// Devices 

//==================================================
// LSM303AGR 

// Hard-iron offset register values 
extern const int16_t lsm303agr_hi_offset_reg[NUM_AXES];   // Hard-iron offsets for registers 

// Hard and soft-iron calibration values 
extern const float lsm303agr_hi_offset[NUM_AXES];         // Hard-iron offsets 
extern const float lsm303agr_sid_values[NUM_AXES];        // Soft-iron diagonal values 
extern const float lsm303agr_sio_values[NUM_AXES];        // Soft-iron off-diagonal values 

//==================================================


//==================================================
// M8Q 

#define M8Q_CONFIG_NUM_MSG_PKT_0 12    // Number of messages in a configuration packet 
#define M8Q_CONFIG_MAX_LEN_PKT_0 130   // Max length of a single config message in a packet 

extern const char m8q_config_no_pkt[]; 
extern const char m8q_config_pkt_0[M8Q_CONFIG_NUM_MSG_PKT_0][M8Q_CONFIG_MAX_LEN_PKT_0]; 

//==================================================


//==================================================
// MPU-6050 

extern const uint8_t standby_mask;              // Axis standby status mask 
extern const uint8_t sample_rate_divider;       // Sample Rate Divider 

// When setting accelerometer offsets, be mindful not to cancel out gravity (unless that 
// is desired). The gravity axis offset (typically z when stationary and level) will be 
// the raw reading minus the scalar listed in the description of mpu6050_fs_sel_set_t 
// depending on the full scale range set. 
extern const int16_t accel_offsets[NUM_AXES];   // Accelerometer axis offsets 
extern const int16_t gyro_offsets[NUM_AXES];    // Gyroscope axis offsets 

//==================================================


//==================================================
// nRF24L01 

// Address sent by the PTX and address accepted by the PRX 
extern const uint8_t nrf24l01_pipe_addr[NRF24l01_ADDR_WIDTH]; 

//==================================================

//=======================================================================================


//=======================================================================================
// Tools

//==================================================
// Madgwick Filter 

extern const float madgwick_B;             // Correction weight 
extern const float madgwick_dt;            // Time between samples/calculations (seconds) 

//==================================================


//==================================================
// Navigation Calculations 

extern const float magnetic_declination;   // Magnetic declination at current location 

//==================================================

//=======================================================================================


//=======================================================================================
// Application tests 
//=======================================================================================


//=======================================================================================
// Device tests 
//=======================================================================================


//=======================================================================================
// Peripheral tests 
//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // _DRIVER_TEST_CONFIG_H_ 
