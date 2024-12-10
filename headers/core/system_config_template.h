/**
 * @file system_config_template.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief System configuration template 
 * 
 * @details This file is not to be altered unless there is a system configuration setting 
 *          being added or removed. This means these settings should not be used to 
 *          configure your system. Instead, create a "system_config.h" file in the same 
 *          folder as this file and define the macros below in there and use those to 
 *          configure your system. "system_config.h" is not tracked so it's specific to 
 *          your use case. 
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

/**
 * NOTE: Do not change the setting below to configure your system. This file should only 
 *       be modified if settings are being added or removed. The settings below are 
 *       meant to be overwritten so you can configure your system to your specifc use 
 *       case. To do this, create a "system_config.h" file in the same folder as this 
 *       file and define each of the below settings without the "ifndef" guards. 
 *       "system_config.h" is not tracked so it can be changed for each system. 
 */

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

// The below list allows for choosing which STM32F4 board to use. You can define only 
// what you need in the "system_config.h" file. 
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

//=======================================================================================

#endif   // _SYSTEM_CONFIG_TEMPLATE_H_ 
