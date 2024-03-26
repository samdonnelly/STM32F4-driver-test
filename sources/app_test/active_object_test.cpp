/**
 * @file active_object_test.cpp
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Active Object design pattern test 
 * 
 * @version 0.1
 * @date 2024-03-21
 * 
 * @copyright Copyright (c) 2024
 * 
 */

//=======================================================================================
// Includes 

#include "active_object_test.h" 
#include "includes_drivers.h" 
#include "stm32f4xx_it.h" 

// FreeRTOS 
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
#define AO_C_TEST 0 
#define AO_CPP_TEST 0 

// Memory 
#define MIN_STACK_MULTIPLE 8 
#define THREAD_LOW_STACK_SIZE configMINIMAL_STACK_SIZE * MIN_STACK_MULTIPLE 
#define THREAD_HIGH_STACK_SIZE configMINIMAL_STACK_SIZE * MIN_STACK_MULTIPLE 

// Data 
#define SERIAL_INPUT_MAX_LEN 30 

// Timing 
#define LED_SLOW_BLINK_PERIOD 500   // (ticks) 
#define LED_FAST_BLINK_PERIOD 100   // (ticks) 

//=======================================================================================


//=======================================================================================
// Active Object module data 

//==================================================
// Datatype 

typedef uint8_t Event; 

//==================================================

//==================================================
// Objects 

// Thread Event Info 
typedef struct 
{
    osThreadAttr_t attr;              // Thread attributes 
    uint8_t event;                    // Event index 
    QueueHandle_t ThreadEventQueue;   // Queue 
    void (*dispatch)(Event event);    // Dispatch function 
}
ThreadEventData; 

//==================================================

//==================================================
// Prototypes 

/**
 * @brief Common event loop shared by all threads 
 * 
 * @param thread_info : thread info such as the event queue handler and dispatch function 
 */
void eventLoop(void *thread_info); 

//==================================================

//=======================================================================================


//=======================================================================================
// System threads 

class SystemData 
{
private:   // Private members 

    //==================================================
    // Low Priority Thread 

    // Low Priority Thread States 
    enum class ThreadLowStates
    {
        SERIAL_OUT_STATE, 
        SERIAL_IN_STATE, 
        NUM_STATES 
    } 
    thread_low_state; 

    // Low Priority Thread Events 
    enum class ThreadLowEvents
    {
        NO_EVENT, 
        SERIAL_OUT_EVENT, 
        SERIAL_IN_EVENT 
    }; 

    // System info 
    ThreadEventData thread_low_event_data; 

    // Flags 
    typedef struct 
    {
        // System flags 
        uint8_t state_entry : 1; 

        // State flags 
        uint8_t serial_out  : 1; 
        uint8_t serial_in   : 1; 
    } 
    ThreadLowFlags; 

    //==================================================

    //==================================================
    // High Priority Thread 

    // High Priority Thread States 
    enum class ThreadHighStates 
    {
        LED_SLOW_STATE, 
        LED_FAST_STATE, 
        NUM_STATES 
    }
    thread_high_state; 

    // High Priority Thread Events 
    enum class ThreadHighEvents 
    {
        NO_EVENT, 
        LED_TOGGLE_EVENT 
    }; 

    // System info 
    ThreadEventData thread_high_event_data; 

    // Flags 
    struct ThreadHighFlags
    {
        // System flags 
        uint8_t state_entry  : 1; 
        uint8_t state_change : 1; 

        // State flags 
        uint8_t led_slow     : 1; 
        uint8_t led_fast     : 1; 
    } 
    thread_high_flags; 

    //==================================================

    //==================================================
    // Software Timer Thread 
    //==================================================

    //==================================================
    // General 

    // Software timers 
    TimerHandle_t slow_blink_timer; 
    TimerHandle_t fast_blink_timer; 

    // LED pin data 
    GPIO_TypeDef *led_gpio; 
    gpio_pin_num_t led_pin; 
    gpio_pin_state_t led_state; 

