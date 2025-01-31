/**
 * @file sik_radio_test.cpp
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief SiK radio driver test 
 * 
 * @details Setup 
 *          - Hardware 
 *            * STM32F4 microcontroller with a serial connection to a PC 
 *            * Two SiK telemetry radios (or RFD900): 
 *              1. One connected to the STM32F4 
 *              2. Another connected to a PC running Mission Planner 
 *          - Software 
 *            * Mission Planner on a PC with an established USB connection to one of the 
 *              telemetry radios and using the MAVLINK protocol. 
 *            * Serial monitor on a PC to allow the exchange of info with the STM32F4. 
 *          - Notes 
 *            * A single PC connected to the STM32F4 and running Mission Planner can be 
 *              used or it can be two separate computers. 
 *            * Certain communication (AT/RT) settings must match between both telemetry 
 *              radios in order for them to work together. See the SiK radio documentation 
 *              for more details. 
 *            * A connection between both radios must be established before any 
 *              meaningful work can be done by this code. 
 *          
 *          Configuration 
 *          - UART 
 *            * Two UART ports must be configured, one for the serial terminal connection 
 *              and another for the SiK radio. 
 *          - DMA 
 *            * Since data can be sent to the STM32F4 via UART from either the serial 
 *              terminal or the SiK radio without the STM32F4 requesting it, DMA is 
 *              configured for both UART ports so the data automatically gets stored 
 *              in a buffer. This prevents and loss of data. 
 *          - Interrupts 
 *            * Interrupts are configured for both UART ports for when an RX line is 
 *              detected to have gone idle. This indicates that there was data being 
 *              received and that the data is now done being received. The test code 
 *              won't perform any actions unless and interrupt occurs. 
 *          
 *          Dependencies 
 *          - STM32F4 driver library 
 *            - This library provides an easy interface to the SiK radio device driver 
 *              as well as UART, DMA and interrupt drivers. 
 *          - MAVLINK V2 C library 
 *            - The SiK radios and Mission Planner are designed to work with the MAVLINK 
 *              protocol. This code uses the library to decode and encode MAVLINK 
 *              messages. 
 *            - There are pre-built C and Python libraries but specific library versions 
 *              can be built manually. The pre-built C library works for this 
 *              application. 
 *          
 *          Procedure 
 *          - This code looks for data received via UART from both the SiK telemetry 
 *            radio and the serial terminal. When data is input from either source, 
 *            DMA will transfer the data to a circular buffer and an interrupt will be 
 *            triggered once there is no more data to transfer. This interrupt causes 
 *            the code to parse the circular buffer data into the complete input which 
 *            is then used to carry out various commands. In addition to these inputs, 
 *            there is also a periodic timer interrupt which carries out actions at 
 *            fixed intervals. 
 *          
 *          - There are two primary modes for this test: MAVLink mode and AT command 
 *            mode. 
 *            * MAVLink mode (default) uses the SiK radio as a serial bridge to send and 
 *              receive MAVLink messages with an external telemtry radio. Note that the 
 *              radio can be configured to not use MAVLink packet framing but that is 
 *              not supported in this test. In this mode, the device will attempt to 
 *              decode inputs into MAVLink messages & payloads and it will also send 
 *              MAVLink messages both periodically and at the request of the user. There 
 *              is a sub-mode within MAVLink mode which is user input mode. By default 
 *              user input mode is disabled and in this state the IDs of any received
 *              MAVLink message will be output to the serial terminal for the user to 
 *              see. User input mode can be entered from MAVLink mode by inputting 
 *              anything into the serial terminal (i.e. inputting some formatted command 
 *              will have no other affect). Once in user input mode, message IDs will 
 *              stop being displayed and instead a prompt will be shown where the user 
 *              can input other commands. You must be in user input mode to send commands 
 *              to the radio. From user input mode you can transition to AT command mode. 
 *            * AT command mode is used to configure the settings of the SiK radio. To 
 *              enter this mode, you must input the appropriate command from MAVLink 
 *              user input mode. In this mode, commands input by the user will be relayed 
 *              to the radio and any responses from the radio will be echoed to the 
 *              serial terminal for the user to see. This code provides the interface to 
 *              configure the radio but for info on how to properly configure the radio 
 *              the device documentation should be used. You can exit this mode using the 
 *              appropriate command at the prompt and you will be taken back to user 
 *              input mode in MAVLink mode. 
 *          
 *          - When attempting to transition from MAVLink user input mode to AT command 
 *            mode, the code will enter a temporary "request" mode. The request will be 
 *            sent to the radio to enter AT command mode and the code will wait on the 
 *            response from the radio indicating that AT command mode has been entered. 
 *            In the meantime no other actions can be requested by the user (it typically 
 *            takes ~1s for the radio to enter AT mode). If the confirmation is not seen 
 *            from the radio after a certain amount of time then the request will time 
 *            out and the user will remain in MAVLink user input mode. 
 *          
 *          - For a list of commands available to the user, see the list of pre-defined 
 *            strings below under the "Commands" section. 
 * 
 * @version 0.1
 * @date 2024-12-11
 * 
 * @copyright Copyright (c) 2024
 * 
 */

