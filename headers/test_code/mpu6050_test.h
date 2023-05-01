/**
 * @file mpu6050_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief MPU6050 test code header 
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
#include "state_machine_test.h"

//=======================================================================================


//=======================================================================================
// Macros 

// Test control 
#define MPU6050_CONTROLLER_TEST 0        // Switch between driver and controller testing 
#define MPU6050_SECOND_DEVICE 0          // Include the test code for a second device 
#define MPU6050_INT_PIN 0                // Interrupt pin enable 

// Data 
#define MPU6050_DEV1_STBY_MASK 0x00      // Device 1 axis standby status mask 
#define MPU6050_DEV2_STBY_MASK 0x00      // Device 2 axis standby status mask 
#define MPU6050_SMPLRT_DIV 0             // Sample Rate Divider 

// Driver test 
#define MPU6050_DRIVER_LOOP_DELAY 100    // Delay (blocking) between code loops (ms) 
#define MPU6050_DRIVER_ST_DELAY 10       // Delay (blocking) after self test (ms) 

// Controller test 
#define MPU6050_NUM_TEST_CMDS 22         // Number of controller test commands for the user 
#define MPU6050_DEV1_RATE 250000         // Device 1 time between reading new data (us) 
#define MPU6050_DEV2_RATE 250000         // Device 2 time between reading new data (us) 

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief Accelerometer and gyroscope array index 
 */
typedef enum {
    MPU6050_X_AXIS,
    MPU6050_Y_AXIS,
    MPU6050_Z_AXIS
} mpu6050_axis_t;

//=======================================================================================


//=======================================================================================
// Function pointers 

/**
 * @brief MPU6050 setters function pointer 1 
 * 
 * @details The following setters are called with this function pointer: 
 *           - mpu6050_set_reset_flag 
 *           - mpu6050_set_low_power 
 *           - mpu6050_clear_low_power 
 */
typedef void (*mpu6050_setter_ptr_1)(
    device_number_t device_num); 


/**
 * @brief MPU6050 setters function pointer 2 
 * 
 * @details The following setters are called with this function pointer: 
 *           - mpu6050_cntrl_test_device_one 
 *           - mpu6050_cntrl_test_device_two 
 */
typedef void (*mpu6050_setter_ptr_2)(void); 


/**
 * @brief MPU6050 getters function pointer 1 
 * 
 * @details The following getters are called with this function pointer: 
 *           - mpu6050_get_state 
 */
typedef MPU6050_STATE (*mpu6050_getter_ptr_1)(
    device_number_t device_num); 


/**
 * @brief MPU6050 getters function pointer 2 
 * 
 * @details The following getters are called with this function pointer: 
 *           - mpu6050_get_fault_code 
 */
typedef MPU6050_FAULT_CODE (*mpu6050_getter_ptr_2)(
    device_number_t device_num); 


/**
 * @brief MPU6050 getters function pointer 3 
 * 
 * @details The following getters are called with this function pointer: 
 *           - mpu6050_get_accel_x_raw 
 *           - mpu6050_get_accel_y_raw 
 *           - mpu6050_get_accel_z_raw 
 *           - mpu6050_get_gyro_x_raw 
 *           - mpu6050_get_gyro_y_raw 
 *           - mpu6050_get_gyro_z_raw 
 *           - mpu6050_get_temp_raw 
 */
typedef int16_t (*mpu6050_getter_ptr_3)(
    device_number_t device_num); 


/**
 * @brief MPU6050 getters function pointer 4 
 * 
 * @details The following getters are called with this function pointer: 
 *           - mpu6050_get_accel_x 
 *           - mpu6050_get_accel_y 
 *           - mpu6050_get_accel_z 
 *           - mpu6050_get_gyro_x 
 *           - mpu6050_get_gyro_y 
 *           - mpu6050_get_gyro_z 
 *           - mpu6050_get_temp 
 */
typedef float (*mpu6050_getter_ptr_4)(
    device_number_t device_num); 

//=======================================================================================


//=======================================================================================
// Structures 

/**
 * @brief Structure of all MPU6050 setter and getter function pointers (see above) 
 */
typedef struct mpu6050_func_ptrs_s 
{
    mpu6050_setter_ptr_1 setter_1;   // Function pointer 1 
    mpu6050_setter_ptr_2 setter_2;   // Function pointer 2 
    mpu6050_getter_ptr_1 getter_1;   // Function pointer 3 
    mpu6050_getter_ptr_2 getter_2;   // Function pointer 4 
    mpu6050_getter_ptr_3 getter_3;   // Function pointer 5 
    mpu6050_getter_ptr_4 getter_4;   // Function pointer 6 
}
mpu6050_func_ptrs_t; 

//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief MPU6050 test initialization and setup code 
 */
void mpu6050_test_init(void); 


/**
 * @brief MPU6050 test application code 
 */
void mpu6050_test_app(void); 

//=======================================================================================

#endif  // _MPU6050_TEST_H_ 
