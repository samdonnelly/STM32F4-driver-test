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

#define UINT16_DEC_DIGITS 5
#define NO_DECIMAL_SCALAR 100

#define CHAR_OFFSET 48
#define REMAINDER_100000 100000
#define REMAINDER_10000 10000
#define REMAINDER_1000 1000
#define REMAINDER_100 100
#define REMAINDER_10 10
#define DIVIDE_10000 10000
#define DIVIDE_1000 1000
#define DIVIDE_100 100
#define DIVIDE_10 10
#define DIVIDE_1 1

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

/**
 * @brief 
 * 
 * @param value_to_print 
 */
void separate_digits(uint16_t value_to_print);

/**
 * @brief 
 * 
 * @param print_values 
 */
void print_to_serial(uint8_t *print_values);

/**
 * @brief 
 * 
 */
void print_new_line(void);

//=======================================================================================


#endif  // _USER_APP_H_