//=======================================================================================
// Includes 

#include "sik_radio_test.h" 
#include "stm32f4xx_it.h" 

extern "C"
{
    // For C headers without C++ guards 
    #include "common/mavlink.h" 
}

//=======================================================================================


//=======================================================================================
// Macros 

#define SIK_TEST_MSG_BUFF_SIZE 200 
#define SIK_TEST_SYS_ID 1            // GCS IDs start at 255, systems start at 1 
#define SIK_TEST_HB_TIMEOUT 10 
#define SIK_TEST_AT_TIMEOUT 5 

//=======================================================================================


//=======================================================================================
// Global data 

// User & device data 
typedef struct sik_serial_data_s 
{
    USART_TypeDef *uart; 
    DMA_Stream_TypeDef *dma_stream; 
    uint8_t cb[SIK_TEST_MSG_BUFF_SIZE];              // Circular buffer populated by DMA 
    cb_index_t cb_index;                             // Circular buffer indexing info 
    dma_index_t dma_index;                           // DMA transfer indexing info 
    uint8_t data_in_buff[SIK_TEST_MSG_BUFF_SIZE];    // Buffer that stores latest UART input 
    uint8_t data_out_buff[SIK_TEST_MSG_BUFF_SIZE];   // Buffer that stores outgoing data 
    uint16_t data_in_index;                          // Data input buffer index 
}
sik_serial_data_t; 

static sik_serial_data_t radio_data; 
static sik_serial_data_t user_data; 


// System MAVLink data 
typedef struct sik_system_data_s 
{
    // MAVLink identification 
    int channel; 
    uint8_t system_id; 
    uint8_t component_id; 

    // MAVLink packet handling 
    mavlink_message_t msg; 
    mavlink_status_t status; 

    // Messages 
    mavlink_heartbeat_t heartbeat; 
    mavlink_global_position_int_t global_position; 

    // Timers 
    uint8_t heartbeat_timer; 
    uint8_t at_mode_timer; 

    // Status 
    uint8_t at_mode           : 1;   // AT command mode flag 
    uint8_t at_mode_requested : 1;   // AT command mode requested flag 
    uint8_t ui_mode           : 1;   // User input mode flag 
    uint8_t connected         : 1;   // Radio connected flag 
}
sik_system_data_t; 

static sik_system_data_t system_data; 


const char 
// Formatting 
sik_test_user_prompt[] = "\r\n>>> ", 
sik_test_overwrite[] = "\r", 
// Feedback 
sik_test_user_msgid[] = "msgid: %lu\r\n", 
// Commands 
sik_test_user_exitui[] = "exitui", 
// Status 
sik_test_at_request[] = "Requesting AT command mode... ", 
sik_test_at_confirm[] = "OK\r\n", 
sik_test_timeout_msg[] = "timeout\r\n"; 

