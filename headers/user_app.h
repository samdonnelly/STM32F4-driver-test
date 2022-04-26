/**
 * @file user_app.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Code that gets called once per loop
 * 
 * @version 0.1
 * @date 2022-02-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _USER_APP_H_
#define _USER_APP_H_

//=======================================================================================
// Includes 

#include "includes.h"

//=======================================================================================


//=======================================================================================
// Macros 

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
// Function Prototypes

/**
 * @brief Function that gets called once per loop
 * 
 */
void user_app(void);

//=======================================================================================


#endif  // _USER_APP_H_
