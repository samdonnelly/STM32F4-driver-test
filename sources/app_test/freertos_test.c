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
// Includes 

#include "freertos_test.h" 
#include "includes_drivers.h" 
#include "stm32f4xx_it.h" 

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include "queue.h" 
#include "semphr.h" 
#include "timers.h" 

//=======================================================================================


//=======================================================================================
// Macros 

// Conditional compilation 
#define PERIODIC_BLINK_TEST 1        // Highest priority 
#define MANUAL_BLINK_TEST 0 
#define TASK_SCHEDULING_TEST 0 
#define MEMORY_MANAGEMENT_TEST 0 
#define QUEUE_TEST 0 
#define MUTEX_TEST 0 
#define SEMAPHORE_TEST 0 
#define SOFTWARE_TIMER_TEST_0 0 
#define SOFTWARE_TIMER_TEST_1 0 
#define HARDWARE_INTERRUPT_TEST 0 
#define DEADLOCK_STARVATION_TEST 0 
#define PRIORITY_INVERSION_TEST 0    // Lowest priority 

//==================================================
// Main task loop 

// Memory 
#define MAIN_LOOP_STACK_SIZE configMINIMAL_STACK_SIZE * 8 

// Data 
#define SERIAL_INPUT_MAX_LEN 30 

//==================================================

//=======================================================================================


//=======================================================================================
// Prototypes 

// Setup code 
void periodic_blink_init(void); 
void manual_blink_init(void); 
void task_scheduling_init(void); 
void memory_management_init(void); 
void queue_init(void); 
void mutex_init(void); 
void semaphore_init(void); 
void software_timer_0_init(void); 
void software_timer_1_init(void); 
void hardware_interrupt_init(void); 
void deadlock_starvation_init(void); 
void priority_inversion_init(void); 

// Main task loop code 
void periodic_blink_loop(void); 
void manual_blink_loop(void); 
void task_scheduling_loop(void); 
void memory_management_loop(void); 
void queue_loop(void); 
void mutex_loop(void); 
void semaphore_loop(void); 
void software_timer_0_loop(void); 
void software_timer_1_loop(void); 
void hardware_interrupt_loop(void); 
void deadlock_starvation_loop(void); 
void priority_inversion_loop(void); 

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
static uint8_t buff_index;                            // Circular buffer index 
static uint8_t user_in_buff[SERIAL_INPUT_MAX_LEN];    // Stores latest user input 

//=======================================================================================


//=======================================================================================
// Prototypes 

/**
 * @brief Task function: main loop 
 * 
 * @param argument : NULL 
 */
void TaskLoop(void *argument); 

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
    memset((void *)&buff_index, CLEAR, sizeof(buff_index)); 
    memset((void *)user_in_buff, CLEAR, sizeof(user_in_buff)); 

    //==================================================

#if PERIODIC_BLINK_TEST 
    periodic_blink_init(); 
#elif MANUAL_BLINK_TEST 
    manual_blink_init(); 
#elif TASK_SCHEDULING_TEST 
    task_scheduling_init(); 
#elif MEMORY_MANAGEMENT_TEST 
    memory_management_init(); 
#elif QUEUE_TEST 
    queue_init(); 
#elif MUTEX_TEST 
    mutex_init(); 
#elif SEMAPHORE_TEST 
    semaphore_init(); 
#elif SOFTWARE_TIMER_TEST_0 
    software_timer_0_init(); 
#elif SOFTWARE_TIMER_TEST_1 
    software_timer_1_init(); 
#elif HARDWARE_INTERRUPT_TEST 
    hardware_interrupt_init(); 
#elif DEADLOCK_STARVATION_TEST 
    deadlock_starvation_init(); 
#elif PRIORITY_INVERSION_TEST 
    priority_inversion_init(); 
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
        periodic_blink_loop(); 
#elif MANUAL_BLINK_TEST 
        manual_blink_loop(); 
#elif TASK_SCHEDULING_TEST 
        task_scheduling_loop(); 
#elif MEMORY_MANAGEMENT_TEST 
        memory_management_loop(); 
#elif QUEUE_TEST 
        queue_loop(); 
#elif MUTEX_TEST 
        mutex_loop(); 
#elif SEMAPHORE_TEST 
        semaphore_loop(); 
#elif SOFTWARE_TIMER_TEST_0 
        software_timer_0_loop(); 
#elif SOFTWARE_TIMER_TEST_1 
        software_timer_1_loop(); 
#elif HARDWARE_INTERRUPT_TEST 
        hardware_interrupt_loop(); 
#elif DEADLOCK_STARVATION_TEST 
        deadlock_starvation_loop(); 
#elif PRIORITY_INVERSION_TEST 
        priority_inversion_loop(); 
#endif
    }

    vTaskDelete(NULL); 
}

//=======================================================================================


#if PERIODIC_BLINK_TEST 

// There are two tasks that both toggle the board LED but at different rates. While not 
// toggling the LED state, each task is put into the blocking state. The main loop task 
// does nothing here. 

//=======================================================================================
// Macros 

// Memory 
#define PB_STACK_SIZE configMINIMAL_STACK_SIZE * 4 

// Timing (number of ticks - default tick period is 1ms) 
#define PB_DELAY_1 500 
#define PB_DELAY_2 600 

//=======================================================================================


//=======================================================================================
// Variables 

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

//=======================================================================================


//=======================================================================================
// Prototypes 

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

//=======================================================================================


//=======================================================================================
// Setup 

void periodic_blink_init(void)
{
    // Turn the board LED off 
    gpio_write(GPIOA, GPIOX_PIN_5, led_state); 

    // Create the thread(s) 
    blink01Handle = osThreadNew(TaskBlink01, NULL, &blink01_attributes); 
    blink02Handle = osThreadNew(TaskBlink02, NULL, &blink02_attributes); 
}

//=======================================================================================


//=======================================================================================
// Loop 

void periodic_blink_loop(void)
{
    // Do nothing 
}

//=======================================================================================


//=======================================================================================
// Tasks and functions 

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

//=======================================================================================


#elif MANUAL_BLINK_TEST 

// There are two tasks used to control the blinking rate of an LED. The main loop task 
// will listen for user input on the serial terminal which indicates the blinking rate 
// of the LED (in ticks). If there is input then it will be read and converted to a 
// number and used to update the blink rate. The other task will toggle the LED state 
// at the rate set by the user input. 

//=======================================================================================
// Macros 

// Memory 
#define MB_STACK_SIZE configMINIMAL_STACK_SIZE * 4 

// Data 
#define MB_MIN_LED_RATE 50 

//=======================================================================================


//=======================================================================================
// Variables 

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

//=======================================================================================


//=======================================================================================
// Prototypes 

/**
 * @brief Task function: MB01 
 * 
 * @param argument : NULL 
 */
void TaskMB01(void *argument); 

//=======================================================================================


//=======================================================================================
// Setup 

void manual_blink_init(void)
{
    // Create the thread(s) 
    MB01Handle = osThreadNew(TaskMB01, NULL, &MB01_attributes); 

    uart_sendstring(USART2, "\r\n>>> "); 
}

//=======================================================================================


//=======================================================================================
// Loop 

void manual_blink_loop(void)
{
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
}

//=======================================================================================


//=======================================================================================
// Tasks and functions 

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

//=======================================================================================


#elif TASK_SCHEDULING_TEST 

// There are two tasks that both print to the serial terminal. The first task is of a 
// lower priority and prints an arbitrary string at every 1 second. The second task is 
// of a higher priority and prints an asterisks every 100 ms. The main loop task controls 
// both of these tasks by first periodically suspending task 2, then after this is done 
// a few times task 1 is deleted so that only task 2 remains running. The messages should 
// be printed at a slow baud rate to better observe the preemptive nature of the RTOS. 

//=======================================================================================
// Macros 

// Memory 
#define TS_STACK_SIZE configMINIMAL_STACK_SIZE * 4 

