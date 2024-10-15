/**
 * @file includes_app.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Includes for C test code 
 * 
 * @version 0.1
 * @date 2022-02-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _INCLUDES_APP_H_
#define _INCLUDES_APP_H_

//=======================================================================================
// Includes 

// Application test code 
#include "freertos_test.h" 
#include "wheel_rpm_test.h" 

// Device test code 
#include "esc_readytosky_test.h"
#include "hc05_test.h"
#include "hd44780u_test.h"
#include "hw125_test.h"
#include "lsm303agr_test.h"
#include "m8q_test.h"
#include "mpu6050_test.h"
#include "nrf24l01_test.h"
#include "ws2812_test.h" 

// Peripheral test code 
#include "analog_test.h"
#include "dma_test.h"
#include "gpio_test.h"
#include "interrupt_test.h"
#include "timers_test.h"
#include "uart_test.h"

// Tool test code 
#include "state_machine_test.h" 
#include "switch_debounce_test.h" 

//=======================================================================================

#endif  // _INCLUDES_APP_H_
