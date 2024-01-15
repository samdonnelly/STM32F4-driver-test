/**
 * @file m8q_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief SAM-M8Q driver and controller hardware test code interface 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _M8Q_TEST_H_
#define _M8Q_TEST_H_

//=======================================================================================
// Includes 

#include "includes_drivers.h"
#include "int_handlers.h"

//=======================================================================================


//=======================================================================================
// Macros 

// Conditional compilation 
#define M8Q_TEST_SCREEN_ON_BUS 1     // HD44780U screen on I2C bus 

//=======================================================================================


//=======================================================================================
// Setup code 

/**
 * @brief Setup code for Test 0 
 */
void m8q_test_0_init(void); 


/**
 * @brief Setup code for Test 1 
 */
void m8q_test_1_init(void); 


/**
 * @brief Setup code for Test 2 
 */
void m8q_test_2_init(void); 

//=======================================================================================


//=======================================================================================
// Test code 

/**
 * @brief Test 0 code 
 * 
 * @details No config messages are sent to the device during setup so the device operates 
 *          in its default configuration (assuming no configurations saved in RAM). The 
 *          whole message stream is read and output over UART (to serial terminal). A 
 *          data buffer limit is set and neither TX ready or low power pins are used. 
 *          The code periodically stops reading the stream and lets the buffer overflow. 
 *          After an overflow, the data buffer is flushed and reading continues. 
 *          
 *          Note that during periods where the device is not reading new data ("read 
 *          pause"), the data size is read to keep the I2C port of the device active. If 
 *          this is not done, the device will asssume the host/master is no longer using 
 *          this interface and it will stop scheduling data packets to the port. By 
 *          keeping the port active, the data stream will accumulate past the max data 
 *          buffer size and a data stream flush (buffer overflow) will be triggers. In 
 *          other words, this is all done to demonstrate how the dirver works. 
 */
void m8q_test_0(void); 


/**
 * @brief Test 1 code 
 * 
 * @details Config messages are sent to the device, no data buffer limit is set and 
 *          specific data getters are used to read data. Data availability is checked via 
 *          the TX ready pin. The code will periodically put the device to sleep via the 
 *          low power pin then turn on again and continue to read data. When turning back 
 *          on the data stream is deliberately flushed before reading data normally. The 
 *          read data gets output over UART (to the serial terminal). 
 */
void m8q_test_1(void); 


/**
 * @brief Test 2 code 
 * 
 * @details The device is configured the same way as in Test 1 but this time the device 
 *          controller is used. The code alternates between idle, read and low power 
 *          states and outputs driver and controller data via UART. 
 */
void m8q_test_2(void); 

//=======================================================================================

#endif  // _M8Q_TEST_H_ 