// Timing 
#define TS_DELAY_1 2000   // Ticks 
#define TS_DELAY_2 1000   // Ticks 
#define TS_DELAY_3 100    // Ticks 
#define TS_DELAY_4 5000   // (ms) 

//=======================================================================================


//=======================================================================================
// Variables 

// Task definition: msg01 
osThreadId_t msg01Handle = NULL; 
const osThreadAttr_t msg01_attributes = 
{
    .name = "msg01", 
    .stack_size = TS_STACK_SIZE, 
    .priority = (osPriority_t) osPriorityBelowNormal 
};

// Task definition: msg02 
osThreadId_t msg02Handle = NULL; 
const osThreadAttr_t msg02_attributes = 
{
    .name = "msg02",
    .stack_size = TS_STACK_SIZE,
    .priority = (osPriority_t) osPriorityNormal 
};

// Strings 
const char msg[] = "Barkadeer brig Arr booty rum"; 

//=======================================================================================


//=======================================================================================
// Prototypes 

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

//=======================================================================================


//=======================================================================================
// Setup 

void task_scheduling_init(void)
{
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
    tim_delay_ms(TIM9, TS_DELAY_4); 
}

//=======================================================================================


//=======================================================================================
// Loop 

void task_scheduling_loop(void)
{
    // Suspend the higher priority task for some intervals 
    for (uint8_t i = CLEAR; i < 3; i++)
    {
        osThreadSuspend(msg02Handle); 
        osDelay(TS_DELAY_1); 
        osThreadResume(msg02Handle); 
        osDelay(TS_DELAY_1); 
    }
    
    // Delete the lower priority task 
    if (msg01Handle != NULL)
    {
        osThreadTerminate(msg01Handle); 
        msg01Handle = NULL; 
    }
}

//=======================================================================================


//=======================================================================================
// Tasks and functions 

// Task function: msg01 
void TaskMsg01(void *argument)
{
    // Print the string to the terminal one character at a time. 
    while (1)
    {
        uart_send_new_line(USART2); 
        uart_sendstring(USART2, msg); 
        uart_send_new_line(USART2); 
        osDelay(TS_DELAY_2); 
    }

    osThreadTerminate(NULL); 
}


// Task function: msg02 
void TaskMsg02(void *argument)
{
    while (1)
    {
        uart_sendchar(USART2, AST_CHAR); 
        osDelay(TS_DELAY_3); 
    }

    osThreadTerminate(NULL); 
}

//=======================================================================================


#elif MEMORY_MANAGEMENT_TEST 

// This test uses two tasks to mimic a serial echo program. One task listens and records 
// input from the serial terminal. Once the end of the input is seen (new line or 
// carriage return), all the input up to that point gets stored in allocated heap memory 
// and informs the second task that there is new data. The second task prints the input 
// message back to there terminal when it becomes available and frees the memory used to 
// store the message. 

//=======================================================================================
// Macros 

// Memory 
#define MM_STACK_SIZE configMINIMAL_STACK_SIZE * 4 

// Data 
#define MM_STR_MAX_LEN 100 

//=======================================================================================


//=======================================================================================
// Variables 

// Task definition: MM01 
osThreadId_t MM01Handle; 
const osThreadAttr_t MM01_attributes = 
{
    .name = "MM01", 
    .stack_size = MM_STACK_SIZE, 
    .priority = (osPriority_t) osPriorityNormal 
};

// Pointer used to locate heap allocated memory 
static char *user_msg = NULL; 

//=======================================================================================


//=======================================================================================
// Prototypes 

/**
 * @brief Task function: MM01 
 * 
 * @param argument : NULL 
 */
void TaskMM01(void *argument); 

//=======================================================================================


//=======================================================================================
// Setup 

void memory_management_init(void)
{
    // Create the thread(s) 
    MM01Handle = osThreadNew(TaskMM01, NULL, &MM01_attributes); 
    osThreadSuspend(MM01Handle);   // Suspend to prevent running right away 

    uart_sendstring(USART2, "\r\n>>> "); 
}

//=======================================================================================


//=======================================================================================
// Loop 

void memory_management_loop(void)
{
    // This interrupt flag will be set when an idle line is detected on UART RX after 
    // receiving new data. 
    if (handler_flags.usart2_flag)
    {
        handler_flags.usart2_flag = CLEAR; 

        uint8_t user_in_buff_local[SERIAL_INPUT_MAX_LEN]; 
        uint32_t input_len; 
        uint8_t mem_info[MM_STR_MAX_LEN]; 

        // Get the user input from the circular buffer 
        cb_parse(uart_dma_buff, user_in_buff_local, &buff_index, SERIAL_INPUT_MAX_LEN); 

        snprintf(
            (char *)mem_info, 
            MM_STR_MAX_LEN, 
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
            MM_STR_MAX_LEN, 
            "Free heap after malloc (bytes): %lu\r\n", 
            (uint32_t)xPortGetFreeHeapSize()); 
        uart_sendstring(USART2, (char *)mem_info); 

        osThreadResume(MM01Handle); 
    }
}

//=======================================================================================


//=======================================================================================
// Tasks and functions 

// Task function: MM01 
void TaskMM01(void *argument)
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
        osThreadSuspend(MM01Handle); 
    }

    osThreadTerminate(NULL); 
}

//=======================================================================================


#elif QUEUE_TEST 

// This test has two tasks and two queues. Task A will read serial terminal input from 
// the user and echo it back to the terminal, print new messages from queue 2 and if 
// "delay <value>" is input by the user then send <value> to queue 1. Task B will 
// update a variable with any new values from queue 1, use the value of this variable 
// to control the blink rate/delay of the board LED, and send the number of times the 
// LED has blinked 100 times to queue 2. 

//=======================================================================================
// Macros 

// Memory 
#define QUEUE_STACK_SIZE configMINIMAL_STACK_SIZE * 4 

// Data 
#define QUEUE_MIN_LED_RATE 50 

//=======================================================================================


//=======================================================================================
// Variables 

// Task definition: queue01 
osThreadId_t queue01Handle; 
const osThreadAttr_t queue01_attributes = 
{
    .name = "queue01", 
    .stack_size = QUEUE_STACK_SIZE, 
    .priority = (osPriority_t) osPriorityNormal 
};

// Queue info 
static const uint8_t msg_queue_len = 5; 
static QueueHandle_t msg_queue_0; 
static QueueHandle_t msg_queue_1; 

//=======================================================================================


//=======================================================================================
// Prototypes 

/**
 * @brief Task function: queue01 
 * 
 * @param argument : NULL 
 */
void TaskQueue01(void *argument); 

//=======================================================================================


//=======================================================================================
// Setup 

void queue_init(void)
{
    // Create the thread(s) 
    queue01Handle = osThreadNew(TaskQueue01, NULL, &queue01_attributes); 

    // Create queue handle(s) 
    msg_queue_0 = xQueueCreate(msg_queue_len, sizeof(uint32_t)); 
    msg_queue_1 = xQueueCreate(msg_queue_len, sizeof(uint32_t)); 

    uart_sendstring(USART2, "\r\n>>> "); 
}

//=======================================================================================


//=======================================================================================
// Loop 

