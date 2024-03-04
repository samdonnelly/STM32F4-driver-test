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
// Tests to add 
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
#include "int_handlers.h" 

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"

//=======================================================================================


//=======================================================================================
// Macros 

// Conditional compilation 
#define PERIODIC_BLINK_TEST 0   // Highest priority 
#define MANUAL_BLINK_TEST 0 
#define TASK_CONTROL_TEST 0 
#define TERMINAL_ECHO_TEST 1 
#define _TEST1 0 
#define _TEST2 0 
#define _TEST3 0 
#define _TEST4 0 
#define _TEST5 0 
#define _TEST6 0   // Lowest priority 

// Memory 
#define MAIN_LOOP_STACK_SIZE configMINIMAL_STACK_SIZE * 8 

// Data 
#define SERIAL_INPUT_MAX_LEN 30 

//==================================================
// Periodic Blink (PB) Test 

// There are two tasks that both toggle the board LED but at different rates. While not 
// toggling the LED state, each task is put into the blocking state. The main loop task 
// does nothing here. 

// Memory 
#define PB_STACK_SIZE configMINIMAL_STACK_SIZE * 4 

// Timing (number of ticks - default tick period is 1ms) 
#define PB_DELAY_1 500 
#define PB_DELAY_2 600 

//==================================================

//==================================================
// Manual Blink (MB) Test 

// There are two tasks used to control the blinking rate of an LED. The main loop task 
// will listen for user input on the serial terminal which indicates the blinking rate 
// of the LED (in ticks). If there is input then it will be read and converted to a 
// number and used to update the blink rate. The other task will toggle the LED state 
// at the rate set by the user input. 

// Memory 
#define MB_STACK_SIZE configMINIMAL_STACK_SIZE * 4 

// Data 
#define MB_MIN_LED_RATE 50 

//==================================================

//==================================================
// Task Control (TC) Test 

// There are two tasks that both print to the serial terminal. The first task is of a 
// lower priority and prints an arbitrary string at every 1 second. The second task is 
// of a higher priority and prints an asterisks every 100 ms. The main loop task controls 
// both of these tasks by first periodically suspending task 2, then after this is done 
// a few times task 1 is deleted so that only task 2 remains running. The messages should 
// be printed at a slow baud rate to better observe the preemptive nature of the RTOS. 

// Memory 
#define TC_STACK_SIZE configMINIMAL_STACK_SIZE * 4 

// Timing (number of ticks) 
#define TC_DELAY_1 2000   // Ticks 
#define TC_DELAY_2 1000   // Ticks 
#define TC_DELAY_3 100    // Ticks 
#define TC_DELAY_4 5000   // (ms) 

//==================================================

//==================================================
// Terminal Echo (TE) Test 

// This test uses two tasks to mimic a serial echo program. One task listens and records 
// input from the serial terminal. Once the end of the input is seen (new line or 
// carriage return), all the input up to that point gets stored in allocated heap memory 
// and informs the second task that there is new data. The second task prints the input 
// message back to there terminal when it becomes available and frees the memory used to 
// store the message. 

// Memory 
#define TE_STACK_SIZE configMINIMAL_STACK_SIZE * 4 

// Data 
#define MEM_STR_MAX_LEN 100 

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

// Serial terminal data 
static uint8_t uart_dma_buff[SERIAL_INPUT_MAX_LEN];   // Circular buffer 
static uint8_t buff_index = CLEAR;                    // Circular buffer index 
static uint8_t user_in_buff[SERIAL_INPUT_MAX_LEN];    // Stores latest user input 

#if PERIODIC_BLINK_TEST 

// Task definition: blink01 
osThreadId_t blink01Handle; 
const osThreadAttr_t blink01_attributes = 
{
    .name = "blink01", 
    .stack_size = PB_STACK_SIZE, 
    .priority = (osPriority_t) osPriorityNormal 
};