    // Serial terminal data 
    uint8_t uart_dma_buff[SERIAL_INPUT_MAX_LEN];   // Circular buffer 
    uint8_t buff_index;                            // Circular buffer index 
    uint8_t user_in_buff[SERIAL_INPUT_MAX_LEN];    // Stores latest user input 

    //==================================================

    //==================================================
    // States 

    static void state0(Event event); 
    static void state1(Event event); 

    typedef void (*func_ptr)(Event event); 

    const func_ptr state_table[(uint8_t)ThreadLowStates::NUM_STATES] = 
    {
        &state0, 
        &state1 
    }; 

    //==================================================

public:   // Public members 
    
    //==================================================
    // Low Priority Thread 
    //==================================================

    //==================================================
    // High Priority Thread 
    //==================================================

    //==================================================
    // Software Timer Thread 
    //==================================================

private:   // Private member functions 
    
    //==================================================
    // Low Priority Thread 
    //==================================================

    //==================================================
    // High Priority Thread 
    //==================================================

    //==================================================
    // Software Timer Thread 
    //==================================================

public:   // Public member functions 

    // Constructor(s) 
    SystemData() {} 

    // Destructor(s) 
    ~SystemData() {} 
    
    //==================================================
    // Low Priority Thread
    //==================================================

    //==================================================
    // High Priority Thread
    //==================================================

    //==================================================
    // Software Timer Thread
    //==================================================
}; 

// System object 
static SystemData trackers; 

//=======================================================================================


//=======================================================================================
// Low Priority Thread module data 

// Local data to the low priority thread 

//==================================================
// Enums 

// Low Priority Thread states 
typedef enum {
    THREAD_LOW_SERIAL_OUT_STATE, 
    THREAD_LOW_SERIAL_IN_STATE, 
    THREAD_LOW_NUM_STATES 
} ThreadLowStates; 

// Low Priority Thread event indexes 
typedef enum {
    THREAD_LOW_NO_EVENT, 
    THREAD_LOW_SERIAL_OUT_EVENT, 
    THREAD_LOW_SERIAL_IN_EVENT 
} ThreadLowEvents; 

//==================================================

//==================================================
// Objects 

// Low Priority Thread trackers 
struct ThreadLowTrackers 
{
    // System info 
    ThreadEventData event_data; 
    ThreadLowStates state; 

    // Serial terminal data 
    uint8_t uart_dma_buff[SERIAL_INPUT_MAX_LEN];   // Circular buffer 
    uint8_t buff_index;                            // Circular buffer index 
    uint8_t user_in_buff[SERIAL_INPUT_MAX_LEN];    // Stores latest user input 

    // State flags 
    uint8_t state_entry : 1; 
    uint8_t serial_out  : 1; 
    uint8_t serial_in   : 1; 
}; 

// Low Priority Thread object 
static ThreadLowTrackers thread_low_trackers; 

//==================================================

//==================================================
// Variables 

// Queue info 
static const uint8_t thread_low_queue_len = 3; 

//==================================================

//==================================================
// Function pointers 

// State function pointers 
typedef void (*thread_low_state_ptr)(ThreadLowTrackers *trackers, Event event); 

//==================================================

//==================================================
// Prototypes 

// Setup 
void ThreadLowSetup(void); 

// Dispatch functions 
void DispatchThreadLow(Event event); 

// State functions 
void ThreadLowState0(ThreadLowTrackers *trackers, Event event); 
void ThreadLowState1(ThreadLowTrackers *trackers, Event event); 

//==================================================

//==================================================
// Control data 

// Low Priority Thread state table 
static const thread_low_state_ptr thread_low_state_table[THREAD_LOW_NUM_STATES] = 
{
    &ThreadLowState0,   // State 0 
    &ThreadLowState1    // State 1 
}; 

//==================================================

//=======================================================================================


//=======================================================================================
// High Priority Thread module data 

// Local data to the high priority thread 

//==================================================
// Enums 