//=======================================================================================


//=======================================================================================
// Prototypes 

/**
 * @brief AT command mode radio input decode 
 * 
 * @details Used to decode radio input when in AT command mode. This function doesn't so 
 *          much decode as it does echo the radio input to the serial terminal for the 
 *          user to see. It's labelled as a decoding function to better align with the 
 *          other input mode functions. 
 *          
 *          When a user inputs a valid AT command while in AT command mode, the radio 
 *          should respond with the requested info. Echoing this input helps the user 
 *          configure the radio as needed. 
 */
void sik_radio_test_at_radio_decode(void); 


/**
 * @brief MAVLink mode radio input decode 
 * 
 * @details Used to decode radio input while in MAVLink mode. This function is primarily 
 *          used to decode MAVLink messages received by the radio, however if in MAVLink 
 *          mode and the user requests to go to AT command mode, this function will check 
 *          if the radio responds with an acknowledgement that it has entere AT mode. 
 */
void sik_radio_test_mavlink_radio_decode(void); 


/**
 * @brief MAVLink message payload decode 
 * 
 * @details Decodes the payload of a received MAVLink message once the received radio 
 *          data is decoded into a complete MAVLink message. A function like this is 
 *          recommended by the MAVLink documentation for identifying and handling 
 *          messages. Different messages can be added as needed. 
 */
void sik_radio_test_mavlink_payload_decode(void); 


/**
 * @brief AT command mode user input decode 
 * 
 * @details Used to decode user input when in AT command mode. Some decoding is done to 
 *          check for specific user commands but if no match is found then the user input 
 *          is forwarded to the radio. This gives the user easy control over configuring 
 *          the radio. 
 */
void sik_radio_test_at_user_decode(void); 


/**
 * @brief MAVLink mode user input decode 
 * 
 * @details Used to decode user input when in MAVLink mode. A match to specific predefined 
 *          commands is checked for. If no match is found then nothing happens. When the 
 *          system is in MAVLink mode and not in user input mode then any input will trigger 
 *          user input mode and nothing else. 
 */
void sik_radio_test_mavlink_user_decode(void); 


/**
 * @brief MAVlink mode periodic actions 
 * 
 * @details This function performs periodic actions when in MAVLink mode. 
 */
void sik_radio_test_mavlink_periodic(void); 


/**
 * @brief User prompt 
 * 
 * @param user_msg : message to send to the user 
 */
void sik_radio_test_user_output(const char *user_msg); 


/**
 * @brief AT command mode request reset 
 * 
 * @param user_msg : request status message to send to the user 
 */
void sik_radio_test_at_request_reset(const char *user_msg); 

//=======================================================================================


//=======================================================================================
// Setup code 

