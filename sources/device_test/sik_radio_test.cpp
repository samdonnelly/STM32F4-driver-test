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
 *            fixed intervals. If not using AT command mode, to get the test started 
 *            the user should make sure both ends of the telemtry link are ready, 
 *            then go to Mission Planner and press "Connect". Mission Planner should 
 *            retrieve dummy data and a heartbeat from this system and that data 
 *            should be visible in Mission Planner once retrieved. 
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
 *          - NOTE: While in MAVLink mode, the codes primary purpose is to communicate 
 *            with Mission Planner (MP) which also uses a telemetry radio. This test is 
 *            set up to do the bare minimum to communicate with MP. More info can be 
 *            found online for how to use MP but essentially when the "Connect" button is 
 *            pressed in MP (assuming radios are already connected to their respective 
 *            computers), MP will first look for a heartbeat message. Once the heartbeat 
 *            is seen, MP will proceed to send messages requesting various information 
 *            from this system. This code is set up to respond to these messages with 
 *            dummy data. Note that this test does the bare minimum to provide MP with 
 *            the info it asks for out of the gate but no other functionality of MP is 
 *            supported. MP offers a lot of features, too many to be covered here. 
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

// Buffer sizes 
#define SIK_TEST_MSG_BUFF_SIZE 1000 
#define SIK_TEST_MISSION_MAX_LEN 5 
#define SIK_TEST_NUM_PARAMS 5 

// System IDs - GCS IDs start at 255, vehicles start at 1 
#define SIK_TEST_SYS_ID 1 
#define SIK_TEST_GCS_ID 255 

// Periodic timing data 
// - ARR = Auto Reload Register - number of counts before periodic timer resets. 
// - SIK_TEST_PRESCALAR must coordinate with the timer_us_prescalars_t type used in the 
//   periodic timer init. 
// - SIK_TEST_INT_PERIOD is the period in ms of the periodic interrupt. This is used 
//   to find the timer count limit when the GCS requests a certain message rate. 
#define SIK_TEST_ARR 2500 
#define SIK_TEST_PRESCALAR 10000 
#define SIK_TEST_INT_PERIOD SIK_TEST_ARR * S_TO_MS / SIK_TEST_PRESCALAR 

#define SIK_TEST_HB_FREQ 1       // Heartbeat message send frequency 
#define SIK_TEST_PARAM_FREQ 4    // Parameter value message send frequency 
#define SIK_TEST_HB_TIMEOUT 40   // Heartbeat message timeout - adjust with interrupt period 
#define SIK_TEST_AT_TIMEOUT 20   // AT mode timeout - adjust with interrupt period 

// Mock data - numbers selected simply to make the data make sense 
#define SIK_TEST_MOCK_BOOT_TIME 100 
#define SIK_TEST_MOCK_LAT 506132700 
#define SIK_TEST_MOCK_LON -1151237700 
#define SIK_TEST_MOCK_LAT_HOME 506132550 
#define SIK_TEST_MOCK_LON_HOME -1151204230 
#define SIK_TEST_MOCK_ALTITUDE 1 
#define SIK_TEST_MOCK_NUM_SATELLITES 4 
#define SIK_TEST_MOCK_WP_RADIUS 10 
#define SIK_TEST_MOCK_IMU_DIR 1 
#define SIK_TEST_MOCK_WP_DISTANCE 200 

//=======================================================================================


//=======================================================================================
// Global data 

// User & device serial interface data 
struct sik_serial_data_t 
{
    USART_TypeDef *uart; 
    DMA_Stream_TypeDef *dma_stream; 
    uint8_t cb[SIK_TEST_MSG_BUFF_SIZE];              // Circular buffer populated by DMA 
    cb_index_t cb_index;                             // Circular buffer indexing info 
    dma_index_t dma_index;                           // DMA transfer indexing info 
    uint8_t data_in_buff[SIK_TEST_MSG_BUFF_SIZE];    // Buffer that stores latest UART input 
    uint8_t data_out_buff[SIK_TEST_MSG_BUFF_SIZE];   // Buffer that stores outgoing data 
    uint16_t data_in_index;                          // Data input buffer index 
}; 

static sik_serial_data_t radio_data; 
static sik_serial_data_t user_data; 


// MAVLink message send timing info 
struct sik_msg_timing_t
{
    uint8_t count; 
    uint8_t count_lim; 
    uint8_t enable : 1; 
}; 


// MAVLink messages 
struct sik_mavlink_msgs_t 
{
    // Incoming messages 
    mavlink_heartbeat_t heartbeat_msg_gcs;                       // HEARTBEAT 
    mavlink_mission_request_t mission_request_msg_gcs;           // MISSION_REQUEST 
    mavlink_param_request_list_t param_request_list_msg_gcs;     // PARAM_REQUEST_LIST 
    mavlink_command_long_t command_long_msg_gcs;                 // COMMAND_LONG 
    mavlink_request_data_stream_t request_data_stream_msg_gcs;   // REQUEST_DATA_STREAM 
    
    // Outgoing (stored) messages - other messages get 
    // sent but their data is not stored. 
    mavlink_heartbeat_t heartbeat_msg;                           // HEARTBEAT 
    