// High Priority thread states 
typedef enum {
    THREAD_HIGH_LED_SLOW_STATE, 
    THREAD_HIGH_LED_FAST_STATE, 
    THREAD_HIGH_NUM_STATES 
} ThreadHighStates; 

// High Priority Thread event indexes 
typedef enum {
    THREAD_HIGH_NO_EVENT, 
    THREAD_HIGH_LED_TOGGLE_EVENT 
} ThreadHighEvents; 

//==================================================

//==================================================
// Objects 

// High Priority Thread trackers 
struct ThreadHighTrackers 
{
    // System info 
    ThreadEventData event_data; 
    ThreadHighStates state; 

    // LED timers 
    TimerHandle_t slow_blink_timer; 
    TimerHandle_t fast_blink_timer; 

    // LED pin data 
    GPIO_TypeDef *led_gpio; 
    gpio_pin_num_t led_pin; 
    gpio_pin_state_t led_state; 

    // State flags 
    uint8_t state_entry  : 1; 
    uint8_t state_change : 1; 
    uint8_t led_slow     : 1; 
    uint8_t led_fast     : 1; 
}; 

// High Priority Thread object 
static ThreadHighTrackers thread_high_trackers; 

//==================================================

//==================================================
// Variables 

// Queue info 
static const uint8_t thread_high_queue_len = 3; 

//==================================================

//==================================================
// Function pointers 

// State function pointers 
typedef void (*thread_high_state_ptr)(ThreadHighTrackers *trackers, Event event); 

//==================================================

//==================================================
// Prototypes 

// Setup 
void ThreadHighSetup(void); 

// Dispatch functions 
void DispatchThreadHigh(Event event); 

// State functions 
void ThreadHighState0(ThreadHighTrackers *trackers, Event event); 
void ThreadHighState1(ThreadHighTrackers *trackers, Event event); 

// Helper functions 
void ThreadHighStateToggle(void); 
void ThreadHighStateTrigger(void); 

//==================================================

//==================================================
// Control data 

// High Priority Thread state table 
static const thread_high_state_ptr thread_high_state_table[THREAD_HIGH_NUM_STATES] = 
{
    &ThreadHighState0,   // State 0 
    &ThreadHighState1    // State 1 
}; 

//==================================================

//=======================================================================================


//=======================================================================================
// Software Timer Thread 

//==================================================
// Prototypes 

// Called when LED toggle timers expire 
void LEDTimerCallback(TimerHandle_t xTimer); 

//==================================================

//=======================================================================================


//=======================================================================================
// Event module(s) 

//==================================================
// Prototypes 

/**
 * @brief Event: Serial Output 
 * 
 * @param output_buff : data to output 
 */
void SerialOutEvent(char *output_buff); 

/**
 * @brief Event: Serial Input 
 * 
 * @param circular_buff 
 * @param circular_buff_index 
 * @param input_buff 
 */
void SerialInEvent(
    uint8_t *circular_buff, 
    uint8_t *circular_buff_index, 
    uint8_t *input_buff); 

/**
 * @brief Event: Pin Toggle (used to toggle the board LED) 
 * 
 * @param pin_gpio 
 * @param pin_num 
 * @param pin_state 
 */
void PinToggleEvent(
    GPIO_TypeDef *pin_gpio, 
    gpio_pin_num_t pin_num, 
    gpio_pin_state_t *pin_state); 

//==================================================

//=======================================================================================


//=======================================================================================
// Test functions 

// Active Object test setup code 
void active_object_test_init(void)
{
    // Initialize FreeRTOS scheduler 
    osKernelInitialize(); 

    //==================================================
    // Other general setup (pins, ports, etc.) 

    // Initialize GPIO ports 
    gpio_port_init(); 

    //==================================================

    // Priority group setup 
    ThreadLowSetup(); 
    ThreadHighSetup(); 
}


// Active Object test code 
void active_object_test_app(void)
{
    // Start scheduler 
    osKernelStart(); 
}

//=======================================================================================