void sik_radio_test_init(void)
{
    //==================================================
    // Initialize data 

    // Radio data 
    radio_data.uart = USART1; 
    radio_data.dma_stream = DMA2_Stream2; 
    memset((void *)radio_data.cb, CLEAR, sizeof(radio_data.cb)); 
    radio_data.cb_index.cb_size = SIK_TEST_MSG_BUFF_SIZE; 
    radio_data.cb_index.head = CLEAR; 
    radio_data.cb_index.tail = CLEAR; 
    radio_data.dma_index.data_size = CLEAR; 
    radio_data.dma_index.ndt_old = dma_ndt_read(radio_data.dma_stream); 
    radio_data.dma_index.ndt_new = CLEAR; 
    memset((void *)radio_data.data_in_buff, CLEAR, sizeof(radio_data.data_in_buff)); 
    memset((void *)radio_data.data_out_buff, CLEAR, sizeof(radio_data.data_out_buff)); 
    radio_data.data_in_index = CLEAR; 

    // User data 
    user_data.uart = USART2; 
    user_data.dma_stream = DMA1_Stream5; 
    memset((void *)user_data.cb, CLEAR, sizeof(user_data.cb)); 
    user_data.cb_index.cb_size = SIK_TEST_MSG_BUFF_SIZE; 
    user_data.cb_index.head = CLEAR; 
    user_data.cb_index.tail = CLEAR; 
    user_data.dma_index.data_size = CLEAR; 
    user_data.dma_index.ndt_old = dma_ndt_read(user_data.dma_stream); 
    user_data.dma_index.ndt_new = CLEAR; 
    memset((void *)user_data.data_in_buff, CLEAR, sizeof(user_data.data_in_buff)); 
    memset((void *)user_data.data_out_buff, CLEAR, sizeof(user_data.data_out_buff)); 
    user_data.data_in_index = CLEAR; 

    // System data 
    system_data.channel = MAVLINK_COMM_0; 
    system_data.system_id = SIK_TEST_SYS_ID; 
    system_data.component_id = MAV_COMP_ID_TELEMETRY_RADIO; 
    system_data.heartbeat.custom_mode = CLEAR; 
    system_data.heartbeat.type = MAV_TYPE_SURFACE_BOAT; 
    system_data.heartbeat.autopilot = MAV_AUTOPILOT_GENERIC_MISSION_FULL; 
    system_data.heartbeat.base_mode = MAV_MODE_FLAG_GUIDED_ENABLED; 
    system_data.heartbeat.system_status = MAV_STATE_ACTIVE; 
    system_data.heartbeat_timer = CLEAR; 
    system_data.at_mode_timer = CLEAR; 
    system_data.at_mode = CLEAR_BIT; 
    system_data.at_mode_requested = CLEAR_BIT; 
    system_data.ui_mode = CLEAR_BIT; 
    system_data.connected = CLEAR_BIT; 

    //==================================================

    //==================================================
    // General setup 

    // Initialize GPIO ports 
    gpio_port_init(); 

    // Periodic (counter update) interrupt timer 
    tim_9_to_11_counter_init(
        TIM9, 
        TIM_84MHZ_100US_PSC, 
        0x2710,   // ARR=10000, (10000 counts)*(100us/count) = 1s 
        TIM_UP_INT_ENABLE); 
    tim_enable(TIM9); 
    
    //==================================================

    //==================================================
    // UART init 

    // UART1 init - SiK radio module 
    uart_init(
        radio_data.uart, 
        GPIOA, 
        PIN_10, 
        PIN_9, 
        UART_FRAC_84_115200, 
        UART_MANT_84_115200, 
        UART_DMA_DISABLE, 
        UART_DMA_ENABLE); 

    // UART1 interrupt init - SiK radio module - IDLE line (RX) interrupts 
    uart_interrupt_init(
        radio_data.uart, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE, 
        UART_INT_ENABLE, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE); 

    // UART2 init - Serial terminal 
    uart_init(
        user_data.uart, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_FRAC_42_9600, 
        UART_MANT_42_9600, 
        UART_DMA_DISABLE, 
        UART_DMA_ENABLE); 

    // UART2 interrupt init - Serial terminal - IDLE line (RX) interrupts 
    uart_interrupt_init(
        user_data.uart, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE, 
        UART_INT_ENABLE, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE); 
    
    //==================================================

    //==================================================
    // DMA 

    // DMA2 stream init - UART1 - SiK radio module 
    dma_stream_init(
        DMA2, 
        radio_data.dma_stream, 
        DMA_CHNL_4, 
        DMA_DIR_PM, 
        DMA_CM_ENABLE,
        DMA_PRIOR_VHI, 
        DMA_DBM_DISABLE, 
        DMA_ADDR_INCREMENT,   // Increment the buffer pointer to fill the buffer 
        DMA_ADDR_FIXED,       // No peripheral increment - copy from DR only 
        DMA_DATA_SIZE_BYTE, 
        DMA_DATA_SIZE_BYTE); 

    // DMA2 stream config - UART1 - SiK radio module 
    dma_stream_config(
        radio_data.dma_stream, 
        (uint32_t)(&radio_data.uart->DR), 
        (uint32_t)radio_data.cb, 
        (uint32_t)NULL, 
        (uint16_t)SIK_TEST_MSG_BUFF_SIZE); 

    // DMA1 stream init - UART2 - Serial terminal 
    dma_stream_init(
        DMA1, 
        user_data.dma_stream, 
        DMA_CHNL_4, 
        DMA_DIR_PM, 
        DMA_CM_ENABLE,
        DMA_PRIOR_HI, 
        DMA_DBM_DISABLE, 
        DMA_ADDR_INCREMENT,   // Increment the buffer pointer to fill the buffer 
        DMA_ADDR_FIXED,       // No peripheral increment - copy from DR only 
        DMA_DATA_SIZE_BYTE, 
        DMA_DATA_SIZE_BYTE); 

    // DMA1 stream config - UART2 - Serial terminal 
    dma_stream_config(
        user_data.dma_stream, 
        (uint32_t)(&user_data.uart->DR), 
        (uint32_t)user_data.cb, 
        (uint32_t)NULL, 
        (uint16_t)SIK_TEST_MSG_BUFF_SIZE); 

    // Enable DMA streams 
    dma_stream_enable(radio_data.dma_stream);   // UART1 - Sik radio 
    dma_stream_enable(user_data.dma_stream);    // UART2 - Serial terminal 

    //==================================================
    
    //==================================================
    // Initialize interrupts 

    // Initialize interrupt handler flags 
    int_handler_init(); 

    // Enable the interrupt handlers 
    nvic_config(USART1_IRQn, EXTI_PRIORITY_0);          // UART1 - SiK radio 
    nvic_config(USART2_IRQn, EXTI_PRIORITY_1);          // UART2 - Serial terminal (user input) 
    nvic_config(TIM1_BRK_TIM9_IRQn, EXTI_PRIORITY_2);   // TIM9 - periodic timer 

    //==================================================

    //==================================================
    // Device driver setup 

    sik_init(radio_data.uart); 

    //==================================================
}