void queue_loop(void)
{
    static uint32_t blink_count = CLEAR; 

    // This interrupt flag will be set when an idle line is detected on UART RX after 
    // receiving new data. 
    if (handler_flags.usart2_flag)
    {
        handler_flags.usart2_flag = CLEAR; 

        // Get the user input from the circular buffer 
        cb_parse(uart_dma_buff, user_in_buff, &buff_index, SERIAL_INPUT_MAX_LEN); 
        
        // Check for a valid delay command. If found then updated the LED delay rate. 
        if (str_compare("delay ", (char *)user_in_buff, BYTE_0))
        {
            char *delay_value_cmd = (char *)(user_in_buff + BYTE_6); 
            uint32_t delay_value = (uint32_t)strtol(delay_value_cmd, NULL, 10); 

            if (delay_value != 0)
            {
                if (delay_value < QUEUE_MIN_LED_RATE)
                {
                    delay_value = QUEUE_MIN_LED_RATE; 
                }

                // Send delay value to queue 0 
                if (xQueueSend(msg_queue_0, (void *)&delay_value, 0) == pdTRUE)
                {
                    char delay_value_str[SERIAL_INPUT_MAX_LEN]; 
                    snprintf(
                        delay_value_str, 
                        SERIAL_INPUT_MAX_LEN, 
                        "Blink delay (ms): %lu", 
                        delay_value); 
                    uart_sendstring(USART2, delay_value_str); 
                    uart_send_new_line(USART2); 
                }
                else 
                {
                    uart_sendstring(USART2, "Queue full.\n"); 
                }

            }
        }

        uart_sendstring(USART2, "\r\n>>> "); 
    }

    // Print new messages from queue 1 
    if (xQueueReceive(msg_queue_1, (void *)&blink_count, 0) == pdTRUE)
    {
        char blink_count_str[SERIAL_INPUT_MAX_LEN]; 
        snprintf(
            blink_count_str, 
            SERIAL_INPUT_MAX_LEN, 
            "\rBlink count (x100): %lu\n", 
            blink_count); 
        uart_sendstring(USART2, blink_count_str); 
        uart_sendstring(USART2, "\r\n>>> "); 
    }
}

//=======================================================================================


//=======================================================================================
// Tasks and functions 

// Task function: queue01 
void TaskQueue01(void *argument)
{
    uint32_t delay = QUEUE_MIN_LED_RATE, blink_count = CLEAR, blink_tally = CLEAR; 
    gpio_pin_state_t led_state = GPIO_LOW; 

    while (1)
    {
        led_state = GPIO_HIGH - led_state; 
        gpio_write(GPIOA, GPIOX_PIN_5, led_state); 
        blink_count += (uint32_t)led_state; 

        if (blink_count >= 100)
        {
            blink_count = CLEAR; 
            blink_tally++; 
            xQueueSend(msg_queue_1, (void *)&blink_tally, 0); 
        }

        xQueueReceive(msg_queue_0, (void *)&delay, 0); 
        osDelay(delay); 
    }

    osThreadTerminate(NULL); 
}

//=======================================================================================


#elif MUTEX_TEST 

// There are two tasks that both increment the same counter using the same function call. 
// There is a mutex that prevents the other task from running while the current task 
// increments the counter. 

//=======================================================================================
// Macros 

// Memory 
#define MUTEX_STACK_SIZE configMINIMAL_STACK_SIZE * 4 

// Timing 
#define MUTEX_DELAY_1 1000   // Ticks 
#define MUTEX_DELAY_2 1500   // Ticks 

//=======================================================================================


//=======================================================================================
// Variables 

// Task definition: increment01 
osThreadId_t increment01Handle; 
const osThreadAttr_t increment01_attributes = 
{
    .name = "increment01", 
    .stack_size = MUTEX_STACK_SIZE, 
    .priority = (osPriority_t) osPriorityNormal 
};

// Task definition: increment02 
osThreadId_t increment02Handle; 
const osThreadAttr_t increment02_attributes = 
{
    .name = "increment02",
    .stack_size = MUTEX_STACK_SIZE,
    .priority = (osPriority_t) osPriorityBelowNormal 
};

// Shared task counter 
static uint16_t counter_shared = CLEAR; 

// Mutex 
static SemaphoreHandle_t mutex; 

//=======================================================================================


//=======================================================================================
// Prototypes 

/**
 * @brief Task function: increment01 
 * 
 * @param argument : NULL 
 */
void TaskInc01(void *argument); 


/**
 * @brief Task function: increment02 
 * 
 * @param argument : NULL 
 */
void TaskInc02(void *argument); 


/**
 * @brief Increment counter 
 * 
 * @param delay : task delay time (ticks) 
 */
void increment_counter(uint16_t delay); 

//=======================================================================================


//=======================================================================================
// Setup 

void mutex_init(void)
{
    // Create the thread(s) 
    increment01Handle = osThreadNew(TaskInc01, NULL, &increment01_attributes); 
    increment02Handle = osThreadNew(TaskInc02, NULL, &increment02_attributes); 

    // Create the mutex 
    mutex = xSemaphoreCreateMutex(); 
}

//=======================================================================================


//=======================================================================================
// Loop 

void mutex_loop(void)
{
    // Do nothing 
}

//=======================================================================================


//=======================================================================================
// Tasks and functions 

// Task function: increment01 
void TaskInc01(void *argument)
{
    while (1)
    {
        increment_counter(MUTEX_DELAY_1); 
    }

    osThreadTerminate(NULL); 
}


// Task function: increment02 
void TaskInc02(void *argument)
{
    while (1)
    {
        increment_counter(MUTEX_DELAY_2); 
    }

    osThreadTerminate(NULL); 
}


// Increment counter 
void increment_counter(uint16_t delay)
{
    static uint16_t counter_temp = CLEAR; 

    // Take mutex prior to critical section 
    if (xSemaphoreTake(mutex, 0) == pdTRUE)
    {
        // Purposely increment in a non-efficient manner 
        counter_temp = counter_shared; 
        counter_temp++; 
        osDelay(delay); 
        counter_shared = counter_temp; 

        char num_str[SERIAL_INPUT_MAX_LEN]; 
        snprintf(num_str, SERIAL_INPUT_MAX_LEN, "%u\r\n", counter_shared); 
        uart_sendstring(USART2, num_str); 

        // Give mutex back after critical section 
        xSemaphoreGive(mutex); 
    }
    else 
    {
        // Do something useful if the critical section cannot be accessed. 
    }
}

//=======================================================================================


#elif SEMAPHORE_TEST 

// There are 5 producer tasks that write to a circular buffer and 2 consumer tasks that 
// read from it. The producer tasks write their task number to the buffer 3 times and 
// the consumers print the contents of the buffer to the serial terminal. The serial 
// terminal output should display each task number exactly 3 times and have one number 
// output per line, meaning access to the buffer, the state of the buffer and the serial 
// line must be protected from multiple accesse. Protection is accomplished by a mutex 
// to protect the buffer and serial line as well as two counting semaphores, one for 
// the number of empty buffer slots and one for the number of full buffer slots, that 
// prevent overwriting and overreading the buffer. Each producer task should be created, 
// copy its task number passed as the task argument to a local variable, run once and 
// then deleted itself. 

//=======================================================================================
// Macros 

// Memory 
#define SEMAPHORE_STACK_SIZE configMINIMAL_STACK_SIZE * 4 

// Data 
#define SEMAPHORE_NUM_PRODUCERS 5 
#define SEMAPHORE_NUM_CONSUMERS 2 
#define SEMAPHORE_NUM_WRITES 3 
#define SEMAPHORE_BUFF_SIZE 5 
#define SEMAPHORE_TASK_NAME_LEN 15 

// Timing 
#define SEMPAHORE_DELAY_1 5000   // (ms) 

//=======================================================================================


//=======================================================================================
// Variables 

// Task definition: increment01 
osThreadId_t semaphoreSetupHandle; 
const osThreadAttr_t semaphoreSetup_attributes = 
{
    .name = "semaphoreSetup", 
    .stack_size = SEMAPHORE_STACK_SIZE, 
    .priority = (osPriority_t) osPriorityNormal 
};

// Semaphores 
static SemaphoreHandle_t binary_sem;   // Allows for parameter copying 
static SemaphoreHandle_t filled_sem;   // Number of filled slots in the circular buffer 
static SemaphoreHandle_t empty_sem;    // Number of empty slots in the circular buffer 

// Mutex 
static SemaphoreHandle_t buff_mutex;   // Protects the circular buffer and serial output 

// Data 
static uint8_t buff[SEMAPHORE_BUFF_SIZE];  // Circular buffer 
static uint8_t write_index = CLEAR;        // Write/head index for circular buffer 
static uint8_t read_index = CLEAR;         // Read/tail index for circular buffer 