//=======================================================================================
// Active Object control 

// In practice the event loop and any other context switching or thread related code 
// should be in it's own file. The event loop would then dispatch to the thread it's 
// running in a different file. 

// Common event loop shared by all threads 
void eventLoop(void *thread_info)
{
    ThreadEventData *thread = (ThreadEventData *)thread_info; 

    // Event loop 
    while (1)
    {
        thread->event = CLEAR; 
        xQueueReceive(thread->ThreadEventQueue, (void *)&thread->event, portMAX_DELAY); 
        thread->dispatch(thread->event); 
    }

    vTaskDelete(NULL); 
}

//=======================================================================================


//=======================================================================================
// Low Priority Thread event control 

// In practice these dispatch functions would likely be separated into different files 
// to help organize the code. 

// Setup 
void ThreadLowSetup(void)
{
    // Initialize general data 
    thread_low_trackers.state = THREAD_LOW_SERIAL_OUT_STATE; 
    memset((void *)thread_low_trackers.uart_dma_buff, CLEAR, 
           sizeof(thread_low_trackers.uart_dma_buff)); 
    thread_low_trackers.buff_index = CLEAR; 
    memset((void *)thread_low_trackers.user_in_buff, CLEAR, 
           sizeof(thread_low_trackers.user_in_buff)); 
    thread_low_trackers.state_entry = SET_BIT; 
    thread_low_trackers.serial_out = SET_BIT; 
    thread_low_trackers.serial_in = CLEAR_BIT; 

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
        (uint32_t)thread_low_trackers.uart_dma_buff, 
        (uint16_t)SERIAL_INPUT_MAX_LEN); 
    dma_stream_enable(DMA1_Stream5); 

    // Initialize interrupt handler flags and enable the interrupt handler 
    int_handler_init(); 
    nvic_config(USART2_IRQn, EXTI_PRIORITY_15); 

    // Thread definition, queue handle creation and dispatch function assignment 
    thread_low_trackers.event_data = 
    {
        .attr = { .name = "ThreadLow", 
                  .attr_bits = CLEAR, 
                  .cb_mem = NULL, 
                  .cb_size = CLEAR, 
                  .stack_mem = NULL, 
                  .stack_size = THREAD_LOW_STACK_SIZE, 
                  .priority = (osPriority_t)osPriorityLow, 
                  .tz_module = CLEAR, 
                  .reserved = CLEAR }, 
        .event = CLEAR, 
        .ThreadEventQueue = xQueueCreate(thread_low_queue_len, sizeof(uint32_t)), 
        .dispatch = DispatchThreadLow 
    }; 
    // Check that the queues were created successfully 

    // Create the thread(s) 
    osThreadNew(
        eventLoop, 
        (void *)&thread_low_trackers.event_data, 
        &thread_low_trackers.event_data.attr); 
    // Check that the thread creation worked 

    uart_sendstring(USART2, "\r\n>>> "); 
}


// Dispatch function for low priority task 
void DispatchThreadLow(Event event)
{
    ThreadLowStates state = thread_low_trackers.state; 

    // Continuous events. These are thread events that happen irrespective of state. 

    // State machine 
    switch (state)
    {
        case THREAD_LOW_SERIAL_OUT_STATE: 
            if (thread_low_trackers.serial_in)
            {
                state = THREAD_LOW_SERIAL_IN_STATE; 
            }
            break; 
        
        case THREAD_LOW_SERIAL_IN_STATE: 
            if (thread_low_trackers.serial_out)
            {
                state = THREAD_LOW_SERIAL_OUT_STATE; 
            }
            break; 

        default: 
            state = THREAD_LOW_SERIAL_OUT_STATE; 
            break; 
    }

    // Run state function 
    thread_low_state_table[state](&thread_low_trackers, event); 
    thread_low_trackers.state = state; 
}


// In practice the state functions would likely be in the same file as the function that 
// dispatched them. Each dispatching function would likely have its own file. 

