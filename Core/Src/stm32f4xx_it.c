/**
 * @file stm32f4xx_it.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Interrupt Service Routines (ISRs) 
 * 
 * @version 0.1
 * @date 2024-02-25
 * 
 * @copyright Copyright (c) 2024
 * 
 */

//=======================================================================================
// Includes 

#include "stm32f4xx_it.h"
#include "stm32f4xx_hal.h"
#include "system_settings.h"

#if FREERTOS_ENABLE 
#include "stm32f4xx_hal_tim.h" 
#endif   // FREERTOS_ENABLE 

//=======================================================================================


//=======================================================================================
// Variables 

#if FREERTOS_ENABLE 
extern TIM_HandleTypeDef htim11; 
#endif   // FREERTOS_ENABLE 

//=======================================================================================


//=======================================================================================
// Interrupts 

//==================================================
// Cortex-M4 Processor Interruption and Exception Handlers 

/**
 * @brief Non-maskable interrupt handler 
 */
void NMI_Handler(void)
{
    while (1) {}
}

/**
 * @brief Hard fault interrupt handler 
 */
void HardFault_Handler(void)
{
    while (1) {}
}

/**
 * @brief Memory management fault handler 
 */
void MemManage_Handler(void)
{
    while (1) {}
}

/**
 * @brief Pre-fetch fault, memory access fault handler 
 */
void BusFault_Handler(void)
{
    while (1) {}
}

/**
 * @brief Undefined instruction or illegal state handler 
 */
void UsageFault_Handler(void)
{
    while (1) {}
}

/**
 * @brief Debug monitor handler 
 */
void DebugMon_Handler(void)
{
    // 
}

#if !FREERTOS_ENABLE 

/**
 * @brief This function handles System service call via SWI instruction.
 */
void SVC_Handler(void)
{
    // 
}

/**
 * @brief This function handles Pendable request for system service.
 */
void PendSV_Handler(void)
{
    // 
}

/**
 * @brief This function handles System tick timer.
 */
void SysTick_Handler(void)
{
    // HAL timer counter increment 
    HAL_IncTick();
}

#endif   // !FREERTOS_ENABLE 

//==================================================

//==================================================
// STM32F4xx Peripheral Interrupt Handlers 

// For the available peripheral interrupt handler names, please refer to the startup 
// file (startup_stm32f4xx.s). 

#if FREERTOS_ENABLE 

/**
 * @brief This function handles TIM1 trigger and commutation interrupts and TIM11 global 
 *        interrupt.
 */
void TIM1_TRG_COM_TIM11_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim11);
}

#endif   // FREERTOS_ENABLE 

//==================================================

//=======================================================================================
