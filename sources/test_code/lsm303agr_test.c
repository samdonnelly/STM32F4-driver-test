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

#if LSM303AGR_TEST_AXIS 
static int16_t mx_data; 
static int16_t my_data; 
static int16_t mz_data; 
#endif   // LSM303AGR_TEST_AXIS 

#if LSM303AGR_TEST_HEADING 
static int16_t offsets[8]; 
#endif   // LSM303AGR_TEST_HEADING 

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

    //==================================================
    // LSM303AGR init 

    // Set offsets. These are used to correct for errors in the magnetometer readings. This 
    // is application dependent so it is part of the device init and not integrated into the 
    // driver/library. 
    // Calibration steps: 
    // 1. 
    offsets[0] = -100;    // N  (0/360deg) direction heading offset (degrees * 10) 
    offsets[1] = -20;     // NE (45deg) direction heading offset (degrees * 10) 
    offsets[2] = 0;       // E  (90deg) direction heading offset (degrees * 10) 
    offsets[3] = 0;       // SE (135deg) direction heading offset (degrees * 10) 
    offsets[4] = 180;     // S  (180deg) direction heading offset (degrees * 10) 
    offsets[5] = 200;     // SW (225deg) direction heading offset (degrees * 10) 
    offsets[6] = 140;     // W  (270deg) direction heading offset (degrees * 10) 
    offsets[7] = -90;     // NW (315deg) direction heading offset (degrees * 10) 

    // Driver init 
    lsm303agr_init(
        I2C1, 
        offsets, 
        LSM303AGR_M_ODR_10, 
        LSM303AGR_M_MODE_CONT, 
        LSM303AGR_CFG_DISABLE, 
        LSM303AGR_CFG_DISABLE, 
        LSM303AGR_CFG_DISABLE, 
        LSM303AGR_CFG_DISABLE); 
    
    //==================================================
} 


// Test code 
void lsm303agr_test_app(void)
{
    // Test code for the gpio_test here 

    // Update the magnetometer data 
    lsm303agr_m_read(); 

#if LSM303AGR_TEST_AXIS 
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
    uart_send_new_line(USART2); 
#endif   // LSM303AGR_TEST_AXIS 

#if LSM303AGR_TEST_HEADING 
    uart_sendstring(USART2, "heading = ");
    uart_send_integer(USART2, lsm303agr_m_get_heading());
    uart_send_spaces(USART2, UART_SPACE_2); 
#endif   // LSM303AGR_TEST_HEADING 

    // Delay 
    tim_delay_ms(TIM9, 250);

#if LSM303AGR_TEST_AXIS 
    // Go up a line in the terminal to overwrite old data 
    uart_sendstring(USART2, "\033[1A"); 
#endif   // LSM303AGR_TEST_AXIS 

    // Go to a the start of the line in the terminal 
    uart_sendstring(USART2, "\r"); 
}
