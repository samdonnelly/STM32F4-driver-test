/**
 * @file inertial_navigation_test.cpp
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Inertial navigation test 
 * 
 * @version 0.1
 * @date 2025-07-22
 * 
 * @copyright Copyright (c) 2025
 * 
 */

//=======================================================================================
// Includes 

#include "inertial_navigation_test.h"
#include "device_config.h"
#include "stm32f4xx_it.h"
#include "includes_drivers.h"

//=======================================================================================


//=======================================================================================
// Test data 

InertialNavigationTest inertial_navigation; 

static constexpr uint16_t interrupt_counter = 0x03E8;   // ARR=1000, (1000 counts)*(100us/count) = 100ms = 0.1s 
static constexpr uint8_t max_msg_len = 100; 

//=======================================================================================


//=======================================================================================
// Setup 

InertialNavigationTest::InertialNavigationTest()
    : uart(USART2),
      i2c(I2C1),
      tim_periodic(TIM10),
      device_num(DEVICE_ONE),
      st_result(CLEAR),
      status(MPU6050_OK)
{
}

void InertialNavigationTest::TestInit(void)
{
    // Initialize GPIO ports 
    gpio_port_init(); 

    // Periodic (counter update) interrupt timer 
    tim_9_to_11_counter_init(
        tim_periodic, 
        TIM_84MHZ_100US_PSC, 
        interrupt_counter, 
        TIM_UP_INT_ENABLE); 
    tim_enable(tim_periodic); 

    // UART - serial terminal output 
    uart_init(
        uart, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_PARAM_DISABLE, 
        CLEAR_BIT, 
        UART_FRAC_42_115200, 
        UART_MANT_42_115200, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE); 

    // Initialize I2C1
    i2c_init(
        i2c, 
        PIN_9, 
        GPIOB, 
        PIN_8, 
        GPIOB, 
        I2C_MODE_SM,
        I2C_APB1_42MHZ,
        I2C_CCR_SM_42_100,
        I2C_TRISE_1000_42); 

    // Initialize interrupt handler flags and enable the periodic timer interrupt handler 
    int_handler_init(); 
    nvic_config(TIM1_UP_TIM10_IRQn, EXTI_PRIORITY_0); 
    
    // Initialization the device 
    status |= mpu6050_init(
        device_num, 
        i2c, 
        MPU6050_ADDR_1,
        standby_mask, 
        MPU6050_DLPF_CFG_1,
        sample_rate_divider,
        MPU6050_AFS_SEL_4,
        MPU6050_FS_SEL_500);

    // Run a self-test 
    status |= mpu6050_self_test(device_num, &st_result); 

    // Set the device offsets to calibrate the readings 
    status |= mpu6050_set_offsets(device_num, accel_offsets, gyro_offsets); 

    if (status != MPU6050_OK)
    {
        IMUFault(); 
    }
}

//=======================================================================================


//=======================================================================================
// Application 

void InertialNavigationTest::TestApp(void)
{
    // Periodically update IMU data 
    if (handler_flags.tim1_up_tim10_glbl_flag)
    {
        handler_flags.tim1_up_tim10_glbl_flag = CLEAR; 

        // Read the latest IMU data 
        status |= mpu6050_update(device_num); 

        // Check the IMU for faults 
        if (status != MPU6050_OK)
        {
            IMUFault(); 
        }

        // Get the latest accelerometer and gyroscope data 
        mpu6050_get_accel_axis(device_num, accel_raw);   // Raw 
        mpu6050_get_gyro_axis(device_num, gyro_raw);     // Raw 
        mpu6050_get_accel_axis_gs(device_num, accel);    // g's 
        mpu6050_get_gyro_axis_rate(device_num, gyro);    // deg/s 

        // Perform inertial navigation calcs 
        InertialNavCalcs(); 
    }
}

//=======================================================================================


//=======================================================================================
// Test functions 

void InertialNavigationTest::IMUFault(void)
{
    char fault_msg[max_msg_len]; 
    snprintf(fault_msg, max_msg_len, "\r\nFault Code: %lu", status); 
    uart_send_str(uart, fault_msg); 
    while(TRUE); 
}


// Estimate the heading, velocity and position using data from the IMU 
void InertialNavigationTest::InertialNavCalcs(void)
{
    // Estimate the heading with the gyroscope and time between samples and keep it 
    // within acceptable bounds (0-360 degrees). 

    // Rotate the accelerometer data to find the acceleration in the NED frame. 

    // Integrate the NED acceleration using the sample interval to estimate the velocity 
    // then do that again to estimate the position. 
}


// Estimate orientation using the accelerometer, gyroscope and magnetometer data 
void InertialNavigationTest::MadgwickFilter(void)
{
    // 
}

//=======================================================================================
