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
// Variables 

// Thread handles 
static osThreadId_t ThreadLowHandle = nullptr; 
static osThreadId_t ThreadHighHandle = nullptr; 

// Thread definition: Low Priority 
const static osThreadAttr_t thread_low_attributes = 
{
    .name = "ThreadLow", 
    .attr_bits = CLEAR, 
    .cb_mem = NULL, 
    .cb_size = CLEAR, 
    .stack_mem = NULL, 
    .stack_size = THREAD_LOW_STACK_SIZE, 
    .priority = (osPriority_t)osPriorityLow, 
    .tz_module = CLEAR, 
    .reserved = CLEAR 
}; 

// Thread definition: High Priority 
const static osThreadAttr_t thread_high_attributes = 
{
    .name = "ThreadHigh", 
    .attr_bits = CLEAR, 
    .cb_mem = NULL, 
    .cb_size = CLEAR, 
    .stack_mem = NULL, 
    .stack_size = THREAD_HIGH_STACK_SIZE, 
    .priority = (osPriority_t)osPriorityHigh, 
    .tz_module = CLEAR, 
    .reserved = CLEAR 
}; 


// Queue info 
static const uint8_t queue_len = 3; 
static QueueHandle_t ThreadLowEventQueue; 
static QueueHandle_t ThreadHighEventQueue; 

//=======================================================================================


//=======================================================================================
// Prototypes 

/**
 * @brief Common event loop shared by all threads 
 * 
 * @param argument : 
 */
void eventLoop(void *argument); 


// Dispatch functions 
void DispatchThreadLow(void); 
void DispatchThreadHigh(void); 

// State functions 
void ThreadLowState0(void); 
void ThreadLowState1(void); 
void ThreadHighState0(void); 
void ThreadHighState1(void); 

// Events 
void ThreadLowEvent0(void); 
void ThreadLowEvent1(void); 
void ThreadLowEvent2(void); 
void ThreadHighEvent0(void); 
void ThreadHighEvent1(void);
void ThreadHighEvent2(void); 

//=======================================================================================


//=======================================================================================
// Setup 

// Active Object test setup code 
void active_object_test_init(void)
{
    // Initialize FreeRTOS scheduler 
    osKernelInitialize(); 

    // Create the thread(s) 
    ThreadLowHandle = osThreadNew(eventLoop, NULL, &thread_low_attributes); 
    ThreadHighHandle = osThreadNew(eventLoop, NULL, &thread_high_attributes); 

    // Create queue handle(s) 
    ThreadLowEventQueue = xQueueCreate(queue_len, sizeof(uint32_t)); 
    ThreadHighEventQueue = xQueueCreate(queue_len, sizeof(uint32_t)); 
}

//=======================================================================================


//=======================================================================================
// Test code 

// Active Object test code 
void active_object_test_app(void)
{
    // Start scheduler 
    osKernelStart(); 
}

#if AO_C_TEST 
#elif AO_CPP_TEST 
#endif 

//=======================================================================================


//=======================================================================================
// Event loop 

// Common event loop shared by all threads 
void eventLoop(void *argument)
{
    // 

    // Event loop 
    while (1)
    {
        // Queue 
        // xQueueReceive(queue, (void *)&event, portMAX_DELAY); 
        // Dispatch 
    }

    vTaskDelete(NULL); 
}

//=======================================================================================


//=======================================================================================
// Dispatch functions 

// Dispatch function for low priority task 
void DispatchThreadLow(void)
{
    // State machine 

    // if (xQueueSend(queue, (void *)&e, 0) == pdTRUE); 
}


// Dispatch function for high priority thread 
void DispatchThreadHigh(void)
{
    // State machine 
}

//=======================================================================================


//=======================================================================================
// State functions 

// Low Priority Thread: State 0 
void ThreadLowState0(void)
{
    // Event switch statement 
}


// Low Priority Thread: State 1 
void ThreadLowState1(void)
{
    // Event switch statement 
}


// High Priority Thread: State 0 
void ThreadHighState0(void)
{
    // Event switch statement 
}


// High Priority Thread: State 1 
void ThreadHighState1(void)
{
    // Event switch statement 
}


//=======================================================================================


//=======================================================================================
// Event functions 

// In practice these would each be an independent module that can be used wherever 
// needed. 

// Low Priority Thread: Event 0 
void ThreadLowEvent0(void)
{
    // 
}


// Low Priority Thread: Event 1 
void ThreadLowEvent1(void)
{
    // 
}


// Low Priority Thread: Event 2 
void ThreadLowEvent2(void)
{
    // 
}


// High Priority Thread: Event 0 
void ThreadHighEvent0(void)
{
    // 
}


// High Priority Thread: Event 1 
void ThreadHighEvent1(void)
{
    // 
}


// High Priority Thread: Event 2 
void ThreadHighEvent2(void)
{
    // 
}

//=======================================================================================