    // Periodic outgoing message timing info 
    sik_msg_timing_t heartbeat_msg_timing;                       // HEARTBEAT 
    sik_msg_timing_t raw_imu_msg_timing;                         // RAW_IMU 
    sik_msg_timing_t gps_raw_int_msg_timing;                     // GPS_RAW_INT 
    sik_msg_timing_t rc_channels_scaled_msg_timing;              // RC_CHANNELS_SCALED 
    sik_msg_timing_t rc_channels_raw_msg_timing;                 // RC_CHANNELS_RAW 
    sik_msg_timing_t servo_output_raw_msg_timing;                // SERVO_OUTPUT_RAW 
    sik_msg_timing_t attitude_msg_timing;                        // ATTITUDE 
    sik_msg_timing_t position_target_global_int_msg_timing;      // POSITION_TARGET_GLOBAL_INT 
    sik_msg_timing_t nav_controller_output_msg_timing;           // NAV_CONTROLLER_OUTPUT 
    sik_msg_timing_t local_position_ned_msg_timing;              // LOCAL_POSITION_NED 
    sik_msg_timing_t global_pos_int_msg_timing;                  // GLOBAL_POSITION_INT 
    sik_msg_timing_t param_value_msg_timing;                     // PARAM_VALUE 
}; 


// System data 
class SikSystemData : public sik_mavlink_msgs_t 
{
public: 
    // MAVLink identification 
    int channel; 
    uint8_t system_id; 
    uint8_t component_id; 
    
    // MAVLink packet handling 
    mavlink_message_t msg; 
    mavlink_status_t status; 

    // Mission data 
    mavlink_mission_item_int_t mission[SIK_TEST_MISSION_MAX_LEN]; 
    uint16_t mission_size; 

    // Parameters 
    struct ParamInfo 
    {
        const char *name; 
        float value; 
        // Mission planner seems to only accept floats so 
        // no option for MAV_PARAM_TYPE is offered here. 
    };
    uint8_t param_index; 

    // Status timers 
    uint8_t heartbeat_status_timer; 
    uint8_t at_mode_request_timer; 

    // Status flags 
    uint8_t at_mode           : 1;   // AT command mode 
    uint8_t at_mode_requested : 1;   // AT command mode requested 
    uint8_t ui_mode           : 1;   // User input mode 
    uint8_t connected         : 1;   // Radio connected 
}; 

static SikSystemData system_data; 


// Strings for formatting the serial terminal for the user. 
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


// Parameters for this system. Parameters can be nearly any name and value and there are 
// no pre-defined parameters in MAVLink. The below parameters were copied from ArduPilot 
// and selected because they're relatively easy to understand at quick glance, not 
// because they have anything to do with this system. 
// See https://ardupilot.org/rover/docs/parameters.html#parameters for more details. 
static const SikSystemData::ParamInfo system_params[SIK_TEST_NUM_PARAMS] = 
{
    {"CRUISE_SPEED", 1.1},   // Target cruise speed in auto mode (m/s) 
    {"FRAME_CLASS",  2},     // Frame class: Boat 
    {"TURN_RADIUS",  1.0},   // Turn radius of vehicle (meters) 
    {"LOIT_TYPE",    0},     // Loiter type: Forward or reverse to target point 
    {"LOIT_RADIUS",  5.0}    // Loiter radius (meters) 
};

//=======================================================================================


//=======================================================================================
// Prototypes 

/**
 * @brief Data initialization - init system data to make code more readable 
 */
void sik_radio_test_init_data(void); 

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
 * @details Used to decode user input when in MAVLink mode. A match to specific 
 *          predefined commands is checked for. If no match is found then nothing happens.
 *          When the system is in MAVLink mode and not in user input mode then any input 
 *          will trigger user input mode and nothing else. 
 */
void sik_radio_test_mavlink_user_decode(void); 


/**
 * @brief MAVLink message payload decode 
 * 
 * @details Decodes the payload of a received MAVLink message once the received radio 
 *          data is decoded into a complete MAVLink message. A function like this is 
 *          recommended by the MAVLink documentation for identifying and handling 
 *          messages. Different messages can be added as needed. 
 */
void sik_radio_test_mavlink_payload_decode(void); 

// MAVLink received message actions 
void sik_radio_test_mavlink_request_data_stream(void); 
void sik_radio_test_mavlink_param_request_list(void); 
void sik_radio_test_mavlink_mission_request(void); 
void sik_radio_test_mavlink_heartbeat(void); 
void sik_radio_test_mavlink_command_long(void); 


/**
 * @brief MAVlink mode periodic actions 
 * 
 * @details This function performs periodic actions when in MAVLink mode. 
 */
void sik_radio_test_mavlink_periodic(void); 

// MAVLink periodic actions 
void sik_radio_test_mavlink_periodic_timeout(void); 
void sik_radio_test_mavlink_periodic_send(void); 


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


/**
 * @brief Serialize and send MAVLink message 
 * 
 * @details Once a message payload is packed/encoded, this can be called to insert the 
 *          message into a buffer and sent it to the radio. 
 */
void sik_radio_test_mavlink_send_msg(void); 

//=======================================================================================


//=======================================================================================
// Setup code 

