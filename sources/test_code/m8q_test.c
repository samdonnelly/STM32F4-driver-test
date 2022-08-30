/**
 * @file m8q_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief SAM-M8Q test code 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//=======================================================================================
// Includes 

#include "m8q_test.h"

//=======================================================================================


//=======================================================================================
// Variables 

uint8_t reg_0xff = 0; 

//=======================================================================================


// Setup code
void m8q_test_init()
{
    // Timer initialization 
    tim9_init(TIMERS_APB2_84MHZ_1US_PRESCALAR); 

    // Serial terminal (UART2) setup 
    uart_init(USART2, UART_BAUD_9600, UART_CLOCK_42); 

    // I2C communication setup 
    i2c1_init(
        I2C1, 
        I2C1_SDA_PB9,
        I2C1_SCL_PB8,
        I2C_MODE_SM,
        I2C_APB1_42MHZ,
        I2C_CCR_SM_42_100,
        I2C_TRISE_1000_42);
    
    // SAM-M8Q device setup 
    // Currently none 

    // Delay to let everything finish setup before starting to send and receieve data 
    tim9_delay_ms(500); 
} 


// Test code 
void m8q_test_app()
{
    // Generate a start condition 
    i2c_start(I2C1); 

    // Write the slave address with read access 
    i2c_write_address(I2C1, M8Q_I2C_8_BIT_ADDR + M8Q_R_OFFSET); 
    i2c_clear_addr(I2C1); 

    // Read the 0xFF register 
    i2c_read_master_mode(I2C1, &reg_0xff, I2C_1_BYTE); 

    // Print the value of the register to see what was returned 
    uart_sendchar(USART2, reg_0xff); 
    uart_send_new_line(USART2); 

    // Delay before starting over 
    tim9_delay_ms(1000); 
}
