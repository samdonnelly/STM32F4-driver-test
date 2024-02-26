/**
 * @file stm32f4xx_it.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Interrupt Service Routines (ISRs) interface 
 * 
 * @version 0.1
 * @date 2024-02-25
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef _STM32F4xx_IT_H_
#define _STM32F4xx_IT_H_

#ifdef __cplusplus
 extern "C" {
#endif

//=======================================================================================
// Includes 
//=======================================================================================


//=======================================================================================
// Prototypes 

void NMI_Handler(void); 
void HardFault_Handler(void); 
void MemManage_Handler(void); 
void BusFault_Handler(void); 
void UsageFault_Handler(void); 
void DebugMon_Handler(void); 

#if !FREERTOS_ENABLE 
void SVC_Handler(void); 
void PendSV_Handler(void); 
void SysTick_Handler(void); 
#endif   // !FREERTOS_ENABLE 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // _STM32F4xx_IT_H_ 
