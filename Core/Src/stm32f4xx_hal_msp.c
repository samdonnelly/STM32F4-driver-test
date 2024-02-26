/**
 * @file stm32f4xx_hal_msp.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief MSP Initialization and de-Initialization implementation 
 * 
 * @version 0.1
 * @date 2024-02-25
 * 
 * @copyright Copyright (c) 2024
 * 
 */

//=======================================================================================
// Includes 

#include "stm32f4xx_hal.h"
#include "system_settings.h"

//=======================================================================================


//=======================================================================================
// Functions 

/**
 * @brief Initializes the global MSP 
 */
void HAL_MspInit(void)
{
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

#if FREERTOS_ENABLE 
    // PendSV_IRQn interrupt configuration 
    HAL_NVIC_SetPriority(PendSV_IRQn, 15, 0); 
#else   // FREERTOS_ENABLE
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_0);  
#endif   // FREERTOS_ENABLE 

    // System interrupt init 
}

//=======================================================================================
