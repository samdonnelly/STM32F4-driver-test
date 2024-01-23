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
// TODO 
// - Test magnetometer heading with low power, idle (mode), different ODR, LFP enabled, 
//   and offset cancellation 
//=======================================================================================


//=======================================================================================
// Macros 

#define LSM303AGR_TEST_NUM_DIRS 8         // Number of directions of heading offset calcs 

//=======================================================================================


//=======================================================================================
// Function prototypes 
//=======================================================================================


//=======================================================================================
// Global variables 
//=======================================================================================


//=======================================================================================
// Setup code

void lsm303agr_test_init(void)
{
    // Setup code for the LSM303AGR here 

    //===================================================
    // Standard setup 

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
        UART_FRAC_42_9600, 
        UART_MANT_42_9600, 
        UART_DMA_DISABLE, 
        UART_DMA_DISABLE); 

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
    
    //===================================================

    //===================================================
    // Conditional setup 
    
#if LSM303AGR_TEST_SCREEN_ON_BUS 
    // HD44780U screen driver setup. Used to provide user feedback. Note that is the 
    // screen is on the same I2C bus as the M8Q then the screen must be setup first 
    // to prevent the screen interfering  with the bus. 
    hd44780u_init(I2C1, TIM9, PCF8574_ADDR_HHH); 
    hd44780u_clear(); 
    hd44780u_backlight_off(); 
#endif   // LSM303AGR_TEST_SCREEN_ON_BUS 

    //===================================================

    //==================================================
    // LSM303AGR init 

    // Set offsets. These are used to correct for errors in the magnetometer readings. This 
    // is application/device dependent so it is part of the device init and not integrated 
    // into the driver/library. For a given application/device, these values should not change 
    // so it is ok to have them hard coded into the application code. 
    // 
    // Calibration steps: 
    // 1. Set the below values to zero. 
    // 2. Make sure LSM303AGR_TEST_HEADING is enabled and build the project. 
    // 3. Connect the LSM303AGR to the STM32, connect the STM32F4 to your machine, open PuTTy 
    //    so you can see the output and flash the code. 
    // 4. Using a trusted compasss (such as the one on your phone), align it in the North 
    //    direction, then align the long edge of the LSM303AGR (X-axis) with the compass so 
    //    they're both pointing North. 
    // 5. Observe the output of the magnetometer via Putty (Note that depending on the compass 
    //    you use you may have to move it away from the magnetometer once it's aligned or else 
    //    else you could get magnetometer interference - this happens with phone compasses). 
    //    Note the difference between the magnetometer output and the compass heading and 
    //    record it in offsets[0] below. If the magnetometer reading is clockwise of the compass 
    //    heading then the value recorded will be negative. Recorded offsets are scaled by 10. 
    //    Compass and magnetometer heading are from 0-359 degrees. For example, if the compass 
    //    reads 0 degrees (Magnetic North) and the magnetometer output reads +105 (10.5 degrees) 
    //    then the offset recorded is -105. 
    // 6. Repeat steps 4 and 5 for all directions in 45 degree increments (NE, E, SE, etc.) and 
    //    record each subsequent direction in the next 'offsets' element. 

    // Magnetometer directional offsets to correct for heading errors (units: degrees * 10) 
    int16_t mag_offsets[LSM303AGR_TEST_NUM_DIRS] = 
    {
        -160,     // N  (0/360deg) direction heading offset 
        32,       // NE (45deg) direction heading offset 
        215,      // E  (90deg) direction heading offset 
        385,      // SE (135deg) direction heading offset 
        435,      // S  (180deg) direction heading offset 
        20,       // SW (225deg) direction heading offset 
        -450,     // W  (270deg) direction heading offset 
        -365      // NW (315deg) direction heading offset 
    }; 

    // Driver init 
    lsm303agr_init(
        I2C1, 
        mag_offsets, 
        LSM303AGR_M_ODR_10, 
        LSM303AGR_M_MODE_CONT, 
        LSM303AGR_CFG_DISABLE, 
        LSM303AGR_CFG_DISABLE, 
        LSM303AGR_CFG_DISABLE, 
        LSM303AGR_CFG_DISABLE); 
    
    //==================================================
} 

//=======================================================================================


//=======================================================================================
// Test code 

void lsm303agr_test_app(void)
{
    // Test code for the LSM303AGR here 

#if LSM303AGR_TEST_HEADING 
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

    uart_sendstring(USART2, "heading = ");
    uart_send_integer(USART2, lsm303agr_m_get_heading());
    uart_send_spaces(USART2, UART_SPACE_2); 

    // Delay 
    tim_delay_ms(TIM9, 150);

#if LSM303AGR_TEST_AXIS 
    // Go up a line in the terminal to overwrite old data 
    uart_sendstring(USART2, "\033[1A"); 
#endif   // LSM303AGR_TEST_AXIS 

    // Go to a the start of the line in the terminal 
    uart_sendstring(USART2, "\r"); 
#endif   // LSM303AGR_TEST_HEADING 
}

//=======================================================================================


//=======================================================================================
// Test functions 
//=======================================================================================
