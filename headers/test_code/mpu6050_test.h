/**
 * @file mpu6050_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief MPU-6050 test code header 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _MPU6050_TEST_H_
#define _MPU6050_TEST_H_

//=======================================================================================
// Includes 

#include "includes_drivers.h"

//=======================================================================================


//=======================================================================================
// Macros 

#define MPU6050_CONTROLLER_TEST 0   // Switch between driver and controller testing 

#define LOOP_DELAY 1000

#define UINT16_DEC_DIGITS 6
#define NO_DECIMAL_SCALAR 100

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief 
 * 
 */
typedef enum {
    ACCEL_X_AXIS,
    ACCEL_Y_AXIS,
    ACCEL_Z_AXIS
} accelerometer_axis_t;

/**
 * @brief 
 * 
 */
typedef enum {
    GYRO_X_AXIS,
    GYRO_Y_AXIS,
    GYRO_Z_AXIS
} gyroscope_axis_t;

//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief MPU-6050 setup code 
 * 
 */
void mpu6050_test_init(void); 


/**
 * @brief MPU-6050 test code 
 * 
 */
void mpu6050_test_app(void); 

//=======================================================================================

#endif  // _MPU6050_TEST_H_ 