//=======================================================================================


//=======================================================================================
// Test code 

void sik_radio_test_app(void)
{
    // New SiK radio module data received 
    if (handler_flags.usart1_flag)
    {
        handler_flags.usart1_flag = CLEAR_BIT; 
        radio_data.data_in_index = CLEAR; 

        // Parse the new radio data from the circular buffer into the data buffer. 
        dma_cb_index(radio_data.dma_stream, &radio_data.dma_index, &radio_data.cb_index); 
        cb_parse(radio_data.cb, &radio_data.cb_index, radio_data.data_in_buff); 

        // Choose an action based on whether the radio is in AT command mode or not. 
        if (system_data.at_mode)
        {
            sik_radio_test_at_radio_decode(); 
        }
        else 
        {
            sik_radio_test_mavlink_radio_decode(); 
        }
    }

    // New serial terminal (user input) data received 
    if (handler_flags.usart2_flag)
    {
        handler_flags.usart2_flag = CLEAR_BIT; 

        // Parse the new user message from the circular buffer into the data buffer 
        dma_cb_index(user_data.dma_stream, &user_data.dma_index, &user_data.cb_index); 
        cb_parse(user_data.cb, &user_data.cb_index, user_data.data_in_buff); 

        // Choose an action based on whether the radio is in AT command mode or not. 
        if (system_data.at_mode)
        {
            sik_radio_test_at_user_decode(); 
        }
        else 
        {
            sik_radio_test_mavlink_user_decode(); 
        }
    }
    
    // Periodic interrupt 
    if (handler_flags.tim1_brk_tim9_glbl_flag)
    {
        handler_flags.tim1_brk_tim9_glbl_flag = CLEAR_BIT; 

        // Only perform MAVLink actions when the SiK radio is not in AT command mode. 
        if (!system_data.at_mode)
        {
            sik_radio_test_mavlink_periodic(); 
        }
    }
}

