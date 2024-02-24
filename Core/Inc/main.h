/**
 * @file main.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Header for main.c file 
 * 
 * @version 0.1
 * @date 2024-02-23
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes 

#include "stm32f4xx_hal.h"

//=======================================================================================


//=======================================================================================
// Macros 

// Conditional compilation 
#define FREERTOS_ENABLE 1 

// Ports and pins 
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB

//=======================================================================================


//=======================================================================================
// Prototypes 

/**
 * @brief This function is executed in case of error occurrence 
 */
void Error_Handler(void);

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // __MAIN_H 