// Low Priority Thread: State 0 
void ThreadLowState0(
    ThreadLowTrackers *trackers, 
    Event event)
{
    // State entry 
    if (thread_low_trackers.state_entry)
    {
        thread_low_trackers.state_entry = CLEAR_BIT; 
    }

    // Event selection 
    switch (event)
    {
        case THREAD_LOW_SERIAL_OUT_EVENT: 
            SerialOutEvent((char *)thread_low_trackers.user_in_buff); 
            break; 

        default: 
            break; 
    }

    // State exit 
    if (thread_low_trackers.serial_in)
    {
        thread_low_trackers.state_entry = SET_BIT; 
        thread_low_trackers.serial_out = CLEAR_BIT; 
    }
}


// Low Priority Thread: State 1 
void ThreadLowState1(
    ThreadLowTrackers *trackers, 
    Event event)
{
    // State entry 
    if (thread_low_trackers.state_entry)
    {
        thread_low_trackers.state_entry = CLEAR_BIT; 
    }

    // Event selection 
    switch (event)
    {
        case THREAD_LOW_SERIAL_IN_EVENT: 
            SerialInEvent(
                thread_low_trackers.uart_dma_buff, 
                &thread_low_trackers.buff_index, 
                thread_low_trackers.user_in_buff); 

            // Toggle the high priority thread state 
            ThreadHighStateToggle(); 
            
            // Trigger the next state and event 
            thread_low_trackers.serial_out = SET_BIT; 
            thread_low_trackers.event_data.event = THREAD_LOW_SERIAL_OUT_EVENT; 
            xQueueSend(thread_low_trackers.event_data.ThreadEventQueue, 
                       (void *)&thread_low_trackers.event_data.event, 0); 
            break; 

        default: 
            break; 
    }

    // State exit 
    if (thread_low_trackers.serial_out)
    {
        thread_low_trackers.state_entry = SET_BIT; 
        thread_low_trackers.serial_in = CLEAR_BIT; 
    }
}


#if INTERRUPT_OVERRIDE 

// USART2 - overridden 
void USART2_IRQHandler(void)
{
    handler_flags.usart2_flag = SET_BIT; 

    BaseType_t task_woken = pdFALSE; 

    // Trigger a state change and queue a serial input event 
    thread_low_trackers.serial_out = CLEAR_BIT; 
    thread_low_trackers.serial_in = SET_BIT; 
    thread_low_trackers.state_entry = SET_BIT; 

    thread_low_trackers.event_data.event = THREAD_LOW_SERIAL_IN_EVENT; 
    xQueueSendFromISR(thread_low_trackers.event_data.ThreadEventQueue, 
                      (void *)&thread_low_trackers.event_data.event, &task_woken); 

    // The following is needed to exit the ISR 
    dummy_read(USART2->SR); 
    dummy_read(USART2->DR); 
}

#endif   // INTERRUPT_OVERRIDE 

//=======================================================================================


//=======================================================================================
// High Priority Thread event control 

// In practice these dispatch functions would likely be separated into different files 
// to help organize the code. 