//=======================================================================================


//=======================================================================================
// Prototypes 

/**
 * @brief Task function: semaphoreSetup 
 * 
 * @param argument : NULL 
 */
void TaskSemaphoreSetup(void *argument); 


/**
 * @brief Task function: semaphoreProducer 
 * 
 * @param argument : NULL 
 */
void TaskSemaphoreProducer(void *argument); 


/**
 * @brief Task function: semaphoreConsumer 
 * 
 * @param argument : NULL 
 */
void TaskSemaphoreConsumer(void *argument); 

//=======================================================================================


//=======================================================================================
// Setup 

void semaphore_init(void)
{
    // Create the thread(s) 
    semaphoreSetupHandle = osThreadNew(TaskSemaphoreSetup, NULL, &semaphoreSetup_attributes); 

    // Create semaphore(s) 
    binary_sem = xSemaphoreCreateBinary(); 
    filled_sem = xSemaphoreCreateCounting(SEMAPHORE_BUFF_SIZE, 0); 
    empty_sem = xSemaphoreCreateCounting(SEMAPHORE_BUFF_SIZE, SEMAPHORE_BUFF_SIZE); 

    // Create mutex 
    buff_mutex = xSemaphoreCreateMutex(); 

    // Blocking delay to provide time for the user to connect to the serial terminal 
    tim_delay_ms(TIM9, SEMPAHORE_DELAY_1); 
}

//=======================================================================================


//=======================================================================================
// Loop 

void semaphore_loop(void)
{
    // Do nothing 
}

//=======================================================================================


//=======================================================================================
// Tasks and functions 

// Task function: semaphoreSetup 
void TaskSemaphoreSetup(void *argument)
{
    char task_name[SEMAPHORE_TASK_NAME_LEN]; 

    // Create and start producer threads 
    for (uint8_t i = CLEAR; i < SEMAPHORE_NUM_PRODUCERS; i++)
    {
        snprintf(task_name, SEMAPHORE_TASK_NAME_LEN, "producer-%u", i); 

        const osThreadAttr_t producer_attributes = 
        {
            .name = task_name, 
            .stack_size = SEMAPHORE_STACK_SIZE, 
            .priority = (osPriority_t) osPriorityNormal 
        };

        // Tasks delete themselves after executing the task function 
        osThreadNew(TaskSemaphoreProducer, (void *)&i, &producer_attributes); 

        // Wait for each task to read the argument 
        xSemaphoreTake(binary_sem, portMAX_DELAY); 
    }

    // Create and start consumer threads 
    for (uint8_t i = CLEAR; i < SEMAPHORE_NUM_CONSUMERS; i++)
    {
        snprintf(task_name, SEMAPHORE_TASK_NAME_LEN, "consumer-%i", i); 

        const osThreadAttr_t consumer_attributes = 
        {
            .name = task_name, 
            .stack_size = SEMAPHORE_STACK_SIZE, 
            .priority = (osPriority_t) osPriorityNormal 
        };

        osThreadNew(TaskSemaphoreConsumer, NULL, &consumer_attributes); 
    }

    while (1) {} 

    // Delete the task 
    osThreadTerminate(NULL); 
}


// Task function: semaphoreProducer 
void TaskSemaphoreProducer(void *argument)
{
    // Copy 'argument' into a local variable 
    uint8_t num = *(uint8_t *)argument; 

    // Release the binary semaphore 
    xSemaphoreGive(binary_sem); 

    // Fill the shared buffer with the task number 3 times 
    for (uint8_t i = CLEAR; i < SEMAPHORE_NUM_WRITES; i++)
    {
        // Critical section 

        xSemaphoreTake(empty_sem, portMAX_DELAY); 
        xSemaphoreTake(buff_mutex, portMAX_DELAY); 

        buff[write_index] = num; 
        write_index = (write_index + 1) % SEMAPHORE_BUFF_SIZE; 

        xSemaphoreGive(buff_mutex); 
        xSemaphoreGive(filled_sem); 
    }

    // Delete the task 
    osThreadTerminate(NULL); 

    // This loop does not execute 
    while (1) {}
}


// Task function: semaphoreConsumer 
void TaskSemaphoreConsumer(void *argument)
{
    uint8_t val; 

    // Read from the buffer 
    while (1)
    {
        // Critical section 

        xSemaphoreTake(filled_sem, portMAX_DELAY); 
        xSemaphoreTake(buff_mutex, portMAX_DELAY); 

        val = buff[read_index]; 
        read_index = (read_index + 1) % SEMAPHORE_BUFF_SIZE; 
        uart_send_integer(USART2, (int16_t)val); 
        uart_send_new_line(USART2); 

        xSemaphoreGive(buff_mutex); 
        xSemaphoreGive(empty_sem); 
    }

    osThreadTerminate(NULL); 
}

//=======================================================================================


#elif SOFTWARE_TIMER_TEST_0

// There are two software timers, one is a one-shot and the other is a repeat. Both 
// timers have the same callback function that will display which timer has expired 
// in the serial terminal. 

//=======================================================================================
// Macros 

// Timing 
#define SOFTWARE_TIMER_0_DELAY_1 1000    // ms 
#define SOFTWARE_TIMER_0_PERIOD_1 1000   // Ticks 
#define SOFTWARE_TIMER_0_PERIOD_2 2000   // Ticks 

//=======================================================================================


//=======================================================================================
// Variables 

// Timer(s) - FreeRTOS API method 
static TimerHandle_t one_shot_timer = NULL; 
static TimerHandle_t auto_reload_timer = NULL; 

// Timer(s) - CMSIS API method 
// static osTimerId_t oneShotTimerHandle; 
// const osTimerAttr_t oneShot_attributes = { .name = "oneShotTimer" }; 

//=======================================================================================


//=======================================================================================
// Prototypes 

/**
 * @brief Called when one of the timers expires 
 * 
 * @param xTimer : software timer ID 
 */
void myTimerCallback(TimerHandle_t xTimer); 

//=======================================================================================


//=======================================================================================
// Setup 

void software_timer_0_init(void)
{
    //==================================================
    // FreeRTOS API method 

    // Create timers 
    one_shot_timer = xTimerCreate(
        "one_shot_timer",               // Name of timer 
        SOFTWARE_TIMER_0_PERIOD_1,      // Period of timer (ticks) 
        pdFALSE,                        // Auto-relead --> pdFASLE == One Shot Timer 
        (void *)0,                      // Timer ID 
        myTimerCallback);               // Callback function 
    
    auto_reload_timer = xTimerCreate(
        "auto_reload_timer",            // Name of timer 
        SOFTWARE_TIMER_0_PERIOD_2,      // Period of timer (ticks) 
        pdTRUE,                         // Auto-relead --> pdTRUE == Repeat Timer 
        (void *)1,                      // Timer ID 
        myTimerCallback);               // Callback function 

    // Check that the timers were created properly 
    if ((one_shot_timer == NULL) || (auto_reload_timer == NULL))
    {
        uart_sendstring(USART2, "Couldn't create at least one of the timers.\r\n"); 
    }
    else 
    {
        tim_delay_ms(TIM9, SOFTWARE_TIMER_0_DELAY_1); 
        uart_sendstring(USART2, "Starting timers...\r\n"); 

        // Start timers (max block time if command queue is full). 
        // Essentially says to wait forever if the queue is full. 
        xTimerStart(one_shot_timer, portMAX_DELAY); 
        xTimerStart(auto_reload_timer, portMAX_DELAY); 
    }
    
    //==================================================

    //==================================================
    // CMSIS API method 

    // Note that using this function doesn't allow you to adjust the period of the timer. 
    // oneShotTimerHandle = osTimerNew(myTimerCallback, osTimerOnce, NULL, &oneShot_attributes); 
    // osTimerStart(oneShotTimerHandle, 1); 
    
    //==================================================
}

//=======================================================================================


//=======================================================================================
// Loop 

