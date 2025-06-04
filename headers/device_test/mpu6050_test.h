/**
 * @file mpu6050_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief MPU-6050 test code interface 
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
// Function pointers 

/**
 * @brief MPU6050 setters function pointer 1 
 * 
 * @details The following setters are called with this function pointer: 
 *           - mpu6050_set_low_power 
 *           - mpu6050_clear_low_power 
 *           - mpu6050_set_read_flag 
 *           - mpu6050_set_reset_flag 
 */
typedef void (*mpu6050_setter_ptr_1)(device_number_t device_num); 


/**
 * @brief MPU6050 setters function pointer 2 
 * 
 * @details The following setters are called with this function pointer: 
 *           - mpu6050_set_smpl_type 
 *           - mpu6050_set_read_state 
 */
typedef void (*mpu6050_setter_ptr_2)(
    device_number_t device_num, 
    uint8_t param); 


/**
 * @brief MPU6050 setters function pointer 3 
 * 
 * @details The following setters are called with this function pointer: 
 *           - mpu6050_cntrl_test_device_one 
 *           - mpu6050_cntrl_test_device_two 
 */
typedef void (*mpu6050_setter_ptr_3)(void); 


/**
 * @brief MPU6050 getters function pointer 1 
 * 
 * @details The following getters are called with this function pointer: 
 *           - mpu6050_get_state 
 */
typedef MPU6050_STATE (*mpu6050_getter_ptr_1)(device_number_t device_num); 


/**
 * @brief MPU6050 getters function pointer 2 
 * 
 * @details The following getters are called with this function pointer: 
 *           - mpu6050_get_fault_code 
 */
typedef MPU6050_FAULT_CODE (*mpu6050_getter_ptr_2)(device_number_t device_num); 


/**
 * @brief MPU6050 getters function pointer 3 
 * 
 * @details The following getters are called with this function pointer: 
 *           - mpu6050_get_accel_axis 
 *           - mpu6050_get_gyro_axis 
 */
typedef void (*mpu6050_getter_ptr_3)(
    device_number_t device_num, 
    int16_t *x, 
    int16_t *y, 
    int16_t *z); 


/**
 * @brief MPU6050 getters function pointer 4 
 * 
 * @details The following getters are called with this function pointer: 
 *           - mpu6050_get_temp_raw 
 */
typedef int16_t (*mpu6050_getter_ptr_4)(device_number_t device_num); 


/**
 * @brief MPU6050 getters function pointer 5 
 * 
 * @details The following getters are called with this function pointer: 
 *           - mpu6050_get_accel_axis_gs 
 *           - mpu6050_get_gyro_axis_rate 
 */
typedef void (*mpu6050_getter_ptr_5)(
    device_number_t device_num, 
    float *x, 
    float *y, 
    float *z); 


/**
 * @brief MPU6050 getters function pointer 6 
 * 
 * @details The following getters are called with this function pointer: 
 *           - mpu6050_get_temp 
 */
typedef float (*mpu6050_getter_ptr_6)(device_number_t device_num); 

//=======================================================================================


//=======================================================================================
// Structures 

/**
 * @brief Structure of all MPU6050 setter and getter function pointers (see above) 
 */
typedef struct mpu6050_func_ptrs_s 
{
    mpu6050_setter_ptr_1 setter_1; 
    mpu6050_setter_ptr_2 setter_2; 
    mpu6050_setter_ptr_3 setter_3; 
    mpu6050_getter_ptr_1 getter_1; 
    mpu6050_getter_ptr_2 getter_2; 
    mpu6050_getter_ptr_3 getter_3; 
    mpu6050_getter_ptr_4 getter_4; 
    mpu6050_getter_ptr_5 getter_5; 
    mpu6050_getter_ptr_6 getter_6; 
}
mpu6050_func_ptrs_t; 

//=======================================================================================


//=======================================================================================
// Prototypes 

/**
 * @brief MPU-6050 test initialization and setup code 
 */
void mpu6050_test_init(void); 


/**
 * @brief MPU-6050 test application code 
 */
void mpu6050_test_app(void); 

//=======================================================================================

#endif  // _MPU6050_TEST_H_ 
