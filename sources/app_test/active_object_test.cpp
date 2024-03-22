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
// Low Priority Thread module data 

// Local data to the low priority thread 

//==================================================
// Enums 

// Low Priority Thread states 
typedef enum {
    THREAD_LOW_STATE_0, 
    THREAD_LOW_STATE_1, 
    THREAD_LOW_NUM_STATES 
} ThreadLowStates; 

// Low Priority Thread event indexes 
typedef enum {
    THREAD_LOW_NO_EVENT, 
    THREAD_LOW_EVENT_0, 
    THREAD_LOW_EVENT_1, 
    THREAD_LOW_EVENT_2, 
} ThreadLowEvents; 

//==================================================

//==================================================
// Objects 

// Low Priority Thread trackers 
struct ThreadLowTrackers 
{
    ThreadEventData event_data; 
    ThreadLowStates state; 
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
    THREAD_HIGH_STATE_0, 
    THREAD_HIGH_STATE_1, 
    THREAD_HIGH_NUM_STATES 
} ThreadHighStates; 

// High Priority Thread event indexes 
typedef enum {
    THREAD_HIGH_NO_EVENT, 
    THREAD_HIGH_EVENT_0, 
    THREAD_HIGH_EVENT_1, 
    THREAD_HIGH_EVENT_2, 
} ThreadHighEvents; 

//==================================================

//==================================================
// Objects 

// High Priority Thread trackers 
struct ThreadHighTrackers 
{
    ThreadEventData event_data; 
    ThreadHighStates state; 
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
// Event module(s) 

//==================================================
// Prototypes 

// Events 
void Event0(void); 
void Event1(void); 
void Event2(void); 
void Event3(void); 
void Event4(void);
void Event5(void); 

//==================================================

//=======================================================================================


//=======================================================================================
// Test functions 

// Active Object test setup code 
void active_object_test_init(void)
{
    // Initialize FreeRTOS scheduler 
    osKernelInitialize(); 

    // Other general setup (pins, ports, etc.) 

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
}


// Dispatch function for low priority task 
void DispatchThreadLow(Event event)
{
    ThreadLowStates state = thread_low_trackers.state; 

    // Continuous events 

    // State machine 
    switch (state)
    {
        case THREAD_LOW_STATE_0: 
            state = THREAD_LOW_STATE_1; 
            break; 

        case THREAD_LOW_STATE_1: 
            state = THREAD_LOW_STATE_0; 
            break; 
        
        default: 
            state = THREAD_LOW_STATE_0; 
            break; 
    }

    // if (xQueueSend(queue, (void *)&e, 0) == pdTRUE); 

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

    // Event selection 
    switch (event)
    {
        case THREAD_LOW_EVENT_0: 
            Event0(); 
            break; 

        case THREAD_LOW_EVENT_1: 
            Event1(); 
            break; 

        default: 
            break; 
    }

    // State exit 
}


// Low Priority Thread: State 1 
void ThreadLowState1(
    ThreadLowTrackers *trackers, 
    Event event)
{
    // State entry 

    // Event selection 
    switch (event)
    {
        case THREAD_LOW_EVENT_0: 
            Event0(); 
            break; 

        case THREAD_LOW_EVENT_2: 
            Event2(); 
            break; 

        default: 
            break; 
    }

    // State exit 
}

//=======================================================================================


//=======================================================================================
// High Priority Thread event control 

// In practice these dispatch functions would likely be separated into different files 
// to help organize the code. 

// Setup 
void ThreadHighSetup(void)
{
    // Thread definition, queue handle creation and dispatch function assignment 
    thread_low_trackers.event_data = 
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
        .event = CLEAR, 
        .ThreadEventQueue = xQueueCreate(thread_high_queue_len, sizeof(uint32_t)), 
        .dispatch = DispatchThreadHigh 
    }; 
    // Check that the queues were created successfully 

    // Create the thread(s) 
    osThreadNew(
        eventLoop, 
        (void *)&thread_low_trackers.event_data, 
        &thread_low_trackers.event_data.attr); 
    // Check that the thread creation worked 
}


// Dispatch function for high priority thread 
void DispatchThreadHigh(Event event)
{
    // A thread does not necessarily need a state machine. Instead, there could simply be 
    // a list of events available for when they're requested. For example, the high 
    // priority thread could be dedicated to reading data from devices irrespective of 
    // state and each read gets triggered by a timer. 

    ThreadHighStates state = thread_high_trackers.state; 

    // Continuous events 

    // State machine 
    switch (state)
    {
        case THREAD_HIGH_STATE_0: 
            state = THREAD_HIGH_STATE_1; 
            break; 

        case THREAD_HIGH_STATE_1: 
            state = THREAD_HIGH_STATE_0; 
            break; 
        
        default: 
            state = THREAD_HIGH_STATE_0; 
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

    // Event selection 
    switch (event)
    {
        case THREAD_HIGH_EVENT_0: 
            Event3(); 
            break; 

        case THREAD_HIGH_EVENT_1: 
            Event4(); 
            break; 

        default: 
            break; 
    }

    // State exit 
}


// High Priority Thread: State 1 
void ThreadHighState1(
    ThreadHighTrackers *trackers, 
    Event event)
{
    // State entry 

    // Event selection 
    switch (event)
    {
        case THREAD_HIGH_EVENT_0: 
            Event3(); 
            break; 

        case THREAD_HIGH_EVENT_2: 
            Event5(); 
            break; 

        default: 
            break; 
    }

    // State exit 
}

//=======================================================================================


//=======================================================================================
// Event modules 

// In practice these would be independent modules that can be used wherever needed. 

// Event 0 
void Event0(void)
{
    // 
}


// Event 1 
void Event1(void)
{
    // 
}


// Event 2 
void Event2(void)
{
    // 
}


// Event 3 
void Event3(void)
{
    // 
}


// Event 4 
void Event4(void)
{
    // 
}


// Event 5 
void Event5(void)
{
    // 
}

//=======================================================================================
