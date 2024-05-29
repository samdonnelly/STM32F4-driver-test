/**
 * @file lsm303agr_config.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief LSM303AGR IMU configuration file 
 * 
 * @version 0.1
 * @date 2024-01-23
 * 
 * @copyright Copyright (c) 2024
 * 
 */

//=======================================================================================
// Includes 

#include "lsm303agr_config.h" 

//=======================================================================================


//=======================================================================================
// Data 

// Zero offsets used for calibrating the device 
const int16_t lsm303agr_calibrate_offsets[LSM303AGR_M_NUM_DIR] = { 0, 0, 0, 0, 0, 0, 0, 0}; 

//==================================================
// Directional offsets to correct for heading errors (units: degrees*10) 

const int16_t lsm303agr_config_dir_offsets_0[LSM303AGR_M_NUM_DIR] = 
{
    -160,     // N  (0/360deg) 
    32,       // NE (45deg) 
    215,      // E  (90deg) 
    385,      // SE (135deg) 
    435,      // S  (180deg) 
    20,       // SW (225deg) 
    -450,     // W  (270deg) 
    -365      // NW (315deg) 
}; 

const int16_t lsm303agr_config_dir_offsets_1[LSM303AGR_M_NUM_DIR] = 
{
    -180,     // N  (0/360deg) 
    20,       // NE (45deg) 
    190,      // E  (90deg) 
    340,      // SE (135deg) 
    360,      // S  (180deg) 
    -200,     // SW (225deg) 
    -450,     // W  (270deg) 
    -405      // NW (315deg) 
}; 

//==================================================

//=======================================================================================