// Task definition: blink02 
osThreadId_t blink02Handle; 
const osThreadAttr_t blink02_attributes = 
{
    .name = "blink02",
    .stack_size = PB_STACK_SIZE,
    .priority = (osPriority_t) osPriorityBelowNormal 
};

// Board LED state - tasks will fight over the LED state 
static gpio_pin_state_t led_state = GPIO_LOW; 

#elif MANUAL_BLINK_TEST 

// Task definition: MB01 
osThreadId_t MB01Handle; 
const osThreadAttr_t MB01_attributes = 
{
    .name = "MB01", 
    .stack_size = MB_STACK_SIZE, 
    .priority = (osPriority_t) osPriorityNormal 
};

// Board LED blink rate 
static uint32_t mb_ticks = MB_MIN_LED_RATE; 

#elif TASK_CONTROL_TEST 

// Task definition: msg01 
osThreadId_t msg01Handle = NULL; 
const osThreadAttr_t msg01_attributes = 
{
    .name = "msg01", 
    .stack_size = TC_STACK_SIZE, 
    .priority = (osPriority_t) osPriorityBelowNormal 
};

// Task definition: msg02 
osThreadId_t msg02Handle = NULL; 
const osThreadAttr_t msg02_attributes = 
{
    .name = "msg02",
    .stack_size = TC_STACK_SIZE,
    .priority = (osPriority_t) osPriorityNormal 
};

// Strings 
const char msg[] = "Barkadeer brig Arr booty rum"; 

#elif TERMINAL_ECHO_TEST 

// Task definition: TE01 
osThreadId_t TE01Handle; 
const osThreadAttr_t TE01_attributes = 
{
    .name = "TE01", 
    .stack_size = TE_STACK_SIZE, 
    .priority = (osPriority_t) osPriorityNormal 
};

// Pointer used to locate heap allocated memory 
static char *user_msg = NULL; 

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

/**
 * @brief Task function: MB01 
 * 
 * @param argument : NULL 
 */
void TaskMB01(void *argument); 

#elif TASK_CONTROL_TEST 

/**
 * @brief Task function: msg01 
 * 
 * @param argument : NULL 
 */
void TaskMsg01(void *argument); 


/**
 * @brief Task function: msg02 
 * 
 * @param argument : NULL 
 */
void TaskMsg02(void *argument); 

#elif TERMINAL_ECHO_TEST 

/**
 * @brief Task function: TE01 
 * 
 * @param argument : NULL 
 */
void TaskTE01(void *argument); 

#endif

//=======================================================================================


//=======================================================================================
// Setup code