void sik_radio_test_init(void)
{
    // Initialize data 
    sik_radio_test_init_data(); 

    //==================================================
    // General setup 

    // Initialize GPIO ports 
    gpio_port_init(); 

    // Periodic (counter update) interrupt timer 
    tim_9_to_11_counter_init(
        TIM9, 
        TIM_84MHZ_100US_PSC, 
        SIK_TEST_ARR,   // ARR * (100us/count) = 0.25s period 
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
        UART_FRAC_84_57600, 
        UART_MANT_84_57600, 
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
        DMA_PRIOR_HI, 
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


// Data initialization 
void sik_radio_test_init_data(void)
{
    //==================================================
    // Serial interface data 

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
    
    //==================================================

    //==================================================
    // System data 
    
    system_data.channel = MAVLINK_COMM_0; 
    system_data.system_id = SIK_TEST_SYS_ID; 
    system_data.component_id = MAV_COMP_ID_AUTOPILOT1; 

    // Default home location 
    system_data.mission[BYTE_0] = 
    {
        .param1 = ZERO, 
        .param2 = SIK_TEST_MOCK_WP_RADIUS, 
        .param3 = SIK_TEST_MOCK_WP_RADIUS, 
        .param4 = ZERO, 
        .x = SIK_TEST_MOCK_LAT_HOME, 
        .y = SIK_TEST_MOCK_LON_HOME, 
        .z = SIK_TEST_MOCK_ALTITUDE, 
        .seq = ZERO, 
        .command = MAV_CMD_NAV_WAYPOINT, 
        .target_system = SIK_TEST_GCS_ID, 
        .target_component = MAV_COMP_ID_MISSIONPLANNER, 
        .frame = MAV_FRAME_GLOBAL, 
        .current = ZERO, 
        .autocontinue = ZERO, 
        .mission_type = MAV_MISSION_TYPE_MISSION 
    }; 
    system_data.mission_size = BYTE_1; 

    system_data.param_index = CLEAR; 
    
    system_data.heartbeat_status_timer = CLEAR; 
    system_data.at_mode_request_timer = CLEAR; 
    
    system_data.at_mode = CLEAR_BIT; 
    system_data.at_mode_requested = CLEAR_BIT; 
    system_data.ui_mode = CLEAR_BIT; 
    system_data.connected = CLEAR_BIT; 
    
    //==================================================
    
    //==================================================
    // MAVLink messages 

    // Only the heartbeat message is stored and defined here as this defines the system. All 
    // other data that gets sent contains dummy data and is inserted directly into messages 
    // before sending. 

    // HEARTBEAT 
    system_data.heartbeat_msg = 
    {
        .custom_mode = CLEAR, 
        .type = MAV_TYPE_SURFACE_BOAT, 
        .autopilot = MAV_AUTOPILOT_GENERIC_WAYPOINTS_ONLY, 
        .base_mode = MAV_MODE_FLAG_CUSTOM_MODE_ENABLED | 
                     MAV_MODE_FLAG_MANUAL_INPUT_ENABLED | 
                     MAV_MODE_FLAG_GUIDED_ENABLED | 
                     MAV_MODE_FLAG_SAFETY_ARMED, 
        .system_status = MAV_STATE_ACTIVE, 
        .mavlink_version = ZERO   // Gets overwritten by MAVLink 
    }; 

    // Message timing info 
    system_data.heartbeat_msg_timing = { CLEAR, 
                                         S_TO_MS / (SIK_TEST_HB_FREQ * SIK_TEST_INT_PERIOD), 
                                         SET_BIT }; 
    system_data.raw_imu_msg_timing                    = { CLEAR, CLEAR, CLEAR_BIT }; 
    system_data.gps_raw_int_msg_timing                = { CLEAR, CLEAR, CLEAR_BIT }; 
    system_data.rc_channels_scaled_msg_timing         = { CLEAR, CLEAR, CLEAR_BIT }; 
    system_data.rc_channels_raw_msg_timing            = { CLEAR, CLEAR, CLEAR_BIT }; 
    system_data.servo_output_raw_msg_timing           = { CLEAR, CLEAR, CLEAR_BIT }; 
    system_data.attitude_msg_timing                   = { CLEAR, CLEAR, CLEAR_BIT }; 
    system_data.position_target_global_int_msg_timing = { CLEAR, CLEAR, CLEAR_BIT }; 
    system_data.nav_controller_output_msg_timing      = { CLEAR, CLEAR, CLEAR_BIT }; 
    system_data.local_position_ned_msg_timing         = { CLEAR, CLEAR, CLEAR_BIT }; 
    system_data.global_pos_int_msg_timing             = { CLEAR, CLEAR, CLEAR_BIT }; 
    system_data.param_value_msg_timing = { CLEAR, 
                                           S_TO_MS / (SIK_TEST_PARAM_FREQ * SIK_TEST_INT_PERIOD), 
                                           CLEAR_BIT }; 
    
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
// Serial input decoding and actions 

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
    radio_data.data_in_index = CLEAR; 

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
    while (radio_data.data_in_index < radio_data.dma_index.data_size)
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
        sik_send_data(user_data.data_in_buff, strlen((char *)user_data.data_in_buff)); 
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

//=======================================================================================


//=======================================================================================
// MAVLink payload decoding and actions 

// MAVLink message payload decode 
void sik_radio_test_mavlink_payload_decode(void)
{
    switch (system_data.msg.msgid)
    {
        case MAVLINK_MSG_ID_HEARTBEAT: 
            sik_radio_test_mavlink_heartbeat(); 
            break; 

        case MAVLINK_MSG_ID_PARAM_REQUEST_LIST: 
            sik_radio_test_mavlink_param_request_list(); 
            break; 

        case MAVLINK_MSG_ID_MISSION_REQUEST: 
            sik_radio_test_mavlink_mission_request(); 
            break; 

        case MAVLINK_MSG_ID_REQUEST_DATA_STREAM: 
            sik_radio_test_mavlink_request_data_stream(); 
            break; 
        
        case MAVLINK_MSG_ID_COMMAND_LONG: 
            sik_radio_test_mavlink_command_long(); 
            break; 
        
        default: 
            break; 
    }
}


// MAVLink HEARTBEAT message actions 
void sik_radio_test_mavlink_heartbeat(void)
{
    mavlink_msg_heartbeat_decode(
        &system_data.msg, 
        &system_data.heartbeat_msg_gcs); 

    // This system is only concerned with heartbeats from the GCS it's communicating with. 
    // The system considers itself connected only if the heatbeat message type and source 
    // are correct. 
    if ((system_data.heartbeat_msg_gcs.type == MAV_TYPE_GCS) && 
        (system_data.heartbeat_msg_gcs.autopilot == MAV_AUTOPILOT_INVALID) && 
        (system_data.msg.sysid == SIK_TEST_GCS_ID) && 
        (system_data.msg.compid == MAV_COMP_ID_MISSIONPLANNER))
    {
        system_data.heartbeat_status_timer = CLEAR; 
        system_data.connected = SET_BIT; 
    }
}


// MAVLink PARAM_REQUEST_LIST message actions 
void sik_radio_test_mavlink_param_request_list(void)
{
    mavlink_msg_param_request_list_decode(
        &system_data.msg, 
        &system_data.param_request_list_msg_gcs); 

    // This system is only concerned with messages meant for this system. If the taget 
    // system and component ID in the message does not match this system then abort. 
    if ((system_data.param_request_list_msg_gcs.target_system != system_data.system_id) || 
        (system_data.param_request_list_msg_gcs.target_component != system_data.component_id))
    {
        return; 
    }

    // Enable the PARAM_VALUE message which gets sent periodically until all parameters 
    // in the system have been sent. 
    system_data.param_value_msg_timing.enable = SET_BIT; 
    system_data.param_index = CLEAR; 
}


// MAVLink MISSION_REQUEST message actions 
void sik_radio_test_mavlink_mission_request(void)
{
    // Mission planner sends MISSION_REQUEST messages despite the message being 
    // deprecated by MAVLink in favour of MISSION_REQUEST_INT. When this message is 
    // received, Mission Planner expects MISSION_ITEM_INT in return as discovered through 
    // trial and error (i.e. MISSION_ITEM does not work). 
    // MISSION_ITEM_INT takes the system and component IDs in its payload of the system 
    // the message is being sent to. This should not be confused with the full MAVLink 
    // message system and component IDs which identify where a message is coming from. 
    // Mission Planner varries from standard MAVLink mission protocol in that the item 
    // at mission sequence 0 is the home location, not the first waypoint location. 

    mavlink_msg_mission_request_decode(
        &system_data.msg, 
        &system_data.mission_request_msg_gcs); 

    // This system is only concerned with messages meant for this system. If the taget 
    // system and component ID in the message does not match this system then abort. 
    if ((system_data.mission_request_msg_gcs.target_system != system_data.system_id) || 
        (system_data.mission_request_msg_gcs.target_component != system_data.component_id))
    {
        return; 
    }

    // Only send the mission item if it exists 
    if (system_data.mission_request_msg_gcs.seq < system_data.mission_size)
    {
        mavlink_msg_mission_item_int_encode_chan(
            system_data.system_id, 
            system_data.component_id, 
            system_data.channel, 
            &system_data.msg, 
            &system_data.mission[system_data.mission_request_msg_gcs.seq]); 
        sik_radio_test_mavlink_send_msg(); 
    }
}


// MAVLink REQUEST_DATA_STREAM message actions 
void sik_radio_test_mavlink_request_data_stream(void)
{
    // Mission Planner sends this message to request data from the autopilot. This 
    // message is often sent in bursts to request all the needed messages. 

    mavlink_msg_request_data_stream_decode(
        &system_data.msg, 
        &system_data.request_data_stream_msg_gcs); 
    
    // This system is only concerned with messages meant for this system. If the taget 
    // system and component ID in the message does not match this system then abort. 
    if ((system_data.request_data_stream_msg_gcs.target_system != system_data.system_id) || 
        (system_data.request_data_stream_msg_gcs.target_component != system_data.component_id))
    {
        return; 
    }

    // This message comes with a cooresponding requested message rate. The calculated 
    // timer counter limit is the same calculation for each requested message so it's 
    // done once here and assigned to the requested message. Note that the periodic 
    // interrupt period should be equipped to handle whatever the requested rate is. 
    uint8_t timer_limit = (uint8_t)(S_TO_MS / 
        (system_data.request_data_stream_msg_gcs.req_message_rate * SIK_TEST_INT_PERIOD)); 
    uint8_t enable = system_data.request_data_stream_msg_gcs.start_stop; 

    // Enable/disable the requested message and assign the message timer counter limit 
    // so it gets sent to the GCS at the requested rate. 
    switch (system_data.request_data_stream_msg_gcs.req_stream_id)
    {
        case MAV_DATA_STREAM_ALL: 
            break; 
        
        case MAV_DATA_STREAM_RAW_SENSORS: 
            system_data.raw_imu_msg_timing.enable = enable; 
            system_data.raw_imu_msg_timing.count_lim = timer_limit; 
            system_data.gps_raw_int_msg_timing.enable = enable; 
            system_data.gps_raw_int_msg_timing.count_lim = timer_limit; 
            break; 

        case MAV_DATA_STREAM_EXTENDED_STATUS: 
            break; 

        case MAV_DATA_STREAM_RC_CHANNELS: 
            system_data.rc_channels_scaled_msg_timing.enable = enable; 
            system_data.rc_channels_scaled_msg_timing.count_lim = timer_limit; 
            system_data.rc_channels_raw_msg_timing.enable = enable; 
            system_data.rc_channels_raw_msg_timing.count_lim = timer_limit; 
            system_data.servo_output_raw_msg_timing.enable = enable; 
            system_data.servo_output_raw_msg_timing.count_lim = timer_limit; 
            break; 

        case MAV_DATA_STREAM_RAW_CONTROLLER: 
            system_data.attitude_msg_timing.enable = enable; 
            system_data.attitude_msg_timing.count_lim = timer_limit; 
            system_data.position_target_global_int_msg_timing.enable = enable; 
            system_data.position_target_global_int_msg_timing.count_lim = timer_limit; 
            system_data.nav_controller_output_msg_timing.enable = enable; 
            system_data.nav_controller_output_msg_timing.count_lim = timer_limit; 
            break; 

        case MAV_DATA_STREAM_POSITION: 
            system_data.local_position_ned_msg_timing.enable = enable; 
            system_data.local_position_ned_msg_timing.count_lim = timer_limit; 
            system_data.global_pos_int_msg_timing.enable = enable; 
            system_data.global_pos_int_msg_timing.count_lim = timer_limit; 
            break; 

        case MAV_DATA_STREAM_EXTRA1: 
            break; 

        case MAV_DATA_STREAM_EXTRA2: 
            break; 

        case MAV_DATA_STREAM_EXTRA3: 
            break; 

        default: 
            break; 
    }
}


// MAVLink COMMAND_LONG message actions 
void sik_radio_test_mavlink_command_long(void)
{
    mavlink_msg_command_long_decode(
        &system_data.msg, 
        &system_data.command_long_msg_gcs); 

    // This system is only concerned with messages meant for this system. If the taget 
    // system and component ID in the message does not match this system then abort. 
    if ((system_data.command_long_msg_gcs.target_system != system_data.system_id) || 
        (system_data.command_long_msg_gcs.target_component != system_data.component_id))
    {
        return; 
    }

    // Acknowledge the command 
    mavlink_msg_command_ack_pack_chan(
        system_data.system_id, 
        system_data.component_id, 
        system_data.channel, 
        &system_data.msg, 
        system_data.command_long_msg_gcs.command, 
        MAV_RESULT_ACCEPTED, 
        ZERO, 
        ZERO, 
        SIK_TEST_GCS_ID, 
        MAV_COMP_ID_MISSIONPLANNER); 
    sik_radio_test_mavlink_send_msg(); 

    // Perform the needed action based on the command 
    uint16_t cmd_id = (uint16_t)system_data.command_long_msg_gcs.param1; 

    switch (cmd_id)
    {
        case MAVLINK_MSG_ID_AUTOPILOT_VERSION: 
        {
            uint8_t version[8], hw_uid[18]; 
            memset((void *)version, CLEAR, sizeof(version)); 
            memset((void *)hw_uid, CLEAR, sizeof(hw_uid)); 

            uint32_t capabilities = MAV_PROTOCOL_CAPABILITY_MISSION_FLOAT | 
                                    MAV_PROTOCOL_CAPABILITY_MISSION_INT | 
                                    MAV_PROTOCOL_CAPABILITY_COMMAND_INT | 
                                    MAV_PROTOCOL_CAPABILITY_SET_POSITION_TARGET_LOCAL_NED | 
                                    MAV_PROTOCOL_CAPABILITY_SET_POSITION_TARGET_GLOBAL_INT | 
                                    MAV_PROTOCOL_CAPABILITY_SET_ATTITUDE_TARGET | 
                                    MAV_PROTOCOL_CAPABILITY_PARAM_FLOAT |
                                    MAV_PROTOCOL_CAPABILITY_COMPASS_CALIBRATION | 
                                    MAV_PROTOCOL_CAPABILITY_MAVLINK2; 

            mavlink_msg_autopilot_version_pack_chan(
                system_data.system_id, 
                system_data.component_id, 
                system_data.channel, 
                &system_data.msg, 
                capabilities,   // Autopilot capabilities (bitmap) 
                ZERO,           // Firmware version number 
                ZERO,           // Middleware version number 
                ZERO,           // Operating system version number 
                ZERO,           // HW/board version 
                version,        // Custom firmware version 
                version,        // Custom middleware version 
                version,        // Custom operating system version 
                ZERO,           // ID of board vendor 
                ZERO,           // ID of the product 
                ZERO,           // UID if provided by hardware 
                hw_uid);        // UID if provided by hardware 
            sik_radio_test_mavlink_send_msg(); 
            break; 
        }

        default: 
            break; 
    }
}

//=======================================================================================


//=======================================================================================
// Periodic actions 

// MAVlink mode periodic actions 
void sik_radio_test_mavlink_periodic(void)
{
    // Everything here is performed periodically when in MAVLink mode. 
    sik_radio_test_mavlink_periodic_timeout(); 
    sik_radio_test_mavlink_periodic_send(); 
}


// MAVLink periodic timeout check 
void sik_radio_test_mavlink_periodic_timeout(void)
{
    // Check for a connection (heartbeat) timeout 
    if (system_data.heartbeat_status_timer++ >= SIK_TEST_HB_TIMEOUT)
    {
        // Have not seen a heartbeat message from a GCS for too long. The system is 
        // considered to be disconnected. 
        system_data.heartbeat_status_timer--; 
        system_data.connected = CLEAR_BIT; 
    }

    // Check for an AT command mode request timeout 
    if (system_data.at_mode_requested)
    {
        // If AT command mode is being requested then the code waits on the appropriate 
        // response from the radio. Waiting for this response blocks other actions so 
        // once the system has waited too long then cancel the search and return to 
        // normal MAVLink mode. 
        if (system_data.at_mode_request_timer++ >= SIK_TEST_AT_TIMEOUT)
        {
            sik_radio_test_at_request_reset(sik_test_timeout_msg); 
            sik_at_mode(SIK_AT_EXIT); 
        }
    }
}


// MAVLink message periodic send 
void sik_radio_test_mavlink_periodic_send(void)
{
    // Check if any of the enabled periodic messages must be sent. 

    // This test does not read from other devices to populate system information so dummy 
    // data is inserted into each message before sending. There is no real reason for 
    // choosing specific numbers besides making them valid for the message. 

    // HEARTBEAT 
    if (system_data.heartbeat_msg_timing.enable && 
       (++system_data.heartbeat_msg_timing.count >= 
          system_data.heartbeat_msg_timing.count_lim))
    {
        system_data.heartbeat_msg_timing.count = CLEAR; 
        mavlink_msg_heartbeat_encode_chan(
            system_data.system_id, 
            system_data.component_id, 
            system_data.channel, 
            &system_data.msg, 
            &system_data.heartbeat_msg); 
        sik_radio_test_mavlink_send_msg(); 
    }

    // RAW_IMU 
    if (system_data.raw_imu_msg_timing.enable && 
       (++system_data.raw_imu_msg_timing.count >= 
          system_data.raw_imu_msg_timing.count_lim))
    {
        system_data.raw_imu_msg_timing.count = CLEAR; 
        mavlink_msg_raw_imu_pack_chan(
            system_data.system_id, 
            system_data.component_id, 
            system_data.channel, 
            &system_data.msg, 
            SIK_TEST_MOCK_BOOT_TIME,   // Time since boot 
            ZERO,                      // X accelerometer 
            ZERO,                      // Y accelerometer 
            SIK_TEST_MOCK_IMU_DIR,     // Z accelerometer 
            ZERO,                      // X gyroscope 
            ZERO,                      // Y gyroscope 
            ZERO,                      // Z gyroscope 
            SIK_TEST_MOCK_IMU_DIR,     // X magnetometer 
            ZERO,                      // Y magnetometer 
            ZERO,                      // Z magnetometer 
            ZERO,                      // IMU ID 
            ZERO);                     // Temperature 
        sik_radio_test_mavlink_send_msg(); 
    }

    // GPS_RAW_INT 
    if (system_data.gps_raw_int_msg_timing.enable && 
       (++system_data.gps_raw_int_msg_timing.count >= 
          system_data.gps_raw_int_msg_timing.count_lim))
    {
        system_data.gps_raw_int_msg_timing.count = CLEAR; 
        mavlink_msg_gps_raw_int_pack_chan(
            system_data.system_id, 
            system_data.component_id, 
            system_data.channel, 
            &system_data.msg, 
            SIK_TEST_MOCK_BOOT_TIME,        // Timestamp 
            GPS_FIX_TYPE_3D_FIX,            // GPS fix type 
            SIK_TEST_MOCK_LAT,              // Latitude 
            SIK_TEST_MOCK_LON,              // Longitude 
            SIK_TEST_MOCK_ALTITUDE,         // Altitude 
            HIGH_16BIT,                     // GPS HDOP horizontal dilution of position 
            HIGH_16BIT,                     // GPS VDOP vertical dilution of position 
            HIGH_16BIT,                     // GPS ground speed 
            HIGH_16BIT,                     // Course over ground 
            SIK_TEST_MOCK_NUM_SATELLITES,   // Number of satellites visible 
            SIK_TEST_MOCK_ALTITUDE,         // Altitude 
            ZERO,                           // Position uncertainty 
            ZERO,                           // Altitude uncertainty 
            ZERO,                           // Speed uncertainty 
            ZERO,                           // Heading / track uncertainty 
            ZERO);                          // Yaw in earth frame from north 
        sik_radio_test_mavlink_send_msg(); 
    }

    // RC_CHANNELS_SCALED 
    if (system_data.rc_channels_scaled_msg_timing.enable && 
       (++system_data.rc_channels_scaled_msg_timing.count >= 
          system_data.rc_channels_scaled_msg_timing.count_lim))
    {
        system_data.rc_channels_scaled_msg_timing.count = CLEAR; 
        mavlink_msg_rc_channels_scaled_pack_chan(
            system_data.system_id, 
            system_data.component_id, 
            system_data.channel, 
            &system_data.msg, 
            SIK_TEST_MOCK_BOOT_TIME,                          // Time since boot 
            ZERO,                                             // Servo output port 
            ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO,   // RC channels 1-8 
            HIGH_8BIT);                                       // Receive signal strength 
        sik_radio_test_mavlink_send_msg(); 
    }

    // RC_CHANNELS_RAW 
    if (system_data.rc_channels_raw_msg_timing.enable && 
       (++system_data.rc_channels_raw_msg_timing.count >= 
          system_data.rc_channels_raw_msg_timing.count_lim))
    {
        system_data.rc_channels_raw_msg_timing.count = CLEAR; 
        mavlink_msg_rc_channels_raw_pack_chan(
            system_data.system_id, 
            system_data.component_id, 
            system_data.channel, 
            &system_data.msg, 
            SIK_TEST_MOCK_BOOT_TIME,                          // Time since boot 
            ZERO,                                             // Servo output port 
            ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO,   // RC channels 1-8 
            HIGH_8BIT);                                       // Receive signal strength 
        sik_radio_test_mavlink_send_msg(); 
    }

    // SERVO_OUTPUT_RAW 
    if (system_data.servo_output_raw_msg_timing.enable && 
       (++system_data.servo_output_raw_msg_timing.count >= 
          system_data.servo_output_raw_msg_timing.count_lim))
    {
        system_data.servo_output_raw_msg_timing.count = CLEAR; 
        mavlink_msg_servo_output_raw_pack_chan(
            system_data.system_id, 
            system_data.component_id, 
            system_data.channel, 
            &system_data.msg, 
            SIK_TEST_MOCK_BOOT_TIME,                           // Time since boot 
            ZERO,                                              // Servo output port 
            ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO,    // Servo output 1-8 
            ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO);   // Servo output 9-16 
        sik_radio_test_mavlink_send_msg(); 
    }

    // ATTITUDE 
    if (system_data.attitude_msg_timing.enable && 
       (++system_data.attitude_msg_timing.count >= 
          system_data.attitude_msg_timing.count_lim))
    {
        system_data.attitude_msg_timing.count = CLEAR; 
        mavlink_msg_attitude_pack_chan(
            system_data.system_id, 
            system_data.component_id, 
            system_data.channel, 
            &system_data.msg, 
            SIK_TEST_MOCK_BOOT_TIME,   // Time since boot 
            ZERO,                      // Roll angle (rad) 
            ZERO,                      // Pitch angle (rad) 
            ZERO,                      // Yaw angle (rad) 
            ZERO,                      // Roll angular speed (rad/s) 
            ZERO,                      // Pitch angular speed (rad/s) 
            ZERO);                     // Yaw angular speed (rad/s) 
        sik_radio_test_mavlink_send_msg(); 
    }

    // POSITION_TARGET_GLOBAL_INT 
    if (system_data.position_target_global_int_msg_timing.enable && 
       (++system_data.position_target_global_int_msg_timing.count >= 
          system_data.position_target_global_int_msg_timing.count_lim))
    {
        system_data.position_target_global_int_msg_timing.count = CLEAR; 
        uint16_t type_mask = POSITION_TARGET_TYPEMASK_VX_IGNORE | 
                             POSITION_TARGET_TYPEMASK_VY_IGNORE | 
                             POSITION_TARGET_TYPEMASK_VZ_IGNORE | 
                             POSITION_TARGET_TYPEMASK_AX_IGNORE | 
                             POSITION_TARGET_TYPEMASK_AY_IGNORE | 
                             POSITION_TARGET_TYPEMASK_AZ_IGNORE | 
                             POSITION_TARGET_TYPEMASK_YAW_IGNORE | 
                             POSITION_TARGET_TYPEMASK_YAW_RATE_IGNORE | 
                             0xF000; 
        mavlink_msg_position_target_global_int_pack_chan(
            system_data.system_id, 
            system_data.component_id, 
            system_data.channel, 
            &system_data.msg, 
            SIK_TEST_MOCK_BOOT_TIME,   // Time since boot 
            MAV_FRAME_GLOBAL,          // Coordinate frame (MAV_FRAME) 
            type_mask,                 // Ignored dimensions (POSITION_TARGET_TYPEMASK) 
            SIK_TEST_MOCK_LAT,         // Latitude 
            SIK_TEST_MOCK_LON,         // Longitude 
            SIK_TEST_MOCK_ALTITUDE,    // Altitude 
            ZERO,                      // X velocity in NED frame (m/s) 
            ZERO,                      // Y velocity in NED frame (m/s) 
            ZERO,                      // Z velocity in NED frame (m/s) 
            ZERO,                      // X acceleration or force in NED frame (N) 
            ZERO,                      // y acceleration or force in NED frame (N) 
            ZERO,                      // Z acceleration or force in NED frame (N) 
            ZERO,                      // Yaw setpoint (rad) 
            ZERO);                     // Yaw rate setpoint (rad/s) 
        sik_radio_test_mavlink_send_msg(); 
    }

    // NAV_CONTROLLER 
    if (system_data.nav_controller_output_msg_timing.enable && 
       (++system_data.nav_controller_output_msg_timing.count >= 
          system_data.nav_controller_output_msg_timing.count_lim))
    {
        system_data.nav_controller_output_msg_timing.count = CLEAR; 
        mavlink_msg_nav_controller_output_pack_chan(
            system_data.system_id, 
            system_data.component_id, 
            system_data.channel, 
            &system_data.msg, 
            ZERO,                          // Current desired roll 
            ZERO,                          // Current desired pitch 
            ZERO,                          // Current desired heading 
            ZERO,                          // Bearing to current waypoint/target 
            SIK_TEST_MOCK_WP_DISTANCE,     // Distance to active waypoint 
            ZERO,                          // Current altitude error 
            ZERO,                          // Current airspeed error 
            ZERO);                         // Current crosstrack error on x-y plane 
        sik_radio_test_mavlink_send_msg(); 
    }

    // LOCAL_POSITION_NED 
    if (system_data.local_position_ned_msg_timing.enable && 
       (++system_data.local_position_ned_msg_timing.count >= 
          system_data.local_position_ned_msg_timing.count_lim))
    {
        system_data.local_position_ned_msg_timing.count = CLEAR; 
        mavlink_msg_local_position_ned_pack_chan(
            system_data.system_id, 
            system_data.component_id, 
            system_data.channel, 
            &system_data.msg, 
            SIK_TEST_MOCK_BOOT_TIME,   // Time since boot 
            ZERO,                      // X position (m) 
            ZERO,                      // Y position (m) 
            ZERO,                      // Z position (m) 
            ZERO,                      // X speed (m/s)  
            ZERO,                      // Y speed (m/s)  
            ZERO);                     // Z speed (m/s)  
        sik_radio_test_mavlink_send_msg(); 
    }

    // GLOBAL_POSITION_INT 
    if (system_data.global_pos_int_msg_timing.enable && 
       (++system_data.global_pos_int_msg_timing.count >= 
          system_data.global_pos_int_msg_timing.count_lim))
    {
        system_data.global_pos_int_msg_timing.count = CLEAR; 
        mavlink_msg_global_position_int_pack_chan(
            system_data.system_id, 
            system_data.component_id, 
            system_data.channel, 
            &system_data.msg, 
            SIK_TEST_MOCK_BOOT_TIME,   // Time since boot 
            SIK_TEST_MOCK_LAT,         // Latitude 
            SIK_TEST_MOCK_LON,         // Longitude 
            SIK_TEST_MOCK_ALTITUDE,    // Altitude 
            SIK_TEST_MOCK_ALTITUDE,    // Relative altitude (above home) 
            ZERO,                      // X velocity 
            ZERO,                      // Y velocity 
            ZERO,                      // Z velocity 
            ZERO);                     // Heading (yaw angle) 
        sik_radio_test_mavlink_send_msg(); 
    }

    // PARAM_VALUE (response to PARAM_REQUEST_LIST) 
    if (system_data.param_value_msg_timing.enable && 
       (++system_data.param_value_msg_timing.count >= 
          system_data.param_value_msg_timing.count_lim))
    {
        system_data.param_value_msg_timing.count = CLEAR; 

        if (system_data.param_index < SIK_TEST_NUM_PARAMS)
        {
            mavlink_msg_param_value_pack_chan(
                system_data.system_id, 
                system_data.component_id, 
                system_data.channel, 
                &system_data.msg, 
                system_params[system_data.param_index].name, 
                system_params[system_data.param_index].value, 
                MAV_PARAM_TYPE_REAL32, 
                SIK_TEST_NUM_PARAMS, 
                system_data.param_index); 
            sik_radio_test_mavlink_send_msg(); 

            system_data.param_index++; 
        }
        else 
        {
            system_data.param_value_msg_timing.enable = CLEAR_BIT; 
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
    system_data.at_mode_request_timer = CLEAR; 
    system_data.at_mode_requested = CLEAR_BIT; 
    sik_radio_test_user_output(user_msg); 
    sik_radio_test_user_output(sik_test_user_prompt); 
}


// Serialize and send MAVLink message 
void sik_radio_test_mavlink_send_msg(void)
{
    sik_send_data(radio_data.data_out_buff, 
                  mavlink_msg_to_send_buffer(radio_data.data_out_buff, &system_data.msg)); 
}

//=======================================================================================
