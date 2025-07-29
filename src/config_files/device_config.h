/**
 * @file device_config.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Device configuration interface 
 * 
 * @details The definition of these device configs are left to the user since each case 
 *          may be different. Users should make a "device_config.c" file in 
 *          "./sources/config_files/" (which is not tracked) to define what's here. 
 * 
 * @version 0.1
 * @date 2025-05-29
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef _DEVICE_CONFIG_H_ 
#define _DEVICE_CONFIG_H_ 

//=======================================================================================
// Includes 

#include "stm32f411xe.h"
#include "lsm303agr_driver.h" 
#include "nrf24l01_driver.h" 

//=======================================================================================


//=======================================================================================
// LSM303AGR 

// Hard-iron offset register values 
extern const int16_t lsm303agr_hi_offset_reg[NUM_AXES];    // Hard-iron offsets for registers 

// Hard and soft-iron calibration values 
extern const float lsm303agr_hi_offset[NUM_AXES];        // Hard-iron offsets 
extern const float lsm303agr_sid_values[NUM_AXES];       // Soft-iron diagonal values 
extern const float lsm303agr_sio_values[NUM_AXES];       // Soft-iron off-diagonal values 

//=======================================================================================


//=======================================================================================
// M8Q 

#define M8Q_CONFIG_NUM_MSG_PKT_0 12    // Number of messages in a configuration packet 
#define M8Q_CONFIG_MAX_LEN_PKT_0 130   // Max length of a single config message in a packet 

extern const char m8q_config_no_pkt[]; 
extern const char m8q_config_pkt_0[M8Q_CONFIG_NUM_MSG_PKT_0][M8Q_CONFIG_MAX_LEN_PKT_0]; 

//=======================================================================================


//=======================================================================================
// MPU-6050 

extern const uint8_t standby_mask;              // Axis standby status mask 
extern const uint8_t sample_rate_divider;       // Sample Rate Divider 
extern const int16_t accel_offsets[NUM_AXES];   // Accelerometer axis offsets 
extern const int16_t gyro_offsets[NUM_AXES];    // Gyroscope axis offsets 

//=======================================================================================


//=======================================================================================
// nRF24L01 

// Address sent by the PTX and address accepted by the PRX 
extern const uint8_t nrf24l01_pipe_addr[NRF24l01_ADDR_WIDTH]; 

//=======================================================================================

#endif   // _DEVICE_CONFIG_H_ 