void software_timer_0_loop(void)
{
    // Yield the main loop task to the lower priority software timers. 
    osDelay(SOFTWARE_TIMER_0_DELAY_1); 
}

//=======================================================================================


//=======================================================================================
// Tasks and functions 

// Called when one of the timers expires 
void myTimerCallback(TimerHandle_t xTimer)
{
    // The 'xTimer' parameter can be used to identify which timer called this function 
    // when multiple timers call this function. 

    // Print message if timer 0 expired 
    if ((uint32_t)pvTimerGetTimerID(xTimer) == 0)
    {
        uart_sendstring(USART2, "One-shot timer expired.\r\n"); 
    }

    // Print message if timer 1 expired 
    if ((uint32_t)pvTimerGetTimerID(xTimer) == 1)
    {
        uart_sendstring(USART2, "Auto-reload timer expired. Reloading...\r\n"); 
    }
}

//=======================================================================================


#elif SOFTWARE_TIMER_TEST_1 

// Use the board LED to simulate a backlight display. When characters are entered in 
// the serial terminal (doesn't matter what), the LED lights up as if we're interfacing 
// with a screen menu. Turn the LED off after 5 second of no input (inactivity). Use a 
// software timer to accomplish this. Hint: xTimerStart will restart a counter if it's 
// called before the timer expires. 

//=======================================================================================
// Macros 

// Memory 
#define SOFTWARE_TIMER_STACK_SIZE configMINIMAL_STACK_SIZE * 4 

// Timing 
#define SOFTWARE_TIMER_1_DELAY_1 1000   // ms 
#define SOFTWARE_TIMER_1_DELAY_2 5      // ms
#define SOFTWARE_TIMER_1_PERIOD 5000    // ticks 

//=======================================================================================


//=======================================================================================
// Variables 

// Task definition: softwareTimer 
osThreadId_t softwareTimerHandle; 
const osThreadAttr_t softwareTimer_attributes = 
{
    .name = "softwareTimer",
    .stack_size = SOFTWARE_TIMER_STACK_SIZE,
    .priority = (osPriority_t) osPriorityNormal 
};

// Timer(s) 
static TimerHandle_t display_timer = NULL; 

//=======================================================================================


//=======================================================================================
// Prototypes 

/**
 * @brief Task function: softwareTimer 
 * 
 * @param argument : NULL 
 */
void TaskSoftwareTimer(void *argument); 


/**
 * @brief Called when the display sees no activity 
 * 
 * @param argument : NULL 
 */
void DisplayBacklightCallback(TimerHandle_t argument); 

//=======================================================================================


//=======================================================================================
// Setup 

void software_timer_1_init(void)
{
    // Create the thread(s) 
    softwareTimerHandle = osThreadNew(TaskSoftwareTimer, NULL, &softwareTimer_attributes); 

    // Create timer(s) 
    display_timer = xTimerCreate(
        "display_backlight_timer",      // Name of timer 
        SOFTWARE_TIMER_1_PERIOD,        // Period of timer (ticks) 
        pdFALSE,                        // Auto-relead --> pdFASLE == One Shot Timer 
        NULL,                           // Timer ID 
        DisplayBacklightCallback);      // Callback function 
}

//=======================================================================================


//=======================================================================================
// Loop 

void software_timer_1_loop(void)
{
    // This interrupt flag will be set when an idle line is detected on UART RX after 
    // receiving new data. 
    if (handler_flags.usart2_flag)
    {
        handler_flags.usart2_flag = CLEAR; 
        uart_sendstring(USART2, ">>> "); 

        // Restart the display timeout and turn the board LED on 
        xTimerStart(display_timer, portMAX_DELAY); 
        gpio_write(GPIOA, GPIOX_PIN_5, GPIO_HIGH); 
    }

    // Yield the main loop task to the lower priority software timers. 
    osDelay(SOFTWARE_TIMER_1_DELAY_2); 
}

//=======================================================================================


//=======================================================================================
// Tasks and functions 

// Task function: softwareTimer 
void TaskSoftwareTimer(void *argument)
{
    if (display_timer == NULL)
    {
        uart_sendstring(USART2, "Couldn't create timer.\r\n"); 
    }
    else 
    {
        tim_delay_ms(TIM9, SOFTWARE_TIMER_1_DELAY_1); 
        uart_sendstring(USART2, "Starting interface...\r\n>>> "); 

        // Turn the board LED on 
        gpio_write(GPIOA, GPIOX_PIN_5, GPIO_HIGH); 

        // Start timers (max block time if command queue is full). 
        xTimerStart(display_timer, portMAX_DELAY); 
    }

    osThreadTerminate(softwareTimerHandle); 
}


// Called when the display sees no activity 
void DisplayBacklightCallback(TimerHandle_t argument)
{
    // Turn the board LED off 
    gpio_write(GPIOA, GPIOX_PIN_5, GPIO_LOW); 
}

//=======================================================================================


#elif HARDWARE_INTERRUPT_TEST 

// Use a hardware timer (ISR) to sample an ADC 10 times per second (10 Hz) and store the 
// values in a double or circular buffer. A double or circular buffer is recommended so 
// data can be read/used at the same time new data gets written. Once 10 samples have 
// been collected, the ISR should wake up task A which computes the average of the 10 
// samples and stores the result in a global floating point variable. Assume this global 
// variable cannot be written to or read from in a single instruction cycle (critical 
// section). Task B will output the value of the global variable to the serial terminal 
// if "avg" is entered by the user, otherwise the input is ignored. 

//=======================================================================================
// Macros 

// Memory 
#define HARDWARE_INT_STACK_SIZE configMINIMAL_STACK_SIZE * 4 

// Data 
#define HARDWARE_INT_NUM_SAMPLES 10 

//=======================================================================================


//=======================================================================================
// Variables 

// Task definition: hardwareInterrupt01 
osThreadId_t hardwareInterrupt01Handle; 
const osThreadAttr_t hardwareInterrupt01_attributes = 
{
    .name = "hardwareInterrupt01", 
    .stack_size = HARDWARE_INT_STACK_SIZE, 
    .priority = (osPriority_t) osPriorityNormal 
};

// Data 
static double avg = CLEAR; 
static volatile uint16_t adc_result[2][HARDWARE_INT_NUM_SAMPLES]; 
static uint8_t write_index = CLEAR; 
static uint8_t read_index = SET_BIT; 

// Semaphore(s) and Mutex 
static SemaphoreHandle_t binary_sem_0; 
static SemaphoreHandle_t binary_sem_1; 
static SemaphoreHandle_t mutex; 

//=======================================================================================


//=======================================================================================
// Prototypes 

/**
 * @brief Task function: hardwareInterrupt 
 * 
 * @param argument : NULL 
 */
void TaskHardwareInterrupt(void *argument); 

//=======================================================================================


//=======================================================================================
// Setup 

void hardware_interrupt_init(void)
{
    // Re-define timer 9 to be a periodic (counter update) interrupt timer 
    tim_disable(TIM9); 
    tim_9_to_11_counter_init(
        TIM9, 
        TIM_84MHZ_100US_PSC, 
        0x03E8,  // ARR=1000, (1000 counts)*(100us/count) = 100ms 
        TIM_UP_INT_ENABLE); 
    tim_enable(TIM9); 

    // Enable the interrupt handlers. This interrupt calls interrupt safe FreeRTOS API 
    // functions so it has to have a lower (numerically higher) priority than 
    // 'configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY' in 'FreeRTOSConfig.h'. 
    nvic_config(TIM1_BRK_TIM9_IRQn, EXTI_PRIORITY_7); 

    // Initialize the ADC port, pin and read sequence 
    adc1_clock_enable(RCC); 
    adc_port_init(
        ADC1, 
        ADC1_COMMON, 
        ADC_PCLK2_4, 
        ADC_RES_8, 
        ADC_PARAM_ENABLE, 
        ADC_PARAM_ENABLE, 
        ADC_PARAM_DISABLE, 
        ADC_PARAM_DISABLE, 
        ADC_PARAM_DISABLE, 
        ADC_PARAM_DISABLE, 
        ADC_PARAM_DISABLE); 
    adc_pin_init(ADC1, GPIOC, PIN_0, ADC_CHANNEL_10, ADC_SMP_15); 
    adc_seq(ADC1, ADC_CHANNEL_10, ADC_SEQ_1); 
    adc_on(ADC1); 

    // Create the thread(s) 
    hardwareInterrupt01Handle = osThreadNew(
        TaskHardwareInterrupt, NULL, &hardwareInterrupt01_attributes); 

    // Create semaphore(s) and mutex 
    binary_sem_0 = xSemaphoreCreateBinary();   // Indicates when samples can be averaged 
    binary_sem_1 = xSemaphoreCreateBinary();   // Controls double buffer read/write index 
    mutex = xSemaphoreCreateMutex();           // Protects 'avg' 

    // We give the buffer index semaphore immediately to avoid the initial conflict 
    // between updating the double buffer index and averaging the first sample of 10. 
    xSemaphoreGive(binary_sem_1); 

    uart_sendstring(USART2, ">>> "); 
}

