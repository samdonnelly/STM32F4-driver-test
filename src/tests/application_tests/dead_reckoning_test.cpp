/**
 * @file dead_reckoning_test.cpp
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Dead reckoning test 
 * 
 * @version 0.1
 * @date 2025-07-22
 * 
 * @copyright Copyright (c) 2025
 * 
 */

//=======================================================================================
// Includes 

#include "dead_reckoning_test.h"
#include "device_config.h"
#include "stm32f4xx_it.h"
#include "includes_drivers.h"

//=======================================================================================


//=======================================================================================
// Test data 

DeadReckoningTest dead_reckoning; 

// Timing 
static constexpr uint16_t interrupt_counter = 0x03E8;   // ARR=1000, (1000 counts)*(100us/count) = 100ms = 0.1s 

//=======================================================================================


//=======================================================================================
// Setup 

DeadReckoningTest::DeadReckoningTest()
    : uart(USART2),
      i2c(I2C1),
      tim_periodic(TIM10),
      device_num(DEVICE_ONE),
      st_result(CLEAR),
      status(MPU6050_OK)
{
}

void DeadReckoningTest::TestInit(void)
{
    // Initialize GPIO ports 
    gpio_port_init(); 

    // Periodic (counter update) interrupt timer 
    tim_9_to_11_counter_init(
        dead_reckoning.tim_periodic, 
        TIM_84MHZ_100US_PSC, 
        interrupt_counter, 
        TIM_UP_INT_ENABLE); 
    tim_enable(dead_reckoning.tim_periodic); 

    // UART - serial terminal output 
    uart_init(
        dead_reckoning.uart, 
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
        dead_reckoning.i2c, 
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
    
    // MPU-6050 driver initialization 
    dead_reckoning.status |= mpu6050_init(
        dead_reckoning.device_num, 
        dead_reckoning.i2c, 
        MPU6050_ADDR_1,
        standby_mask, 
        MPU6050_DLPF_CFG_1,
        sample_rate_divider,
        MPU6050_AFS_SEL_4,
        MPU6050_FS_SEL_500);

    // MPU-6050 self-test 
    dead_reckoning.status |= mpu6050_self_test(dead_reckoning.device_num, &dead_reckoning.st_result); 

    if (dead_reckoning.status != MPU6050_OK)
    {
        dead_reckoning.IMUFault(); 
    }
}

//=======================================================================================


//=======================================================================================
// Application 

void DeadReckoningTest::TestApp(void)
{
    // Periodically update IMU data 
    if (handler_flags.tim1_up_tim10_glbl_flag)
    {
        handler_flags.tim1_up_tim10_glbl_flag = CLEAR; 

        // Read the latest IMU data 
        dead_reckoning.status |= mpu6050_update(dead_reckoning.device_num); 

        // Check the IMU for faults 
        if (dead_reckoning.status != MPU6050_OK)
        {
            dead_reckoning.IMUFault(); 
        }
    }
}

//=======================================================================================


//=======================================================================================
// Test functions 

void DeadReckoningTest::IMUFault(void)
{
    // 
}

//=======================================================================================
