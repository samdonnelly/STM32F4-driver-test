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

#include "lsm303agr_driver.h" 
#include "nrf24l01_driver.h" 

//=======================================================================================


//=======================================================================================
// LSM303AGR 

// Zero offsets used for calibrating the device 
extern const int16_t lsm303agr_calibrate_offsets[LSM303AGR_M_NUM_DIR]; 

// Directional offsets to correct for heading errors (units: degrees*10) 
extern const int16_t lsm303agr_config_dir_offsets_0[LSM303AGR_M_NUM_DIR]; 
extern const int16_t lsm303agr_config_dir_offsets_1[LSM303AGR_M_NUM_DIR]; 

//=======================================================================================


//=======================================================================================
// M8Q 

// Number of messages in a configuration packet 
#define M8Q_CONFIG_NUM_MSG_PKT_0 12 

// Max length of a single config message in a packet 
#define M8Q_CONFIG_MAX_LEN_PKT_0 130 

extern const char m8q_config_no_pkt[]; 
extern const char m8q_config_pkt_0[M8Q_CONFIG_NUM_MSG_PKT_0][M8Q_CONFIG_MAX_LEN_PKT_0]; 

//=======================================================================================


//=======================================================================================
// nRF24L01 

// Address sent by the PTX and address accepted by the PRX 
extern const uint8_t nrf24l01_pipe_addr[NRF24l01_ADDR_WIDTH]; 

//=======================================================================================

#endif   // _DEVICE_CONFIG_H_ 