//=======================================================================================


//=======================================================================================
// Loop 

void hardware_interrupt_loop(void)
{
    // This interrupt flag will be set when an idle line is detected on UART RX after 
    // receiving new data. 
    if (handler_flags.usart2_flag)
    {
        handler_flags.usart2_flag = CLEAR; 

        // Get the user input from the circular buffer 
        cb_parse(uart_dma_buff, user_in_buff, &buff_index, SERIAL_INPUT_MAX_LEN); 

        // Check for the average command by checking that the input is exactly "avg". 
        if (strlen((char *)user_in_buff) == 3)
        {
            char avg_str[SERIAL_INPUT_MAX_LEN]; 

            if (str_compare("avg", (char *)user_in_buff, BYTE_0))
            {
                // Display the average and protect 'avg' so it cannot be updated during 
                // the serial output process. 
                xSemaphoreTake(mutex, portMAX_DELAY); 
                snprintf(
                    avg_str, 
                    SERIAL_INPUT_MAX_LEN, 
                    "Average: %u.%u\r\n", 
                    (uint16_t)avg, 
                    (uint16_t)(avg * SCALE_10) % DIVIDE_10); 
                xSemaphoreGive(mutex); 
                uart_sendstring(USART2, avg_str); 
            }
        }

        uart_sendstring(USART2, ">>> "); 
    }
}

//=======================================================================================


//=======================================================================================
// Tasks and functions 

// Task function: hardwareInterrupt 
void TaskHardwareInterrupt(void *argument)
{
    while (1)
    {
        // Waits for indication from the ISR that 10 samples have been read before 
        // proceeding to average the samples. 
        xSemaphoreTake(binary_sem_0, portMAX_DELAY); 

        // Compute the average of 10 samples in the part of the double buffer not 
        // being written to by the ISR. 
        uint32_t sum = CLEAR; 
        for (uint8_t i = CLEAR; i < HARDWARE_INT_NUM_SAMPLES; i++)
        {
            sum += (uint32_t)adc_result[read_index][i]; 
        }

        // Allow for read and write section indexes of the double buffer to be updated 
        // in the ISR now that averaging is done. 
        xSemaphoreGive(binary_sem_1); 

        // Protect 'avg' so it can't be used for serial terminal output at the same 
        // time. 
        xSemaphoreTake(mutex, portMAX_DELAY); 
        avg = (double)sum / DIVIDE_10; 
        xSemaphoreGive(mutex); 
        // Could also use vPortEnterCritical and vPortExitCritical here to diable 
        // interrupts for the critical section. 
    }

    osThreadTerminate(NULL); 
}


// Timer 1 break + timer 9 global interrupt - overridden 
void TIM1_BRK_TIM9_IRQHandler(void)
{
    static uint8_t adc_index = CLEAR; 
    BaseType_t task_woken = pdFALSE; 

    // ADC read 
    adc_result[write_index][adc_index++] = adc_read_single(ADC1, ADC_CHANNEL_10); 

    if (adc_index >= HARDWARE_INT_NUM_SAMPLES)
    {
        adc_index = CLEAR; 

        // If averaging is still happening then don't swap the read and write sections 
        // and restart the 10 samples. This can be handled in multiple different ways 
        // such as blocking only one sample at a time. 
        if (xSemaphoreTakeFromISR(binary_sem_1, &task_woken) == pdTRUE)
        {
            read_index = write_index; 
            write_index = SET_BIT - write_index; 
        }
        // Can output a message if data is dropped to help see what's going on. 

        // Give the semaphore to wake up the averaging task (tell it that data is ready). 
        // This sepcial ISR API function will never block since this is not a task. There 
        // are also FreeRTOS ISR functions for directly notifying tasks which could have 
        // been used instead. 
        xSemaphoreGiveFromISR(binary_sem_0, &task_woken); 
    }

    // The following is needed to exit the ISR 
    tim_uif_clear(TIM1); 
    tim_uif_clear(TIM9); 

    // Exit from ISR - if 'task_woken' == 'pdTRUE' then 'portYIELD_FROM_ISR' requests a 
    // context switch to allow a higher priority task to immediately run if it's waiting 
    // on data from this ISR. 
    portYIELD_FROM_ISR(task_woken); 
}

//=======================================================================================


#elif DEADLOCK_STARVATION_TEST 

// "Dining Philosophers Problem" - based on video series 
// 5 philosophers at a table, 5 chopsticks available, one bowl of noodles. A chopstick 
// is placed in between each philosopher and a philosopher can only eat when they have 
// two chopsticks. The challenge is to find a logical way to make sure all philosophers 
// get to eat. 
// For this example, the philosophers are the tasks, the chopsticks are the semaphores 
// and mutexes, and the bowl of food is the shared resource. There are 5 tasks that all 
// call the same function which takes a left chopstick, then a right chopstick and "eats" 
// for a while (i.e. prints to the serial terminal). After it's done then the chopsticks 
// are put down and the function exits. If all philosophers get a chance to eat then a 
// "done" message will be printed to the serial terminal. To accomplish this, a hierarchy 
// solution or an arbitrator solution can be used: 
// - Hierarchy: A hierarchy (number) is assigned to the chopsticks and philosophers can 
//              pick up a chopstick next to them with the lowest number. This leaves one 
//              philosopher free to pick up two chopsticks and eat. Eventually the eating 
//              philosopher will put down the chopsticks and it will trigger a chain of 
//              eating. 
// - Arbitrator: A mutex that protects the chopsticks so only one philosopher can eat at 
//               a time. This makes the system no better than executing everything in a 
//               super loop but it's used for the sake of the problem. 

//=======================================================================================
// Macros 

// Memory 
#define DEADLOCK_STARVATION_STACK_SIZE configMINIMAL_STACK_SIZE * 4 

// Data 
#define NUM_PHILOSOPHERS 5 
#define DEADLOCK_STARVATION_TASK_NAME_LEN 15 
#define PHILOSOPHER_STR_MAX_LEN 50 

// Timing 
#define DEADLOCK_STARVATION_DELAY_1 5000   // (ms) 

//=======================================================================================


//=======================================================================================
// Variables 

// Task definition: increment01 
osThreadId_t deadlockStarvationSetupHandle; 
const osThreadAttr_t deadlockStarvationSetup_attributes = 
{
    .name = "deadlockStarvationSetup", 
    .stack_size = DEADLOCK_STARVATION_STACK_SIZE, 
    .priority = (osPriority_t) osPriorityNormal 
};

// Semaphores 
static SemaphoreHandle_t binary_sem;                    // Wait for params to read 
static SemaphoreHandle_t finish_sem;                    // Notifies main task when done 

// Mutex 
static SemaphoreHandle_t chopstick[NUM_PHILOSOPHERS];   // Chipstick access 
static SemaphoreHandle_t serial_mutex;                  // Serial terminal protection 
static SemaphoreHandle_t arbitrator_mutex;              // Arbitrator chopstick access 