// Setup 
void ThreadHighSetup(void)
{
    // Initialize general data 
    thread_high_trackers.state = THREAD_HIGH_LED_SLOW_STATE; 
    thread_high_trackers.led_gpio = GPIOA; 
    thread_high_trackers.led_pin = GPIOX_PIN_5; 
    thread_high_trackers.led_state = GPIO_LOW; 
    thread_high_trackers.state_entry = SET_BIT; 
    thread_high_trackers.state_change = CLEAR_BIT; 
    thread_high_trackers.led_slow = SET_BIT; 
    thread_high_trackers.led_fast = CLEAR_BIT; 

    // Initialize board LED (on when logic low) 
    gpio_pin_init(GPIOA, PIN_5, MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 

    // Thread definition, queue handle creation and dispatch function assignment 
    thread_high_trackers.event_data = 
    {
        .attr = { .name = "ThreadHigh", 
                  .attr_bits = CLEAR, 
                  .cb_mem = NULL, 
                  .cb_size = CLEAR, 
                  .stack_mem = NULL, 
                  .stack_size = THREAD_HIGH_STACK_SIZE, 
                  .priority = (osPriority_t)osPriorityHigh, 
                  .tz_module = CLEAR, 
                  .reserved = CLEAR }, 
        .event = THREAD_HIGH_NO_EVENT, 
        .ThreadEventQueue = xQueueCreate(thread_high_queue_len, sizeof(uint32_t)), 
        .dispatch = DispatchThreadHigh 
    }; 
    // Check that the queues were created successfully 

    // Create the thread(s) 
    osThreadNew(
        eventLoop, 
        (void *)&thread_high_trackers.event_data, 
        &thread_high_trackers.event_data.attr); 
    // Check that the thread creation worked 

    // Create timers 
    thread_high_trackers.slow_blink_timer = xTimerCreate(
        "one_shot_timer",               // Name of timer 
        LED_SLOW_BLINK_PERIOD,          // Period of timer (ticks) 
        pdTRUE,                         // Auto-relead --> pdTRUE == Repeat Timer 
        (void *)0,                      // Timer ID 
        LEDTimerCallback);              // Callback function 
    thread_high_trackers.fast_blink_timer = xTimerCreate(
        "one_shot_timer",               // Name of timer 
        LED_FAST_BLINK_PERIOD,          // Period of timer (ticks) 
        pdTRUE,                         // Auto-relead --> pdTRUE == Repeat Timer 
        (void *)1,                      // Timer ID 
        LEDTimerCallback);              // Callback function 
    // Check that timers were created successfully 

    // Queue and empty event to start the LED blinks 
    xQueueSend(thread_high_trackers.event_data.ThreadEventQueue, 
        (void *)&thread_high_trackers.event_data.event, 0); 
}


// Dispatch function for high priority thread 
void DispatchThreadHigh(Event event)
{
    // A thread does not necessarily need a state machine. Instead, there could simply be 
    // a list of events available for when they're requested. For example, the high 
    // priority thread could be dedicated to reading data from devices irrespective of 
    // state and each read gets triggered by a timer. 

    ThreadHighStates state = thread_high_trackers.state; 

    // Continuous events. These are thread events that happen irrespective of state. 

    // State machine 
    switch (state)
    {
        case THREAD_HIGH_LED_SLOW_STATE: 
            if (thread_high_trackers.led_fast)
            {
                state = THREAD_HIGH_LED_FAST_STATE; 
            }
            break; 

        case THREAD_HIGH_LED_FAST_STATE: 
            if (thread_high_trackers.led_slow)
            {
                state = THREAD_HIGH_LED_SLOW_STATE; 
            }
            break; 
        
        default: 
            state = THREAD_HIGH_LED_SLOW_STATE; 
            break; 
    }

    // Run state function 
    thread_high_state_table[state](&thread_high_trackers, event); 
    thread_high_trackers.state = state; 
}


// In practice the state functions would likely be in the same file as the function that 
// dispatched them. Each dispatching function would likely have its own file. 

// High Priority Thread: State 0 
void ThreadHighState0(
    ThreadHighTrackers *trackers, 
    Event event)
{
    // State entry 
    if (thread_high_trackers.state_entry)
    {
        thread_high_trackers.state_entry = CLEAR_BIT; 

        xTimerStart(thread_high_trackers.slow_blink_timer, 0); 
    }

    // Event selection 
    switch (event)
    {
        case THREAD_HIGH_LED_TOGGLE_EVENT: 
            PinToggleEvent(
                thread_high_trackers.led_gpio, 
                thread_high_trackers.led_pin, 
                &thread_high_trackers.led_state); 

            if (thread_high_trackers.state_change)
            {
                thread_high_trackers.state_change = CLEAR_BIT; 
                thread_high_trackers.led_fast = SET_BIT; 

                // Trigger the next state with an empty event 
                ThreadHighStateTrigger(); 
            }
            break; 

        default: 
            break; 
    }

    // State exit 
    if (thread_high_trackers.led_fast)
    {
        thread_high_trackers.state_entry = SET_BIT; 
        thread_high_trackers.led_slow = CLEAR_BIT; 

        xTimerStop(thread_high_trackers.slow_blink_timer, 0); 
    }
}


// High Priority Thread: State 1 
void ThreadHighState1(
    ThreadHighTrackers *trackers, 
    Event event)
{
    // State entry 
    if (thread_high_trackers.state_entry)
    {
        thread_high_trackers.state_entry = CLEAR_BIT; 

        xTimerStart(thread_high_trackers.fast_blink_timer, 0); 
    }

    // Event selection 
    switch (event)
    {
        case THREAD_HIGH_LED_TOGGLE_EVENT: 
            PinToggleEvent(
                thread_high_trackers.led_gpio, 
                thread_high_trackers.led_pin, 
                &thread_high_trackers.led_state); 

            if (thread_high_trackers.state_change)
            {
                thread_high_trackers.state_change = CLEAR_BIT; 
                thread_high_trackers.led_slow = SET_BIT; 

                // Trigger the next state with an empty event 
                ThreadHighStateTrigger(); 
            }
            break; 

        default: 
            break; 
    }

    // State exit 
    if (thread_high_trackers.led_slow)
    {
        thread_high_trackers.state_entry = SET_BIT; 
        thread_high_trackers.led_fast = CLEAR_BIT; 

        xTimerStop(thread_high_trackers.fast_blink_timer, 0); 
    }
}


// Toggle the high priority thread state 
void ThreadHighStateToggle(void)
{
    thread_high_trackers.state_change = SET_BIT; 
}


// Trigger/start the next state 
void ThreadHighStateTrigger(void)
{
    // Queue an empty event to get the thread to change states and start blinking the 
    // LED at an updated rate immediately. 
    thread_high_trackers.event_data.event = THREAD_HIGH_NO_EVENT; 
    xQueueSend(thread_high_trackers.event_data.ThreadEventQueue, 
        (void *)&thread_high_trackers.event_data.event, 0); 
}

//=======================================================================================


//=======================================================================================
// Software Timer Thread 

// Called when LED toggle timers expire 
void LEDTimerCallback(TimerHandle_t xTimer)
{
    // Queue an LED toggle event 
    thread_high_trackers.event_data.event = THREAD_HIGH_LED_TOGGLE_EVENT; 
    xQueueSend(thread_high_trackers.event_data.ThreadEventQueue, 
        (void *)&thread_high_trackers.event_data.event, 0); 
}

//=======================================================================================


//=======================================================================================
// Event modules 

// In practice these would be independent modules that can be used wherever needed. 

// Event: Serial Output 
void SerialOutEvent(char *output_buff)
{
    if (output_buff != NULL)
    {
        // Output the latest content of the circular buffer to the serial terminal 
        uart_sendstring(USART2, "Echo: "); 
        uart_sendstring(USART2, (char *)thread_low_trackers.user_in_buff); 
        uart_sendstring(USART2, "\r\n>>> "); 
    }
}


// Event: Serial Input 
void SerialInEvent(
    uint8_t *circular_buff, 
    uint8_t *circular_buff_index, 
    uint8_t *input_buff)
{
    handler_flags.usart2_flag = CLEAR; 

    // Get the user input from the circular buffer 
    cb_parse(
        circular_buff, 
        input_buff, 
        circular_buff_index, 
        SERIAL_INPUT_MAX_LEN); 
}


// Event: Pin Toggle (used to toggle the board LED) 
void PinToggleEvent(
    GPIO_TypeDef *pin_gpio, 
    gpio_pin_num_t pin_num, 
    gpio_pin_state_t *pin_state)
{
    *pin_state = (gpio_pin_state_t)(GPIO_HIGH - *pin_state); 
    gpio_write(pin_gpio, pin_num, *pin_state); 
}

//=======================================================================================
