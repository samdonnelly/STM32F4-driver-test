/**
 * @file user_init.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Code that gets called once at the beginning of the program
 * 
 * @version 0.1
 * @date 2022-02-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//=======================================================================================
// Includes 

#include "user_init.h"

//=======================================================================================


// User init function 
void user_init()
{
    // Store the result of the accelerometer initialization 
    uint8_t mpu6050_init_status;

    // Store the results of the accelerometer self-test 
    uint8_t mpu_self_test_result;

    // Initialize timers 
    tim9_init(TIMERS_APB2_84MHZ_1US_PRESCALAR);

    // Initialize UART2
    uart2_init(UART2_BAUD_9600);

    // Initialize I2C1
    i2c1_init(
        I2C1_SDA_PB9,
        I2C1_SCL_PB8,
        I2C_SM_MODE,
        I2C_APB1_42MHZ,
        I2C_FM_DUTY_2,
        I2C_CCR_SM_42_100,
        I2C_TRISE_1000_42);

    // Initialize the accelerometer 
    // The return value can be used to enter an error state
    mpu6050_init_status = mpu6050_init(
                            MPU6050_1_ADDRESS,
                            DLPF_CFG_1,
                            SMPLRT_DIV_0,
                            AFS_SEL_4,
                            FS_SEL_500);

    // Return the status of the accelerometer WHO_AM_I register 
    switch(mpu6050_init_status)
    {
        case TRUE:
            // 
            uart2_sendstring("Device seen\r\n");
            break;
        
        case FALSE:
            // 
            uart2_sendstring("Device not seen\r\n");
            break;

        default:
            break;
    }

    // MPU6050 self-test 
    mpu_self_test_result = mpu6050_self_test(MPU6050_1_ADDRESS);
    uart2_sendstring("MPU6050 Self-Test Result = ");
    uart2_send_integer((int16_t)(mpu_self_test_result));
    uart2_send_new_line();
}