void freertos_test_init(void)
{
    //==================================================
    // General setup 

    // Initialize GPIO ports 
    gpio_port_init(); 

    // General purpose timer 
    tim_9_to_11_counter_init(
        TIM9, 
        TIM_84MHZ_1US_PSC, 
        0xFFFF,  // Max ARR value 
        TIM_UP_INT_DISABLE); 
    tim_enable(TIM9); 

    // Initialize board LED (on when logic low) 
    gpio_pin_init(GPIOA, PIN_5, MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 

    // Initialize UART 
    uart_init(
        USART2, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_FRAC_42_9600, 
        UART_MANT_42_9600, 
        UART_DMA_DISABLE, 
        UART_DMA_ENABLE);   // RX DMA enabled for serial terminal reading 
    
    // Enable IDLE line interrupts for reading serial terminal input with DMA 
    uart_interrupt_init(
        USART2, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE, 
        UART_INT_ENABLE, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE); 

    // Initialize the DMA stream 
    dma_stream_init(
        DMA1, 
        DMA1_Stream5, 
        DMA_CHNL_4, 
        DMA_DIR_PM, 
        DMA_CM_ENABLE,
        DMA_PRIOR_VHI, 
        DMA_ADDR_INCREMENT,   // Increment the buffer pointer to fill the buffer 
        DMA_ADDR_FIXED,       // No peripheral increment - copy from DR only 
        DMA_DATA_SIZE_BYTE, 
        DMA_DATA_SIZE_BYTE); 

    // Configure and enable the DMA stream 
    dma_stream_config(
        DMA1_Stream5, 
        (uint32_t)(&USART2->DR), 
        (uint32_t)uart_dma_buff, 
        (uint16_t)SERIAL_INPUT_MAX_LEN); 
    dma_stream_enable(DMA1_Stream5); 

    // Initialize interrupt handler flags and enable the interrupt handler 
    int_handler_init(); 
    nvic_config(USART2_IRQn, EXTI_PRIORITY_0); 

    // Initialize FreeRTOS scheduler 
    osKernelInitialize(); 

    // Create the main loop thread 
    mainLoopHandle = osThreadNew(TaskLoop, NULL, &main_loop_attributes); 

    // Initialize data 
    memset((void *)uart_dma_buff, CLEAR, sizeof(uart_dma_buff)); 
    memset((void *)user_in_buff, CLEAR, sizeof(user_in_buff)); 

    //==================================================

#if PERIODIC_BLINK_TEST 

    // Turn the board LED off 
    gpio_write(GPIOA, GPIOX_PIN_5, led_state); 

    // Create the thread(s) 
    blink01Handle = osThreadNew(TaskBlink01, NULL, &blink01_attributes); 
    blink02Handle = osThreadNew(TaskBlink02, NULL, &blink02_attributes); 
    
#elif MANUAL_BLINK_TEST 

    // Create the thread(s) 
    MB01Handle = osThreadNew(TaskMB01, NULL, &MB01_attributes); 

    uart_sendstring(USART2, "\r\n>>> "); 

#elif TASK_CONTROL_TEST 

    // Reinitialize UART (serial terminal output) for a slower baud rate 
    uart_init(
        USART2, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_FRAC_42_1200, 
        UART_MANT_42_1200, 
        UART_DMA_DISABLE, 
        UART_DMA_DISABLE); 

    // Create the thread(s) 
    msg01Handle = osThreadNew(TaskMsg01, NULL, &msg01_attributes); 
    msg02Handle = osThreadNew(TaskMsg02, NULL, &msg02_attributes); 

    // Blocking delay to provide time for the user to connect to the serial terminal 
    tim_delay_ms(TIM9, TC_DELAY_4); 

#elif TERMINAL_ECHO_TEST 

    // Create the thread(s) 
    TE01Handle = osThreadNew(TaskTE01, NULL, &TE01_attributes); 
    osThreadSuspend(TE01Handle);   // Suspend to prevent running right away 

    uart_sendstring(USART2, "\r\n>>> "); 
    
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

        // This interrupt flag will be set when an idle line is detected on UART RX after 
        // receiving new data. 
        if (handler_flags.usart2_flag)
        {
            handler_flags.usart2_flag = CLEAR; 

            // Get the user input and update the LED blink rate 
            cb_parse(uart_dma_buff, user_in_buff, &buff_index, SERIAL_INPUT_MAX_LEN); 
            mb_ticks = (uint32_t)strtol((char *)user_in_buff, NULL, 10); 

            // Make sure rate can't go below a certain threshold to prevent this task 
            // from never running. 
            if (mb_ticks < MB_MIN_LED_RATE)
            {
                mb_ticks = MB_MIN_LED_RATE; 
            }

            uart_sendstring(USART2, "\r\n>>> "); 
        }
        
#elif TASK_CONTROL_TEST 

        // Suspend the higher priority task for some intervals 
        for (uint8_t i = CLEAR; i < 3; i++)
        {
            osThreadSuspend(msg02Handle); 
            osDelay(TC_DELAY_1); 
            osThreadResume(msg02Handle); 
            osDelay(TC_DELAY_1); 
        }
        
        // Delete the lower priority task 
        if (msg01Handle != NULL)
        {
            osThreadTerminate(msg01Handle); 
            msg01Handle = NULL; 
        }

#elif TERMINAL_ECHO_TEST 

        // This interrupt flag will be set when an idle line is detected on UART RX after 
        // receiving new data. 
        if (handler_flags.usart2_flag)
        {
            handler_flags.usart2_flag = CLEAR; 

            uint8_t user_in_buff_local[SERIAL_INPUT_MAX_LEN]; 
            uint32_t input_len; 
            uint8_t mem_info[MEM_STR_MAX_LEN]; 

            // Get the user input from the circular buffer 
            cb_parse(uart_dma_buff, user_in_buff_local, &buff_index, SERIAL_INPUT_MAX_LEN); 

            snprintf(
                (char *)mem_info, 
                MEM_STR_MAX_LEN, 
                "Free task stack (words): %lu\r\nFree heap before malloc (bytes): %lu\r\n", 
                (uint32_t)uxTaskGetStackHighWaterMark(NULL), 
                (uint32_t)xPortGetFreeHeapSize()); 
            uart_sendstring(USART2, (char *)mem_info); 

            // Store the input in heap memory. 'input_len' is made one longer than strlen 
            // provides so that the null termination of 'user_in_buff_local' will be 
            // copied to the heap and print properly to the serial terminal. 
            input_len = (uint32_t)strlen((char *)user_in_buff_local) + 1; 
            user_msg = (char *)pvPortMalloc(input_len); 

            if (user_msg != NULL)
            {
                memcpy((void *)user_msg, (void *)user_in_buff_local, input_len); 
            }

            snprintf(
                (char *)mem_info, 
                MEM_STR_MAX_LEN, 
                "Free heap after malloc (bytes): %lu\r\n", 
                (uint32_t)xPortGetFreeHeapSize()); 
            uart_sendstring(USART2, (char *)mem_info); 

            osThreadResume(TE01Handle); 
        }

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
        blink_led_toggle(PB_DELAY_1); 
    }

    osThreadTerminate(NULL); 
}