//=======================================================================================


//=======================================================================================
// State functions 

// AT command mode radio input decode 
void sik_radio_test_at_radio_decode(void)
{
    // Echo the AT response from the radio to the serial terminal for the user to see. A 
    // prompt will be output after the user enters anything so it needs to be overwritten 
    // to make way for the radio response. 
    sik_radio_test_user_output(sik_test_overwrite); 
    sik_radio_test_user_output((char *)radio_data.data_in_buff); 
    sik_radio_test_user_output(sik_test_user_prompt); 
}


// MAVLink mode radio input decode 
void sik_radio_test_mavlink_radio_decode(void)
{
    // Besides a MAVLink message, the only other data received from the radio in MAVLink 
    // mode is an AT command mode enter response (assuming the user doesn't configure the 
    // radio to not use MAVLink). This is only checked for right after the user requests 
    // to go to AT command mode. 
    if (system_data.at_mode_requested)
    {
        if (str_compare(sik_at_enter_resp, (char *)user_data.data_in_buff, BYTE_0))
        {
            system_data.at_mode = SET_BIT; 
            sik_radio_test_at_request_reset(sik_test_at_confirm); 
        }
    }

    // Look at each byte of the received data and try to decode MAVLink messages 
    // until there is no more data to check. 
    while (radio_data.data_in_buff[radio_data.data_in_index] != NULL_CHAR)
    {
        if (mavlink_parse_char(
                system_data.channel, 
                radio_data.data_in_buff[radio_data.data_in_index++], 
                &system_data.msg, 
                &system_data.status))
        {
            // MAVLink message has been successfully decoded. Proceed to decode the 
            // message payload. Note that this can happen more than once if multiple 
            // messages are received or not at all if a valid message can't be decoded. 

            if (!system_data.ui_mode)
            {
                // If not in user input mode then display the message ID for the user to 
                // see. This is done here so all received messages are shown to the user 
                // even if the message payload is not meant to be decoded. 
                snprintf((char *)user_data.data_out_buff, 
                         SIK_TEST_MSG_BUFF_SIZE, 
                         sik_test_user_msgid, 
                         (uint32_t)system_data.msg.msgid); 
                sik_radio_test_user_output((char *)user_data.data_out_buff); 
            }

            sik_radio_test_mavlink_payload_decode(); 
        }
    }
}


// MAVLink message payload decode 
void sik_radio_test_mavlink_payload_decode(void)
{
    switch (system_data.msg.msgid)
    {
        case MAVLINK_MSG_ID_HEARTBEAT: 
            mavlink_msg_heartbeat_decode(
                &system_data.msg, 
                &system_data.heartbeat); 
            system_data.heartbeat_timer = CLEAR; 
            system_data.connected = SET_BIT; 
            break; 

        case MAVLINK_MSG_ID_GLOBAL_POSITION_INT: 
            break; 

        case MAVLINK_MSG_ID_GPS_STATUS: 
            break; 
        
        case MAVLINK_MSG_ID_COMMAND_INT: 
            // Call a separate function to decode mavlink_command_int_t.command 
            break; 
        
        case MAVLINK_MSG_ID_COMMAND_LONG: 
            // Call a separate function to decode mavlink_command_long_t.command 
            break; 
        
        default: 
            break; 
    }
}


// AT command mode user input decode 
void sik_radio_test_at_user_decode(void)
{
    // Take the user input while in AT command mode and try to match it to one of the 
    // predefined commands available in this test. If no command is matched then the 
    // input is sent as is to the radio and a new prompt will be displayed. Any response 
    // from the radio will be echoed to the serial terminal for the user to see. 
    if (str_compare(sik_ato_cmd, (char *)user_data.data_in_buff, BYTE_0))
    {
        // Exit AT command mode 
        sik_at_mode(SIK_AT_EXIT); 
        system_data.at_mode = CLEAR_BIT; 
    }
    else 
    {
        // Send input as is to the radio 
        sik_send_data((char *)user_data.data_in_buff); 
    }

    sik_radio_test_user_output(sik_test_user_prompt); 
}


