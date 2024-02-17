/**
 * @file lsm303agr_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief LSM303AGR test code header 
 * 
 * @version 0.1
 * @date 2023-06-11
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef _LSM303AGR_TEST_H_ 
#define _LSM303AGR_TEST_H_ 

//=======================================================================================
// Includes 

#include "includes_drivers.h"

//=======================================================================================


//=======================================================================================
// Macros 

//==================================================
// Conditional compilation 

// Only one mode is meant to be used at a time. The first macro under each mode is the 
// main/parent mode setter. Subsequent macros under the same mode are only used once the 
// mode is active. 

// Magnetometer axis data mode 
#define LSM303AGR_TEST_AXIS 0             // Magnetometer axis data read 

// Magnetometer heading mode 
#define LSM303AGR_TEST_HEADING 1          // Magnetometer heading read (compass) 
#define LSM303AGR_TEST_CALIBRATION 0      // Magnetometer heading calibration 

// Configurations - mode independent 
#define LSM303AGR_TEST_SCREEN_ON_BUS 1    // HD44780U screen on same I2C bus as device 

//==================================================

//=======================================================================================


//=======================================================================================
// Setup code 

/**
 * @brief Setup code
 */
void lsm303agr_test_init(void); 

//=======================================================================================


//=======================================================================================
// Test code 

/**
 * @brief Test code 
 */
void lsm303agr_test_app(void); 

//=======================================================================================

#endif   // _LSM303AGR_TEST_H_ 
