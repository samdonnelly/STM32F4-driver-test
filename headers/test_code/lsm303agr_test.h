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

// Test code 
#include "m8q_test.h"            // Included for coordinate calculation functions 

//=======================================================================================


//=======================================================================================
// Macros 

// Conditional compilation 
#define LSM303AGR_TEST_HEADING 0          // Heading with magnetometer as a compass 
#define LSM303AGR_TEST_AXIS 0             // Magnetometer axis readings 
#define LSM303AGR_TEST_NAV 1              // Navigating with magnetometer as a compass 
#define LSM303AGR_TEST_SCREEN 1           // HD44780U screen on same I2C bus as device 

// Mavigation test 
#define LSM303AGR_TEST_NUM_DIRS 8         // Number of directions of heading offset calcs 
#define LSM303AGR_TEST_NUM_WAYPOINTS 5    // Number of waypoints for location testing 
#define LSM303AGR_TEST_M_READ_INT 200000  // Magnetometer read interval (us) 
#define LSM303AGR_TEST_GPS_RAD 50         // GPS radius threshold (m*10) 
#define LSM303AGR_TEST_KP 1               // Proportional control constant 
#define LSM303AGR_TEST_KI 0               // Integral control constant 
#define LSM303AGR_TEST_KD 0               // Derivative control constant 

//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief Setup code
 */
void lsm303agr_test_init(void); 


/**
 * @brief Test code 
 */
void lsm303agr_test_app(void); 

//=======================================================================================

#endif   // _LSM303AGR_TEST_H_ 
