/**
 * @file freertos_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief FreeRTOS setup and test implementation 
 * 
 * @details Basic applications of FreeRTOS features are written in this test to show its 
 *          capabilities. 
 * 
 * @version 0.1
 * @date 2024-02-20
 * 
 * @copyright Copyright (c) 2024
 * 
 */

//=======================================================================================
// Notes 
// - "osDelay" allows the scheduler to run other tasks while we wait. 
// - Adding "osThreadTerminate()" after the forever loop in the tasks is a good idea in 
//   case the task loop accidentally exits. This function will terminate and clean up the 
//   thread. 
// - When FreeRTOS is used, SysTick is left to FreeRTOS and HAL has a new timer assigned 
//   to it. If, for example, TIM11 is used for the HAL time base then the time base 
//   counter (uwTick) is incremented through the following chain of events: 
//   - interrupt from TIM11 --> TIM1_TRG_COM_TIM11_IRQHandler --> HAL_TIM_IRQHandler 
//     --> HAL_TIM_PeriodElapsedCallback --> HAL_IncTick 
//=======================================================================================


//=======================================================================================
// Includes 

#include "freertos_test.h" 

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"

//=======================================================================================


//=======================================================================================
// Macros 

#define BLINK_STACK_SIZE 128 
#define WORD_SIZE 4 

//=======================================================================================


//=======================================================================================
// Variables 

//==================================================
// Task definitions 

// blink01 
osThreadId_t blink01Handle;
const osThreadAttr_t blink01_attributes = 
{
    .name = "blink01",
    .stack_size = BLINK_STACK_SIZE * WORD_SIZE,
    .priority = (osPriority_t) osPriorityNormal,
};

// blink02 
osThreadId_t blink02Handle;
const osThreadAttr_t blink02_attributes = 
{
    .name = "blink02",
    .stack_size = BLINK_STACK_SIZE * WORD_SIZE,
    .priority = (osPriority_t) osPriorityBelowNormal,
};

//==================================================

// Board LED state - tasks will fight over the LED state 
static gpio_pin_state_t led_state = GPIO_LOW; 

//=======================================================================================


//=======================================================================================
// Prototypes 

//==================================================
// Tasks 

// Blink 01 
void StartBlink01(void *argument); 

// Blink 02 
void StartBlink02(void *argument); 

//==================================================


//==================================================
// Helper functions 

// LED toggle for blink example 
void blink_led_toggle(uint32_t ms); 

//==================================================

//=======================================================================================


//=======================================================================================
// Setup code

void freertos_test_init()
{
    // Initialize GPIO ports 
    gpio_port_init(); 

    // Initialize board LED (on when logic low) 
    gpio_pin_init(GPIOA, PIN_5, MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 
    gpio_write(GPIOA, GPIOX_PIN_5, led_state); 

    // Initialize FreeRTOS scheduler 
    osKernelInitialize(); 

    // Create the thread(s) 
    blink01Handle = osThreadNew(StartBlink01, NULL, &blink01_attributes); 
    blink02Handle = osThreadNew(StartBlink02, NULL, &blink02_attributes); 
} 

//=======================================================================================


//=======================================================================================
// Test code 

void freertos_test_app()
{
    // Start scheduler. From this point on, execution is handled by the scheduler and 
    // only code within the tasks below is run, meaning the code does not pass through 
    // the main while(1) loop anymore. 
    osKernelStart(); 
}

//=======================================================================================


//=======================================================================================
// Test functions 

//==================================================
// Tasks 

/**
 * @brief Function implementing the blink01 thread 
 * 
 * @param argument 
 */
void StartBlink01(void *argument)
{
    while (1)
    {
        blink_led_toggle(500); 
    }

    osThreadTerminate(NULL); 
}


/**
 * @brief Function implementing the blink02 thread 
 * 
 * @param argument 
 */
void StartBlink02(void *argument)
{
    while (1)
    { 
        blink_led_toggle(600); 
    }

    osThreadTerminate(NULL); 
}

//==================================================


//==================================================
// Helper functions 

// LED toggle for blink example 
void blink_led_toggle(uint32_t ms)
{
    led_state = GPIO_HIGH - led_state; 
    gpio_write(GPIOA, GPIOX_PIN_5, led_state); 
    osDelay(ms);   // milliseconds 
}

//==================================================

//=======================================================================================