// Task function: blink02 
void TaskBlink02(void *argument)
{
    while (1)
    { 
        blink_led_toggle(PB_DELAY_2); 
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

// Task function: MB01 
void TaskMB01(void *argument)
{
    // Board LED state 
    gpio_pin_state_t led_state = GPIO_LOW; 

    while (1)
    {
        led_state = GPIO_HIGH - led_state; 
        gpio_write(GPIOA, GPIOX_PIN_5, led_state); 
        osDelay(mb_ticks); 
    }

    osThreadTerminate(NULL); 
}

#elif TASK_CONTROL_TEST 

// Task function: msg01 
void TaskMsg01(void *argument)
{
    // Print the string to the terminal one character at a time. 
    while (1)
    {
        uart_send_new_line(USART2); 
        uart_sendstring(USART2, msg); 
        uart_send_new_line(USART2); 
        osDelay(TC_DELAY_2); 
    }

    osThreadTerminate(NULL); 
}


// Task function: msg02 
void TaskMsg02(void *argument)
{
    while (1)
    {
        uart_sendchar(USART2, AST_CHAR); 
        osDelay(TC_DELAY_3); 
    }

    osThreadTerminate(NULL); 
}

#elif TERMINAL_ECHO_TEST 

// Task function: TE01 
void TaskTE01(void *argument)
{
    while (1)
    {
        // Echo the user input back to the serial terminal 
        uart_sendstring(USART2, "Echo: "); 
        uart_sendstring(USART2, user_msg); 
        uart_sendstring(USART2, "\r\n\n>>> "); 

        // Free the heap memory and suspend the task 
        vPortFree(user_msg); 
        user_msg = NULL; 
        osThreadSuspend(TE01Handle); 
    }

    osThreadTerminate(NULL); 
}

#endif

//=======================================================================================
