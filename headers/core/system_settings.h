/**
 * @file system_settings.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief System configuration settings 
 * 
 * @version 0.1
 * @date 2024-02-25
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef _SYSTEM_SETTINGS_H_ 
#define _SYSTEM_SETTINGS_H_ 

//=======================================================================================
// Includes 
//=======================================================================================


//=======================================================================================
// Macros 

//==================================================
// Conditional compilation 

// This needs to be set if using FreeRTOS as it includes additional files and alters 
// some core code that's used with FreeRTOS. Note that the 'RTOS_ENABLE' variable in 
// CMakeLists must be updated to match this macro. 
#define FREERTOS_ENABLE 0 

// This includes interrupt handler definitions that override defaults. Overrides will 
// be used for specific tests. If interrupts are used for a test but the test has no 
// specific override then this should be cleared. 
#define INTERRUPT_OVERRIDE 0 

//==================================================

//=======================================================================================

#endif   // _SYSTEM_SETTINGS_H_ 
