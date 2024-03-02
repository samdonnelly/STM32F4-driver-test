/**
 * @file freertos_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief FreeRTOS practice implementation 
 * 
 * @details The examples/tests implemented here are from the "Introduction to RTOS" video 
 *          playlist by DigiKey: 
 *          https://www.youtube.com/playlist?list=PLEBQazB0HUyQ4hAPU1cJED6t3DU0h34bz 
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
// - As long as you use the built-in kernal functions then writing to a queue is atomic 
//   meaning another task cannot interrupt it during the writing process. 
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
// 
// 4. Two tasks and two queues. Task A should print any new messages from queue 2 to the 
//    serial terminal, read serial input from the user, echo input back to the serial 
//    terminal and send values to queue 1 where values come from a user input that reads 
//    "delay <value>". Task B should update a delay variable with values from queue 1, 
//    blink the board LED with the delay value, and send a string to queue 2 every 100 
//    LED blinks along with a counter to indicate the number of times it's blinked 100 
//    times. 
// 
// 5. Two tasks that access the same function and therefore the same global variable. The 
//    function increments the global variable. There is a mutex in the function that 
//    prevents either task from running while the other is incrementing the cariable. 
// 
// 6. Seven tasks, 5 of which are producers that add values to a circular buffer (shared 
//    resource) and 2 tasks which are consumers that read from the buffer. The producer 
//    tasks write their task number to the buffer 3 times. Semaphores and mutexes are 
//    used to to protect the shared circular buffer. The consumer tasks print out 
//    anything read from the buffer to the serial terminal. 
// 
// 7. Enter characters in the serial terminal. When the code sees characters being 
//    entered it will turn the board LED on. The board LED will only turn off after there 
//    has been no serial terminal input for a period of time (say 5 seconds). Use 
//    software timers to accomplish the task. 
//    - Note: the xTimerStart function will restart a counter if it's called before the 
//            timer expires. 
// 
// 8. Use a hardware timer (ISR) to sample ADC 10 times per second. The values get placed 
//    in a double or circular buffer. Once 10 samples have been collected, the ISR should 
//    wake up task A which computes the average of the 10 samples. A double or circular 
//    buffer is recommended so data can be read/used at the same time new data gets 
//    written using the ISR. The average should a be floating point value stored in a 
//    global variable. Assume this global variable cannot be written to or read from in 
//    a single CPU cycle (critical section). Task B will output to the serial terminal 
//    the global variable value is "avg" is entered by the user, otherwise the input is 
//    exhoed back. 
// 
// 10. Deadlock test. 
// 
// 11. Priority inversion test. 
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
#define _TEST0 0 
#define _TEST1 0 
#define _TEST2 0 
#define _TEST3 0 
#define _TEST4 0 
#define _TEST5 0 
#define _TEST6 0 

// Memory 
#define MAIN_LOOP_STACK_SIZE configMINIMAL_STACK_SIZE * 4 

//==================================================
// Periodic Blink Test 
// There are two tasks that both toggle the board LED but at different rates. While not 
// toggling the LED state, each task is put into the blocking state. The main loop task 
// does nothing here. 

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
    while (1)
    {
#if PERIODIC_BLINK_TEST 
    // Do nothing 
#elif MANUAL_BLINK_TEST 
#elif TASK_CONTROL_TEST 
#endif
    }

    osThreadTerminate(NULL); 
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
