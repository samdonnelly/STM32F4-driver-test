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
// - The application/loop function below can be set up as it's own task to do other 
//   jobs such as controlling other tasks. This way you can still kind of have a main 
//   loop. Just have to be mindful of the priority given to it. 
// - Make sure vTaskDelete is not called on a NULL pointed task handle. 
// - The function uxTaskGetStackHighWaterMark will return the number of words that are 
//   left in the tasks stack. 
// - The function xPortGetFreeHeapSize will return the total amount of heap memory in 
//   bytes that's available. 
//=======================================================================================


//=======================================================================================
// Tests to add 
// 1. Two tasks, both print messages to serial terminal but messages are different and 
//    get written at different intervals. The second task has a higher priority than the 
//    first task. Use a slower baud rate to better see the contect switching. There will 
//    be a third task which is the main loop that will periodically suspend task 2. 
// 
// 2. Two tasks to control the blinking rate of an LED. One task will listen for input 
//    on the serial terminal. When the user enters a number, the delay time on the 
//    blinking LED will be updated to that time. 
// 
// 3. Two tasks that mimic a serial echo program. One task listens for input from the 
//    serial monitor. Once it sees a new line character it stores all input up to that 
//    point in newly allocated heap memory then notifies the second task of a new 
//    message. Task 2 waits for notice from task 1 then prints the new message (that's 
//    stored in heap memeory) to the serial monitor and frees the memory. 
//=======================================================================================


//=======================================================================================
// Includes 

#include "freertos_test.h" 
#include "includes_drivers.h" 

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"

//=======================================================================================


//=======================================================================================
// Macros 

// Conditional compilation 
#define PERIODIC_BLINK_TEST 1 
#define MANUAL_BLINK_TEST 0 
#define TASK_CONTROL_TEST 0 

// Memory 
#define MAIN_LOOP_STACK_SIZE configMINIMAL_STACK_SIZE * 4 

//==================================================
// Periodic Blink Test 

// Memory 
#define BLINK_STACK_SIZE configMINIMAL_STACK_SIZE * 4 

// Timing (number of ticks - default tick period is 1ms) 
#define BLINK_DELAY_1 500 
#define BLINK_DELAY_2 600 

//==================================================

//==================================================
// Manual Blink Test 
//==================================================

//==================================================
// Task Control Test 
//==================================================

//=======================================================================================


//=======================================================================================
// Variables 

// Task definition: main loop 
osThreadId_t mainLoopHandle; 
const osThreadAttr_t main_loop_attributes = 
{
    .name = "main_loop", 
    .stack_size = MAIN_LOOP_STACK_SIZE, 
    .priority = (osPriority_t) osPriorityLow   // Above Idle but below everything else 
}; 


#if PERIODIC_BLINK_TEST 

// Task definition: blink01 
osThreadId_t blink01Handle;
const osThreadAttr_t blink01_attributes = 
{
    .name = "blink01", 
    .stack_size = BLINK_STACK_SIZE, 
    .priority = (osPriority_t) osPriorityNormal 
};

// Task definition: blink02 
osThreadId_t blink02Handle;
const osThreadAttr_t blink02_attributes = 
{
    .name = "blink02",
    .stack_size = BLINK_STACK_SIZE,
    .priority = (osPriority_t) osPriorityBelowNormal 
};

// Board LED state - tasks will fight over the LED state 
static gpio_pin_state_t led_state = GPIO_LOW; 

#elif MANUAL_BLINK_TEST 
#elif TASK_CONTROL_TEST 
#endif 

//=======================================================================================


//=======================================================================================
// Prototypes 

/**
 * @brief Task function: main loop 
 * 
 * @param argument : NULL 
 */
void TaskLoop(void *argument); 


#if PERIODIC_BLINK_TEST 

/**
 * @brief Task function: blink01 
 * 
 * @param argument : NULL 
 */
void TaskBlink01(void *argument); 


/**
 * @brief Task function: blink02 
 * 
 * @param argument : NULL 
 */
void TaskBlink02(void *argument); 


/**
 * @brief LED toggle 
 * 
 * @param ticks : delay between LED toggling (ticks). See notes for info on ticks. 
 */
void blink_led_toggle(uint32_t ticks); 

#elif MANUAL_BLINK_TEST 
#elif TASK_CONTROL_TEST 
#endif

//=======================================================================================


//=======================================================================================
// Setup code

void freertos_test_init(void)
{
    // Initialize FreeRTOS scheduler 
    osKernelInitialize(); 

    // Create the main loop thread 
    mainLoopHandle = osThreadNew(TaskLoop, NULL, &main_loop_attributes); 

#if PERIODIC_BLINK_TEST 

    // Initialize GPIO ports 
    gpio_port_init(); 

    // Initialize board LED (on when logic low) 
    gpio_pin_init(GPIOA, PIN_5, MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 
    gpio_write(GPIOA, GPIOX_PIN_5, led_state); 

    // Create the thread(s) 
    blink01Handle = osThreadNew(TaskBlink01, NULL, &blink01_attributes); 
    blink02Handle = osThreadNew(TaskBlink02, NULL, &blink02_attributes); 
    
#elif MANUAL_BLINK_TEST 
#elif TASK_CONTROL_TEST 
#endif
} 

//=======================================================================================


//=======================================================================================
// Test code 

void freertos_test_app(void)
{
    // Start scheduler. From this point on, execution is handled by the scheduler and 
    // only code within the tasks is run, meaning the code does not pass through the main 
    // while(1) loop anymore. 
    osKernelStart(); 
}


// Task function: main loop 
void TaskLoop(void *argument)
{
#if PERIODIC_BLINK_TEST 

    // Do nothing 

#elif MANUAL_BLINK_TEST 
#elif TASK_CONTROL_TEST 
#endif
}

//=======================================================================================


//=======================================================================================
// Test functions 

#if PERIODIC_BLINK_TEST 

// Task function: blink01 
void TaskBlink01(void *argument)
{
    while (1)
    {
        blink_led_toggle(BLINK_DELAY_1); 
    }

    osThreadTerminate(NULL); 
}


// Task function: blink02 
void TaskBlink02(void *argument)
{
    while (1)
    { 
        blink_led_toggle(BLINK_DELAY_2); 
    }

    osThreadTerminate(NULL); 
}


// LED toggle 
void blink_led_toggle(uint32_t ticks)
{
    led_state = GPIO_HIGH - led_state; 
    gpio_write(GPIOA, GPIOX_PIN_5, led_state); 
    osDelay(ticks);   // Delays the specified number of ticks 
}

#elif MANUAL_BLINK_TEST 
#elif TASK_CONTROL_TEST 
#endif

//=======================================================================================