//=======================================================================================


//=======================================================================================
// Prototypes 

/**
 * @brief Task function: deadlockStarvationSetup 
 * 
 * @param argument : NULL 
 */
void TaskDeadlockStarvationSetup(void *argument); 


/**
 * @brief Task function: philosopherEat 
 * 
 * @param argument : NULL 
 */
void TaskPhilosopherEat(void *argument); 


/**
 * @brief Output philosopher strings to the serial terminal 
 * 
 * @param buff : string to output 
 */
void philosopher_output(char *buff); 

//=======================================================================================


//=======================================================================================
// Setup 

void deadlock_starvation_init(void)
{
    // Create the thread(s) 
    deadlockStarvationSetupHandle = osThreadNew(
        TaskDeadlockStarvationSetup, NULL, &deadlockStarvationSetup_attributes); 

    // Create semaphore(s) 
    binary_sem = xSemaphoreCreateBinary(); 
    finish_sem = xSemaphoreCreateCounting(NUM_PHILOSOPHERS, 0); 

    // Create mutex 
    serial_mutex = xSemaphoreCreateMutex(); 
    arbitrator_mutex = xSemaphoreCreateMutex(); 
    
    for (uint8_t i = CLEAR; i < NUM_PHILOSOPHERS; i++)
    {
        chopstick[i] = xSemaphoreCreateMutex(); 
    }

    // Blocking delay to provide time for the user to connect to the serial terminal 
    tim_delay_ms(TIM9, DEADLOCK_STARVATION_DELAY_1); 
}

//=======================================================================================


//=======================================================================================
// Loop 

void deadlock_starvation_loop(void)
{
    // Do nothing 
}

//=======================================================================================


//=======================================================================================
// Tasks and functions 

// Task function: deadlockStarvationSetup 
void TaskDeadlockStarvationSetup(void *argument)
{
    char task_name[DEADLOCK_STARVATION_TASK_NAME_LEN]; 

    // Have philosophers start eating 
    for (uint8_t i = CLEAR; i < NUM_PHILOSOPHERS; i++)
    {
        snprintf(task_name, DEADLOCK_STARVATION_TASK_NAME_LEN, "philosopher-%u", i); 

        const osThreadAttr_t philosopher_attributes = 
        {
            .name = task_name, 
            .stack_size = DEADLOCK_STARVATION_STACK_SIZE, 
            .priority = (osPriority_t) osPriorityNormal 
        };

        // Tasks delete themselves after executing the task function 
        osThreadNew(TaskPhilosopherEat, (void *)&i, &philosopher_attributes); 

        // Wait for each task to read the argument 
        xSemaphoreTake(binary_sem, portMAX_DELAY); 
    }

    // Wait until all philosophers are done 
    for (uint8_t i = CLEAR; i < NUM_PHILOSOPHERS; i++)
    {
        xSemaphoreTake(finish_sem, portMAX_DELAY); 
    }

    // Indicate that all philosophers ate without deadlock 
    uart_sendstring(USART2, "Done! No deadlock.\r\n"); 

    // Delete the task 
    vTaskDelete(NULL); 
}


// Task function: philosopherEat 
void TaskPhilosopherEat(void *argument)
{
    uint8_t philosopher_num = *(uint8_t *)argument; 
    char buff[PHILOSOPHER_STR_MAX_LEN]; 

    // Increment the sempahore count after the parameter has been copied 
    xSemaphoreGive(binary_sem); 

    //==================================================
    // Arbitrator Method 

    // xSemaphoreTake(arbitrator_mutex, portMAX_DELAY); 
    
    //==================================================

    //==================================================
    // Hierarchy Method (can run with arbitrator method active) 

    uint8_t left_chop = philosopher_num; 
    uint8_t right_chop = (philosopher_num + 1) % NUM_PHILOSOPHERS; 
    uint8_t smallest, largest; 
    
    if (left_chop < right_chop)
    {
        smallest = left_chop; 
        largest = right_chop; 
    }
    else 
    {
        smallest = right_chop; 
        largest = left_chop; 
    }

    // Take the smaller value chopstick 
    xSemaphoreTake(chopstick[smallest], portMAX_DELAY); 
    snprintf(
        buff, 
        PHILOSOPHER_STR_MAX_LEN, 
        "Philosopher %u took chopstick %u\r\n", 
        philosopher_num, 
        philosopher_num); 
    philosopher_output(buff); 

    // Add some delay to force deadlock - this part of the challenge so don't delete it 
    osDelay(1); 

    // Take the largest value chopstick 
    xSemaphoreTake(chopstick[largest], portMAX_DELAY); 
    snprintf(
        buff, 
        PHILOSOPHER_STR_MAX_LEN, 
        "Philosopher %u took chopstick %u\r\n", 
        philosopher_num, 
        (philosopher_num + 1) % NUM_PHILOSOPHERS); 
    philosopher_output(buff); 

    // Eat the noodles 
    snprintf(
        buff, 
        PHILOSOPHER_STR_MAX_LEN, 
        "Philosopher %u is eating\r\n", 
        philosopher_num); 
    philosopher_output(buff); 
    osDelay(10); 

    // Return the largest value chopstick 
    xSemaphoreGive(chopstick[largest]); 
    snprintf(
        buff, 
        PHILOSOPHER_STR_MAX_LEN, 
        "Philosopher %u returned chopstick %u\r\n", 
        philosopher_num, 
        (philosopher_num + 1) % NUM_PHILOSOPHERS); 
    philosopher_output(buff); 

    // Return the smaller value chopstick 
    xSemaphoreGive(chopstick[smallest]); 
    snprintf(
        buff, 
        PHILOSOPHER_STR_MAX_LEN, 
        "Philosopher %u returned chopstick %u\r\n", 
        philosopher_num, 
        philosopher_num); 
    philosopher_output(buff); 
    
    //==================================================

    //==================================================
    // Arbitrator Method 

    // xSemaphoreGive(arbitrator_mutex); 

    //==================================================
    
    // Notify setup task and delete self 
    xSemaphoreGive(finish_sem); 
    vTaskDelete(NULL); 
}


// Output philosopher strings to the serial terminal 
void philosopher_output(char *buff)
{
    xSemaphoreTake(serial_mutex, portMAX_DELAY); 
    uart_sendstring(USART2, buff); 
    xSemaphoreGive(serial_mutex); 
}

//=======================================================================================


#elif PRIORITY_INVERSION_TEST 

// There are 3 tasks each with different priority levels (low, medium, high). The low and 
// high priority tasks share a critical section. The lower priority task is forced to run 
// first so it takes hold of the critical section before the high priority task can. The 
// critical section must be properly protected to ensure the medium priority task can't 
// preempt the low priority task while the low priority task has hold of the critical 
// section and therefore further delaying the high priority task (bounded vs unbounded 
// priority inversion). Mutexes in FreeRTOS handle the priority inversion automatically. 

//=======================================================================================
// Macros 

// Memory 
#define PRIORITY_INVERSION_STACK_SIZE configMINIMAL_STACK_SIZE * 4 

// Data 
#define PRIORITY_INVERSION_STR_MAX_LEN 60 

// Timing 
#define PRIORITY_INVERSION_CS_WAIT 250         // Critical section time (ticks) 
#define PRIORITY_INVERSION_MEDIUM_WAIT 5000    // Medium task working time (ticks) 
#define PRIORITY_INVERSION_OUTPUT_DELAY 1000   // Serial terminal output setup wait (ms) 
#define PRIORITY_INVERSION_TASK_SLEEP 500      // Task sleep (ticks) 
#define PRIORITY_INVERSION_START_DELAY 100     // Task start delay to force PI (ticks) 

//=======================================================================================


//=======================================================================================
// Variables 

