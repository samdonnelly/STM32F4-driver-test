/**
 * @file lsm303agr_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief LSM303AGR test code 
 * 
 * @version 0.1
 * @date 2023-06-11
 * 
 * @copyright Copyright (c) 2023
 * 
 */

//=======================================================================================
// Includes 

#include "lsm303agr_test.h" 

//=======================================================================================


//=======================================================================================
// Global variables 

static int16_t mx_data; 
static int16_t my_data; 
static int16_t mz_data; 

//=======================================================================================


// Setup code
void lsm303agr_test_init(void)
{
    // Setup code for the gpio_test here 

    // Initialize GPIO ports 
    gpio_port_init(); 

    // Initialize timers 
    tim_9_to_11_counter_init(
        TIM9, 
        TIM_84MHZ_1US_PSC, 
        0xFFFF,  // Max ARR value 
        TIM_UP_INT_DISABLE); 
    tim_enable(TIM9); 

    // Initialize UART2
    uart_init(
        USART2, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_BAUD_9600, 
        UART_CLOCK_42); 

    // Initialize I2C1
    i2c_init(
        I2C1, 
        PIN_9, 
        GPIOB, 
        PIN_8, 
        GPIOB, 
        I2C_MODE_SM,
        I2C_APB1_42MHZ,
        I2C_CCR_SM_42_100,
        I2C_TRISE_1000_42);

    // LSM303AGR init 
    lsm303agr_init(
        I2C1, 
        LSM303AGR_M_ODR_10, 
        LSM303AGR_M_MODE_CONT, 
        LSM303AGR_CFG_DISABLE, 
        LSM303AGR_CFG_DISABLE, 
        LSM303AGR_CFG_DISABLE, 
        LSM303AGR_CFG_DISABLE); 
} 


// Test code 
void lsm303agr_test_app(void)
{
    // Test code for the gpio_test here 

    // Update the magnetometer data 
    lsm303agr_m_read(); 
    lsm303agr_m_get_data(&mx_data, &my_data, &mz_data); 

    // Display the first device results - values are scaled to remove decimal 
    uart_sendstring(USART2, "mx = ");
    uart_send_integer(USART2, mx_data);
    uart_send_spaces(USART2, UART_SPACE_2);

    uart_sendstring(USART2, "my = ");
    uart_send_integer(USART2, my_data); 
    uart_send_spaces(USART2, UART_SPACE_2);

    uart_sendstring(USART2, "mz = ");
    uart_send_integer(USART2, mz_data);
    uart_send_spaces(USART2, UART_SPACE_2);

    // Delay 
    tim_delay_ms(TIM9, 100);

    // Go to a the start of the line in the terminal 
    uart_sendstring(USART2, "\r"); 
}
