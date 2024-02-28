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
// - A tick timer is one of the hardware timers allocated to interrupt the processor at a 
//   specific interval, or "time slice". This time slice is known as a tick. By default, 
//   FreeRTOS sets the tick period to 1ms. The operating system must run at each time 
//   slice to identify which task to schedule next which could be a new task or the same 
//   task. 
// - vTaskDelay (called by osDelay) expects the number of ticks to delay, not the number 
//   of milliseconds. 
// - The minimum stack size is the size needed to run an empty task and handle scheduler 
//   overhead. 
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

// Memory 
#define BLINK_STACK_SIZE configMINIMAL_STACK_SIZE * 4 

// Timing (number of ticks - default tick period is 1ms) 
#define BLINK_DELAY_1 500 
#define BLINK_DELAY_2 600 

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
    .stack_size = BLINK_STACK_SIZE, 
    .priority = (osPriority_t) osPriorityNormal, 
};

// blink02 
osThreadId_t blink02Handle;
const osThreadAttr_t blink02_attributes = 
{
    .name = "blink02",
    .stack_size = BLINK_STACK_SIZE,
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

/**
 * @brief Blink01 thread function 
 * 
 * @param argument : NULL 
 */
void TaskBlink01(void *argument); 


/**
 * @brief Blink02 thread function 
 * 
 * @param argument : NULL 
 */
void TaskBlink02(void *argument); 

//==================================================


//==================================================
// Helper functions 

/**
 * @brief LED toggle for blink example 
 * 
 * @param ticks : delay between LED toggling (ticks). See notes for info on ticks. 
 */
void blink_led_toggle(uint32_t ticks); 

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
    blink01Handle = osThreadNew(TaskBlink01, NULL, &blink01_attributes); 
    blink02Handle = osThreadNew(TaskBlink02, NULL, &blink02_attributes); 
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
// Tasks 

// Blink01 thread function 
void TaskBlink01(void *argument)
{
    while (1)
    {
        blink_led_toggle(BLINK_DELAY_1); 
    }

    osThreadTerminate(NULL); 
}


// Blink02 thread function 
void TaskBlink02(void *argument)
{
    while (1)
    { 
        blink_led_toggle(BLINK_DELAY_2); 
    }

    osThreadTerminate(NULL); 
}

//=======================================================================================


//=======================================================================================
// Helper functions 

// LED toggle for blink example 
void blink_led_toggle(uint32_t ticks)
{
    led_state = GPIO_HIGH - led_state; 
    gpio_write(GPIOA, GPIOX_PIN_5, led_state); 
    osDelay(ticks);   // Delays the specified number of ticks 
}

//=======================================================================================
