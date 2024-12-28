/**
 * @file system_config_template.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief System configuration template 
 * 
 * @details This file is a template and is not to be altered unless there is a system 
 *          configuration setting being added or removed. This means these settings should 
 *          not be used to configure your system. Instead, create a "system_config.h" file 
 *          in the same folder as this file and redefine each of the macros below (i.e. 
 *          copy the contents of this file) and change those to configure your system. 
 *          For example, if you're using FreeRTOS, then change FREERTOS_ENABLE to 1 in 
 *          "system_config.h". "system_config.h" is not tracked so it's specific to your 
 *          use case, but it's included in the build path. This file is tracked but 
 *          excluded from the build path. 
 * 
 * @version 0.1
 * @date 2024-02-25
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef _SYSTEM_CONFIG_TEMPLATE_H_ 
#define _SYSTEM_CONFIG_TEMPLATE_H_ 

//=======================================================================================
// System configuration 

//==================================================
// System settings 

// This needs to be set if using FreeRTOS as it includes additional files and alters 
// some core code that's used with FreeRTOS. Note that the 'RTOS_ENABLE' variable in 
// CMakeLists must be updated to match this macro. 
#ifndef FREERTOS_ENABLE 
#define FREERTOS_ENABLE 0 
#endif   // FREERTOS_ENABLE 

// This includes interrupt handler definitions that override defaults. Overrides will 
// be used for specific tests. If interrupts are used for a test but the test has no 
// specific override then this should be cleared. 
#ifndef INTERRUPT_OVERRIDE 
#define INTERRUPT_OVERRIDE 0 
#endif   // INTERRUPT_OVERRIDE 

//==================================================

//==================================================
// Board settings 

// The below list allows for choosing which STM32F4 board to use. Redefine each of the 
// following in the "system_config.h" file and change your selected board to 1. 
#ifndef STM32F4_05xx 
#define STM32F4_05xx 0 
#endif   // STM32F4_05xx 

#ifndef STM32F4_15xx 
#define STM32F4_15xx 0 
#endif   // STM32F4_15xx 

#ifndef STM32F4_07xx 
#define STM32F4_07xx 0 
#endif   // STM32F4_07xx 

#ifndef STM32F4_17xx 
#define STM32F4_17xx 0 
#endif   // STM32F4_17xx 

#ifndef STM32F4_27xx 
#define STM32F4_27xx 0 
#endif   // STM32F4_27xx 

#ifndef STM32F4_37xx 
#define STM32F4_37xx 0 
#endif   // STM32F4_37xx 

#ifndef STM32F4_29xx 
#define STM32F4_29xx 0 
#endif   // STM32F4_29xx 

#ifndef STM32F4_39xx 
#define STM32F4_39xx 0 
#endif   // STM32F4_39xx 

#ifndef STM32F4_01xC 
#define STM32F4_01xC 0 
#endif   // STM32F4_01xC 

#ifndef STM32F4_01xE 
#define STM32F4_01xE 0 
#endif   // STM32F4_01xE 

#ifndef STM32F4_10Tx 
#define STM32F4_10Tx 0 
#endif   // STM32F4_10Tx 

#ifndef STM32F4_10Cx 
#define STM32F4_10Cx 0 
#endif   // STM32F4_10Cx 

#ifndef STM32F4_10Rx 
#define STM32F4_10Rx 0 
#endif   // STM32F4_10Rx 

#ifndef STM32F4_11xE 
#define STM32F4_11xE 0 
#endif   // STM32F4_11xE 

#ifndef STM32F4_46xx 
#define STM32F4_46xx 0 
#endif   // STM32F4_46xx 

#ifndef STM32F4_69xx 
#define STM32F4_69xx 0 
#endif   // STM32F4_69xx 

#ifndef STM32F4_79xx 
#define STM32F4_79xx 0 
#endif   // STM32F4_79xx 

#ifndef STM32F4_12Cx 
#define STM32F4_12Cx 0 
#endif   // STM32F4_12Cx 

#ifndef STM32F4_12Zx 
#define STM32F4_12Zx 0 
#endif   // STM32F4_12Zx 

#ifndef STM32F4_12Rx 
#define STM32F4_12Rx 0 
#endif   // STM32F4_12Rx 

#ifndef STM32F4_12Vx 
#define STM32F4_12Vx 0 
#endif   // STM32F4_12Vx 

#ifndef STM32F4_13xx 
#define STM32F4_13xx 0 
#endif   // STM32F4_13xx 

#ifndef STM32F4_23xx 
#define STM32F4_23xx 0 
#endif   // STM32F4_23xx 

//==================================================

//=======================================================================================


//=======================================================================================
// Test modes (conditional compilation) 

// Toggle the mode for each device/peripheral/tool test 

//==================================================
// General 

#define HD44780U_ON_I2C_BUS 0             // Turn HD44780U screen off if on the same I2C 
                                          // bus as another device and not being used. 

//==================================================

//==================================================
// HD44780U 

#define HD44780U_CONTROLLER_TEST 0        // For switching between driver and controller testing 
#define HD44780U_BACKLIGHT_TEST 0         // Backlight control test 
#define HD44780U_DISPLAY_TEST 0           // Display on/off test 
#define HD44780U_CURSOR_TEST 0            // Cursor visibility test 
#define HD44780U_BLINK_TEST 0             // Cursor blink test 

//==================================================

//==================================================
// LSM303AGR 

// Only one mode is meant to be used at a time. The first macro under each mode is the 
// main/parent mode setter. Subsequent macros under the same mode are only used once the 
// mode is active. 

// Magnetometer axis data mode 
#define LSM303AGR_TEST_AXIS 0             // Magnetometer axis data read 

// Magnetometer heading mode 
#define LSM303AGR_TEST_HEADING 0          // Magnetometer heading read (compass) 
#define LSM303AGR_TEST_CALIBRATION 0      // Magnetometer heading calibration 

//==================================================

//==================================================
// M8Q 
//==================================================

//==================================================
// MPU-6050 driver test 

// Test control 
#define MPU6050_CONTROLLER_TEST 0        // Switch between driver and controller testing 
#define MPU6050_SECOND_DEVICE 0          // Include the test code for a second device 
#define MPU6050_INT_PIN 0                // Interrupt pin enable 

//==================================================

//==================================================
// nRF24L01 driver test 

// Device 
#define NRF24L01_SYSTEM_1 0              // Enable device 1 code 
#define NRF24L01_SYSTEM_2 0              // Enable device 2 code 

// Test code 
#define NRF24L01_HEARTBEAT 0             // Heartbeat 
#define NRF24L01_MANUAL_CONTROL 0        // Perform actions based on user input 

//==================================================

//=======================================================================================

#endif   // _SYSTEM_CONFIG_TEMPLATE_H_ 
