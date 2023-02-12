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

#include "state_machine_test.h"

//=======================================================================================


//=======================================================================================
// Macros 

#define MPU6050_CONTROLLER_TEST 0   // Switch between driver and controller testing 
#define MPU6050_SECOND_DEVICE 0     // Include the test code for a second device 

#define LOOP_DELAY 1000

#define UINT16_DEC_DIGITS 6
#define NO_DECIMAL_SCALAR 100

// User interface 
#define MPU6050_NUM_TEST_CMDS 22      // Number of controller test commands for the user 

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief Function pointer index 
 * 
 * @details This is used along with the state machine tester. It is specific to a device 
 *          test so the names change across test files. This enum is used to define the 
 *          function pointer index within state_request_t that corresponds to a user command. 
 *          An array instance of m8q_func_ptrs_t is made in the test code and this 
 *          index has to match the location of the function pointer in the array. 
 */
typedef enum {
    MPU6050_SETTER_PTR_1, 
    MPU6050_SETTER_PTR_2, 
    MPU6050_GETTER_PTR_1, 
    MPU6050_GETTER_PTR_2, 
    MPU6050_GETTER_PTR_3, 
    MPU6050_GETTER_PTR_4 
} mpu6050_func_ptr_index_t; 


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
// Function pointers 

/**
 * @brief MPU6050 setters function pointer 1 
 * 
 * @details This function pointer is used for calling the following setters from the device 
 *          controller: 
 *           - mpu6050_set_reset_flag 
 *           - mpu6050_set_low_power 
 *           - mpu6050_clear_low_power 
 */
typedef void (*mpu6050_setter_ptr_1)(
    device_number_t device_num); 


/**
 * @brief 
 * 
 * @details This function pointer is used for calling the following setters from the device 
 *          controller: 
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
 * @brief Structure of all M8Q setter and getter function pointers (see above) 
 */
typedef struct mpu6050_func_ptrs_s 
{
    mpu6050_setter_ptr_1 setter_1; 
    mpu6050_setter_ptr_2 setter_2; 
    mpu6050_getter_ptr_1 getter_1; 
    mpu6050_getter_ptr_2 getter_2; 
    mpu6050_getter_ptr_3 getter_3; 
    mpu6050_getter_ptr_4 getter_4; 
}
mpu6050_func_ptrs_t; 

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