// MAVLink mode user input decode 
void sik_radio_test_mavlink_user_decode(void)
{
    // When AT command mode has been requested by the user then block and other inputs 
    // from being registered until either AT command mode has been entered or the 
    // request has timed out (either way the AT command request flag is cleared). 
    if (system_data.at_mode_requested)
    {
        return; 
    }

    // When not waiting on AT command mode response and in MAVLink mode then look to 
    // match the user input to a predefined response. If no match is found then a 
    // new prompt will be displayed without any action or acknowledgement. 
    if (!system_data.ui_mode)
    {
        // If this is true then it means the code is in MAVLink mode (i.e. not AT 
        // command mode), all incoming messages are being displayed for the user to see 
        // and the user has provided any input via the serial terminal. This case will 
        // make it so messages will stop being displayed giving the user an opportunity 
        // to provide uninterrupted input (i.e. user input mode). 
        system_data.ui_mode = SET_BIT; 
        sik_radio_test_user_output(sik_test_user_prompt); 
    }
    else if (str_compare(sik_test_user_exitui, (char *)user_data.data_in_buff, BYTE_0))
    {
        // Exit user input mode 
        system_data.ui_mode = CLEAR_BIT; 
    }
    else if (str_compare(sik_at_enter_cmd, (char *)user_data.data_in_buff, BYTE_0))
    {
        // Submit an AT command mode request 
        sik_at_mode(SIK_AT_ENTER); 
        system_data.at_mode_requested = SET_BIT; 
        sik_radio_test_user_output(sik_test_at_request); 
    }
    else 
    {
        // No matching action found. Do nothing. 
        sik_radio_test_user_output(sik_test_user_prompt); 
    }
}


// MAVlink mode periodic actions 
void sik_radio_test_mavlink_periodic(void)
{
    // Everything here is performed periodically when in MAVLink mode. 

    // Send heartbeat for GCS to see 
    mavlink_msg_heartbeat_encode(
        system_data.system_id, 
        system_data.component_id, 
        &system_data.msg, 
        &system_data.heartbeat); 
    mavlink_msg_to_send_buffer(radio_data.data_out_buff, &system_data.msg); 
    sik_send_data((char *)radio_data.data_out_buff); 

    // Check for a connection (heartbeat) timeout 
    if (system_data.heartbeat_timer++ >= SIK_TEST_HB_TIMEOUT)
    {
        // Have not seen a heartbeat message from a GCS for too long. The system is 
        // considered to be disconnected. 
        system_data.heartbeat_timer--; 
        system_data.connected = CLEAR_BIT; 
    }

    // Check for an AT command mode request timeout 
    if (system_data.at_mode_requested)
    {
        // If AT command mode is being requested then the code waits on the appropriate 
        // response from the radio. Waiting for this response blocks other actions so 
        // once the system has waited too long then cancel the search and return to 
        // normal MAVLink mode. 
        if (system_data.at_mode_timer++ >= SIK_TEST_AT_TIMEOUT)
        {
            sik_radio_test_at_request_reset(sik_test_timeout_msg); 
        }
    }
}

//=======================================================================================


//=======================================================================================
// Helper functions 

// User prompt 
void sik_radio_test_user_output(const char *user_msg)
{
    uart_send_str(user_data.uart, user_msg); 
}


// AT command mode request reset 
void sik_radio_test_at_request_reset(const char *user_msg)
{
    system_data.at_mode_timer = CLEAR; 
    system_data.at_mode_requested = CLEAR_BIT; 
    sik_radio_test_user_output(user_msg); 
    sik_radio_test_user_output(sik_test_user_prompt); 
}

//=======================================================================================