// Task definition: priorityInversionSetup 
osThreadId_t priorityInversionSetupHandle; 
const osThreadAttr_t priorityInversionSetup_attributes = 
{
    .name = "priorityInversionSetup", 
    .stack_size = PRIORITY_INVERSION_STACK_SIZE, 
    .priority = (osPriority_t) osPriorityNormal 
};

// Task definition: lowPriority 
osThreadId_t lowPriorityHandle; 
const osThreadAttr_t lowPriority_attributes = 
{
    .name = "lowPriority", 
    .stack_size = PRIORITY_INVERSION_STACK_SIZE, 
    .priority = (osPriority_t) osPriorityNormal 
};

// Task definition: mediumPriority 
osThreadId_t mediumPriorityHandle; 
const osThreadAttr_t mediumPriority_attributes = 
{
    .name = "mediumPriority", 
    .stack_size = PRIORITY_INVERSION_STACK_SIZE, 
    .priority = (osPriority_t) osPriorityNormal1 
};

// Task definition: highPriority 
osThreadId_t highPriorityHandle; 
const osThreadAttr_t highPriority_attributes = 
{
    .name = "highPriority", 
    .stack_size = PRIORITY_INVERSION_STACK_SIZE, 
    .priority = (osPriority_t) osPriorityNormal2 
};

// Semaphores 
static SemaphoreHandle_t lock; 

// Mutex(es) 
static SemaphoreHandle_t serial_mutex; 

//=======================================================================================


//=======================================================================================
// Prototypes 

/**
 * @brief Task function: priorityInversionSetup 
 * 
 * @param argument : NULL 
 */
void TaskPriorityInversionSetup(void *argument); 


/**
 * @brief Task function: lowPriority 
 * 
 * @param argument : NULL 
 */
void TaskLowPriority(void *argument); 


/**
 * @brief Task function: mediumPriority 
 * 
 * @param argument : NULL 
 */
void TaskMediumPriority(void *argument); 


/**
 * @brief Task function: highPriority 
 * 
 * @param argument : NULL 
 */
void TaskHighPriority(void *argument); 


/**
 * @brief Output task priority info to the serial terminal 
 * 
 * @param buff : string to output to the serial terminal 
 */
void task_priority_output(char *buff); 

//=======================================================================================


//=======================================================================================
// Setup 

void priority_inversion_init(void)
{
    // Create thread(s) 
    priorityInversionSetupHandle = 
        osThreadNew(TaskPriorityInversionSetup, NULL, &priorityInversionSetup_attributes); 

    // Create mutex 
    serial_mutex = xSemaphoreCreateMutex(); 
    lock = xSemaphoreCreateMutex(); 

    // Mutexes in FreeRTOS handle priority inversion automatically. If a low priority 
    // task blocks a high priority task due to the low priority task having a lock on 
    // the critical section, a mutex will make sure the low priority task executes its 
    // critical section so the higher priority task can then take over (bounded priority 
    // inversion). A semaphore does not do this automatically. 

    // Delay to allow user to open terminal 
    tim_delay_ms(TIM9, PRIORITY_INVERSION_OUTPUT_DELAY); 
}

//=======================================================================================


//=======================================================================================
// Loop 

void priority_inversion_loop(void)
{
    // Do nothing 
}

//=======================================================================================


//=======================================================================================
// Tasks and functions 

// Task function: priorityInversionSetup 
void TaskPriorityInversionSetup(void *argument)
{
    // The order of starting tasks is important to force priority inversion 
    lowPriorityHandle = osThreadNew(TaskLowPriority, NULL, &lowPriority_attributes); 
    osDelay(PRIORITY_INVERSION_START_DELAY);   // Delay to force priority inversion 
    highPriorityHandle = osThreadNew(TaskHighPriority, NULL, &highPriority_attributes); 
    mediumPriorityHandle = osThreadNew(TaskMediumPriority, NULL, &mediumPriority_attributes); 

    vTaskDelete(NULL); 
}


// Task function: lowPriority 
void TaskLowPriority(void *argument)
{
    TickType_t timestamp; 
    char priority_str[PRIORITY_INVERSION_STR_MAX_LEN]; 

    while (1)
    {
        // Take lock 
        task_priority_output("Task L trying to take lock...\r\n"); 
        timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS; 

        xSemaphoreTake(lock, portMAX_DELAY); 
        // Can use portENTER_CRITICAL instead of a mutex is desired. This will disable 
        // the scheduler and interrupts which will allow the critical section to run 
        // untouched. 
        // portENTER_CRITICAL(); 

        // Say how long we spend waiting for a lock 
        snprintf(
            priority_str, 
            PRIORITY_INVERSION_STR_MAX_LEN, 
            "Task L got lock. %lu ms waiting for lock. Working...\r\n", 
            (uint32_t)((xTaskGetTickCount() * portTICK_PERIOD_MS) - timestamp)); 
        task_priority_output(priority_str); 

        // Hog the processor and do nothing 
        timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS; 
        while (((xTaskGetTickCount() * portTICK_PERIOD_MS) - timestamp) < 
                PRIORITY_INVERSION_CS_WAIT); 

        // Release lock 
        task_priority_output("Task L releasing lock.\r\n"); 
        xSemaphoreGive(lock); 

        // If using portENTER_CRITICAL then use portEXIT_CRITICAL to exit the critical 
        // section. 
        // portEXIT_CRITICAL(); 

        // Go to sleep 
        osDelay(PRIORITY_INVERSION_TASK_SLEEP); 
    }

    vTaskDelete(NULL); 
}


// Task function: mediumPriority 
void TaskMediumPriority(void *argument)
{
    TickType_t timestamp; 

    while (1)
    {
        // Hog the processor and do nothing. This is done using a while loop 
        // instead of a task delay. 
        task_priority_output("Task M doing some work...\r\n"); 
        timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS; 
        while (((xTaskGetTickCount() * portTICK_PERIOD_MS) - timestamp) < 
                PRIORITY_INVERSION_MEDIUM_WAIT); 

        // Go to sleep 
        task_priority_output("Task M done!\r\n"); 
        osDelay(PRIORITY_INVERSION_TASK_SLEEP); 
    }

    vTaskDelete(NULL); 
}


// Task function: highPriority 
void TaskHighPriority(void *argument)
{
    TickType_t timestamp; 
    char priority_str[PRIORITY_INVERSION_STR_MAX_LEN]; 

    while (1)
    {
        // Take lock 
        task_priority_output("Task H trying to take lock...\r\n"); 
        timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS; 

        xSemaphoreTake(lock, portMAX_DELAY); 
        // Can use portENTER_CRITICAL instead of a mutex is desired. This will disable 
        // the scheduler and interrupts which will allow the critical section to run 
        // untouched. 
        // portENTER_CRITICAL(); 

        // Say how long we spend waiting for a lock 
        snprintf(
            priority_str, 
            PRIORITY_INVERSION_STR_MAX_LEN, 
            "Task H got lock. %lu ms waiting for lock. Working...\r\n", 
            (uint32_t)((xTaskGetTickCount() * portTICK_PERIOD_MS) - timestamp)); 
        task_priority_output(priority_str); 

        // Hog the processor and do nothing 
        timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS; 
        while (((xTaskGetTickCount() * portTICK_PERIOD_MS) - timestamp) < 
                PRIORITY_INVERSION_CS_WAIT); 

        // Release lock 
        task_priority_output("Task H releasing lock.\r\n"); 
        xSemaphoreGive(lock); 

        // If using portENTER_CRITICAL then use portEXIT_CRITICAL to exit the critical 
        // section. 
        // portEXIT_CRITICAL(); 

        // Go to sleep 
        osDelay(PRIORITY_INVERSION_TASK_SLEEP); 
    }

    vTaskDelete(NULL); 
}


// Output task priority info to the serial terminal 
void task_priority_output(char *buff)
{
    xSemaphoreTake(serial_mutex, portMAX_DELAY); 
    uart_sendstring(USART2, buff); 
    xSemaphoreGive(serial_mutex); 
}

//=======================================================================================

#endif 
