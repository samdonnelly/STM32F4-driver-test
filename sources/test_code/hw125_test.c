/**
 * @file hw125_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief HW125 test code 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//=======================================================================================
// Includes 

#include "hw125_test.h"

//=======================================================================================


//=======================================================================================
// Function prototypes 

#if HW125_CONTROLLER_TEST 

//==================================================
// Controller setter and getter wrapper functions 

void hw125_cont_test_eject_set(void);      // Set the eject flag 
void hw125_cont_test_eject_clear(void);    // Clear the eject flag 
void hw125_cont_test_reset_set(void);      // Set reset flag 
void hw125_cont_test_make_dir(void);       // Make a new directory in the project directory 
void hw125_cont_test_file_open(void);      // Open file 
void hw125_cont_test_file_close(void);     // Close the open file 
void hw125_cont_test_file_write(void);     // Write to the open file 
void hw125_cont_test_put_string(void);     // Write a string to the open file 
void hw125_cont_test_printf(void);         // Write a formatted string to the open file 
void hw125_cont_test_file_seek(void);      // Navigate within the open file 
void hw125_cont_test_state(void);          // Get state 
void hw125_cont_test_fault_code(void);     // Get fault code 
void hw125_cont_test_fault_mode(void);     // Get fault mode 
void hw125_cont_test_file_status(void);    // Get open file flag 
void hw125_cont_test_file_read(void);      // Read data from open file 
void hw125_cont_test_get_string(void);     // Reads a string from open file 
void hw125_cont_test_file_end(void);       // Test for end of file on open file 

//==================================================

#else   // HW125_CONTROLLER_TEST 

//==================================================
// FatFs functions 

void mount_card(void);                // Mount the SD card 
void unmount_card(void);              // Unmount the SD card 
void card_capacity(void);             // Card Capacity 
void file_check(void);                // Check files on card 
void file_mkdir(void);                // Make a new directory 
void file_open(void);                 // Open a file 
void file_close(void);                // Close the open file 
void file_put_string(void);           // Write to an open file using f_puts 
void file_get_string(void);           // Read from an open file using f_gets 
void file_printf(void);               // Write a formatted dtring 
void file_write(void);                // Write to an open file using f_write 
void file_read(void);                 // Read from an open file using using f_read 
void file_seek(void);                 // Navigate the file 
void file_rewind(void);               // Navigate to the beginning of the file 
void file_fast_fwd(void);             // Navigate to the end of the file 
void file_remove(void);               // Remove files from the drive 

//==================================================

#endif   // HW125_CONTROLLER_TEST 

//==================================================
// UI functions 

// Get user inputs 
void get_input(
    char *str, 
    char *buff, 
    QWORD *data, 
    format_user_input_t op); 

// Format file name 
uint8_t format_input(
    char *buff, 
    QWORD *data, 
    format_user_input_t op); 

// Display the contents of 'buffer' 
void display_buffer(void); 

//==================================================

//=======================================================================================


//=======================================================================================
// Global variables 

// Data record 
typedef struct hw125_test_record_s 
{
    // User data 
    BYTE  access_mode;                    // File access mode (byte) 
    QWORD position;                       // File position (byte num) 
    QWORD read_len;                       // Read data size (bytes) 
    BYTE  cmd_index;                      // For indixing function pointers 

    // User and data buffers 
    char cmd_buff[CMD_SIZE];              // Stores user commands 
    char buffer[BUFF_SIZE];               // To store the data that we can read or write
    char file_name_buff[CMD_SIZE];        // Stores file names input by the user 
    char file_mode_buff[CMD_SIZE];        // Stores file access modes input by the user 

#if HW125_CONTROLLER_TEST 

#else   // HW125_CONTROLLER_TEST
    
    // File variables 
    FATFS   file_sys;                     // File system 
    FIL     file;                         // File 
    FRESULT fresult;                      // Store the result of each operation 
    UINT    br, bw;                       // Stores f_read and f_write byte counters 
    DIR     dj;                           // Directory object 
    FILINFO fno;                          // File information 

    // Card capacity 
    FATFS *pfs;                           // Pointer to file system object 
    DWORD fre_clust;                      // Stores number of free clusters 
    DWORD total, free_space;              // Total and free volume space 

    #if FORMAT_EXFAT 

    BYTE work[512];                       // Used to format the volume 

    #endif   // FORMAT_EXFAT

#endif   // HW125_CONTROLLER_TEST
} 
hw125_test_record_t; 


// Data record instance 
static hw125_test_record_t hw125_test_record; 


#if HW125_CONTROLLER_TEST 

// For user input prompt 
uint8_t action; 

// Command pointers 
typedef struct hw125_user_cmds_s 
{
    char user_cmds[CMD_SIZE];              // Stores the defined user input commands 
    void (*fatfs_func_ptrs_t)(void);       // Pointer to FatFs file operation function 
}
hw125_user_cmds_t; 


// User commands 
static hw125_user_cmds_t cmd_table[HW125_NUM_CONT_CMDS] = 
{
    {"eject",       &hw125_cont_test_eject_set}, 
    {"insert",      &hw125_cont_test_eject_clear}, 
    {"reset",       &hw125_cont_test_reset_set}, 
    {"make_dir",    &hw125_cont_test_make_dir}, 
    {"open",        &hw125_cont_test_file_open}, 
    {"close",       &hw125_cont_test_file_close}, 
    {"write",       &hw125_cont_test_file_write}, 
    {"puts",        &hw125_cont_test_put_string}, 
    {"printf",      &hw125_cont_test_printf}, 
    {"seek",        &hw125_cont_test_file_seek}, 
    {"state",       &hw125_cont_test_state}, 
    {"fault_code",  &hw125_cont_test_fault_code}, 
    {"fault_mode",  &hw125_cont_test_fault_mode}, 
    {"status",      &hw125_cont_test_file_status}, 
    {"read",        &hw125_cont_test_file_read}, 
    {"gets",        &hw125_cont_test_get_string}, 
    {"eof",         &hw125_cont_test_file_end}, 
    {"read_buffer", &display_buffer} 
}; 


#else   // HW125_CONTROLLER_TEST 


// FatFs layer disk status - used for clearing the init status for re-mounting 
extern Disk_drvTypeDef disk;


// Command pointers 
typedef struct hw125_user_cmds_s 
{
    char user_cmds[CMD_SIZE];              // Stores the defined user input commands 
    void (*fatfs_func_ptrs_t)(void);       // Pointer to FatFs file operation function 
}
hw125_user_cmds_t; 


// User commands 
static hw125_user_cmds_t cmd_table[HW125_NUM_DRIVER_CMDS] = 
{
    {"f_mount",     &mount_card}, 
    {"f_unmount",   &unmount_card}, 
    {"f_cap",       &card_capacity}, 
    {"f_check",     &file_check}, 
    {"f_mkdir",     &file_mkdir}, 
    {"f_open",      &file_open}, 
    {"f_close",     &file_close}, 
    {"f_puts",      &file_put_string}, 
    {"f_gets",      &file_get_string}, 
    {"f_printf",    &file_printf}, 
    {"f_write",     &file_write}, 
    {"f_read",      &file_read}, 
    {"f_lseek",     &file_seek}, 
    {"f_rewind",    &file_rewind}, 
    {"f_fastfwd",   &file_fast_fwd}, 
    {"f_unlink",    &file_remove}, 
    {"read_buffer", &display_buffer} 
}; 

#endif   // HW125_CONTROLLER_TEST 

//=======================================================================================


//=======================================================================================
// Test code 

// Setup code
void hw125_test_init()
{
    // Setup code for the hw125_test here 

    //==================================================
    // Peripherals 

    // Initialize GPIO ports 
    gpio_port_init(); 
    
    // Initialize timers 
    tim_9_to_11_counter_init(
        TIM9, 
        TIM_84MHZ_1US_PSC, 
        0xFFFF,  // Max ARR value 
        TIM_UP_INT_DISABLE); 
    tim_enable(TIM9); 

    // UART2 for serial terminal communication 
    uart_init(USART2, UART_BAUD_9600, UART_CLOCK_42); 

    // SPI for SD card 
    spi_init(SPI2, GPIOB, SPI2_1_SLAVE, BR_FPCLK_8, SPI_CLOCK_MODE_0);

    //==================================================

    //==================================================
    // SD card init 

    // SD card user initialization 
    hw125_user_init(GPIOB, SPI2, GPIOX_PIN_9);

#if HW125_CONTROLLER_TEST 

    // hw125 controller 
    hw125_controller_init("controller_test"); 

    // State machine test 
    // state_machine_init(HW125_NUM_USER_CMDS); 

#endif   // HW125_CONTROLLER_TEST 
    
    //==================================================

    //==================================================
    // Setup 

#if HW125_CONTROLLER_TEST 

    // action = SET; 
    uart_sendstring(USART2, "\r\nOperation >>> "); 

#else   // HW125_CONTROLLER_TEST

    // Short delay to let the system set up 
    tim_delay_ms(TIM9, 500); 

#endif   // HW125_CONTROLLER_TEST

    //==================================================
} 


// Test code 
void hw125_test_app()
{
    // Local variables 
    hw125_test_record.cmd_index = 0xFF; 

#if HW125_CONTROLLER_TEST 

    if (action)
    {
        action = CLEAR; 
        uart_sendstring(USART2, "\r\nOperation >>> "); 
    }
    else
    {
        // Get the info from the user 
        if (uart_data_ready(USART2))
        {
            action = SET; 

            // Retrieve and format the input 
            uart_getstr(USART2, hw125_test_record.cmd_buff, UART_STR_TERM_CARRIAGE); 

            // Format the input and check for validity 
            if (format_input(hw125_test_record.cmd_buff, 
                             &hw125_test_record.read_len, 
                             FORMAT_FILE_STRING))
            {
                // Compare the input to the defined user commands 
                for (uint8_t i = 0; i < HW125_NUM_CONT_CMDS; i++) 
                {
                    if (str_compare(hw125_test_record.cmd_buff, 
                                    cmd_table[i].user_cmds, 
                                    BYTE_0)) 
                    {
                        hw125_test_record.cmd_index = i; 
                        break; 
                    }
                }

                // Use the index to call the function as needed 
                if (hw125_test_record.cmd_index != 0xFF) 
                {
                    (cmd_table[hw125_test_record.cmd_index].fatfs_func_ptrs_t)(); 
                } 
            }
        }
    }

    // HW125 controller 
    hw125_controller(); 

#else   // HW125_CONTROLLER_TEST

    // Look for a user command 
    get_input(
        "\r\nOperation >>> ", 
        hw125_test_record.cmd_buff, &hw125_test_record.read_len, FORMAT_FILE_STRING); 

    // Compare the input to the defined user commands 
    for (uint8_t i = 0; i < HW125_NUM_DRIVER_CMDS; i++) 
    {
        if (str_compare(hw125_test_record.cmd_buff, cmd_table[i].user_cmds, BYTE_0)) 
        {
            hw125_test_record.cmd_index = i; 
            break; 
        }
    }

    // Use the index to call the function as needed 
    if (hw125_test_record.cmd_index != 0xFF) 
    {
        (cmd_table[hw125_test_record.cmd_index].fatfs_func_ptrs_t)(); 
    } 

    // Delay 
    tim_delay_ms(TIM9, 1);

#endif   // HW125_CONTROLLER_TEST
}

//=======================================================================================


//=======================================================================================
// Test functions 

#if HW125_CONTROLLER_TEST 

// Set the eject flag 
void hw125_cont_test_eject_set(void) 
{
    hw125_set_eject_flag(); 
}


// Clear the eject flag 
void hw125_cont_test_eject_clear(void) 
{
    hw125_clear_eject_flag(); 
}


// Set reset flag 
void hw125_cont_test_reset_set(void) 
{
    hw125_set_reset_flag(); 
}


// Make a new directory in the project directory 
void hw125_cont_test_make_dir(void) 
{
    // Get and format the directory path string 
    get_input(
        "\nDirectory: ", 
        hw125_test_record.buffer, &hw125_test_record.read_len, FORMAT_FILE_STRING); 

    // Write to the file 
    // hw125_test_record.fresult = hw125_mkdir(hw125_test_record.buffer); 
    hw125_mkdir(hw125_test_record.buffer); 
}


// Open file 
void hw125_cont_test_file_open(void) 
{
    // Get and format the file name 
    get_input(
        "\nFile to open: ", 
        hw125_test_record.file_name_buff, &hw125_test_record.read_len, FORMAT_FILE_STRING); 

    // Get and format the access mode 
    get_input(
        "\nAccess mode: ", 
        hw125_test_record.file_mode_buff, 
        (QWORD *)(&hw125_test_record.access_mode), 
        FORMAT_FILE_MODE); 

    // Open a file (and create if it doesn't exist) 
    // hw125_test_record.fresult = hw125_open(hw125_test_record.file_name_buff, 
    //                                        hw125_test_record.access_mode); 
    hw125_open(hw125_test_record.file_name_buff, hw125_test_record.access_mode); 
}


// Close the open file 
void hw125_cont_test_file_close(void) 
{
    // hw125_test_record.fresult = hw125_close(); 
    hw125_close(); 
}


// Write to the open file 
void hw125_cont_test_file_write(void) 
{
    // Get and format the file string 
    get_input(
        "\nFile string: ", 
        hw125_test_record.buffer, &hw125_test_record.read_len, FORMAT_FILE_STRING); 

    // Write to the file 
    // hw125_test_record.fresult = hw125_f_write(hw125_test_record.buffer, 
    //                                           strlen(hw125_test_record.buffer)); 
    hw125_f_write(hw125_test_record.buffer, strlen(hw125_test_record.buffer)); 
}


// Write a string to the open file 
void hw125_cont_test_put_string(void) 
{
    // Get and format the file string 
    get_input(
        "\nFile string: ", 
        hw125_test_record.buffer, &hw125_test_record.read_len, FORMAT_FILE_STRING); 

    // Write a string 
    hw125_puts(hw125_test_record.buffer); 
}


// Write a formatted string to the open file 
void hw125_cont_test_printf(void) 
{
    QWORD fmt_value; 

    // Get and format the formatted string integer 
    get_input(
        "\nInteger: ", 
        hw125_test_record.buffer, &fmt_value, FORMAT_FILE_NUM); 
    
    // Get and format the formated string 
    get_input(
        "\nFormatted string: ", 
        hw125_test_record.buffer, &hw125_test_record.read_len, FORMAT_FILE_STRING); 

    // Write the formatted string to the file 
    hw125_printf(hw125_test_record.buffer, (uint16_t)fmt_value); 
}


// Navigate within the open file 
void hw125_cont_test_file_seek(void) 
{
    // Get and format the file position 
    get_input(
        "\nFile position: ", 
        hw125_test_record.buffer, &hw125_test_record.position, FORMAT_FILE_NUM); 

    // Move to the specified position in the file 
    // hw125_test_record.fresult = hw125_lseek(hw125_test_record.position); 
    hw125_lseek(hw125_test_record.position); 
}


// Get state 
void hw125_cont_test_state(void) 
{
    HW125_STATE state = hw125_get_state(); 

    // Show the state 
    uart_sendstring(USART2, "state: "); 
    uart_send_integer(USART2, (int16_t)state); 
    uart_send_new_line(USART2); 
}


// Get fault code 
void hw125_cont_test_fault_code(void) 
{
    HW125_FAULT_CODE code = hw125_get_fault_code(); 

    uart_sendstring(USART2, "fault code: "); 
    uart_send_integer(USART2, (int16_t)code); 
    uart_send_new_line(USART2); 
}


// Get fault mode 
void hw125_cont_test_fault_mode(void)
{
    HW125_FAULT_MODE mode = hw125_get_fault_mode(); 

    uart_sendstring(USART2, "fault mode: "); 
    uart_send_integer(USART2, (int16_t)mode); 
    uart_send_new_line(USART2); 
}


// Get open file flag 
void hw125_cont_test_file_status(void)
{
    HW125_FILE_STATUS status = hw125_get_file_status(); 

    // Show open file flag setpoint 
    uart_sendstring(USART2, "open flag: "); 
    uart_send_integer(USART2, (int16_t)status); 
    uart_send_new_line(USART2); 
}


// Read data from open file 
void hw125_cont_test_file_read(void) 
{
    // Get and format the read size (bytes) 
    get_input(
        "\nRead size (bytes): ", 
        hw125_test_record.buffer, &hw125_test_record.read_len, FORMAT_FILE_NUM); 

    // Read from the file 
    // hw125_test_record.fresult = hw125_f_read(hw125_test_record.buffer, 
    //                                          hw125_test_record.read_len); 
    hw125_f_read(hw125_test_record.buffer, hw125_test_record.read_len); 

    display_buffer(); 
}


// Reads a string from open file 
void hw125_cont_test_get_string(void)
{
    // Get and format the read size (bytes) 
    get_input(
        "\nRead size (bytes): ", 
        hw125_test_record.buffer, &hw125_test_record.read_len, FORMAT_FILE_NUM); 

    // Read from the file 
    hw125_gets(hw125_test_record.buffer, hw125_test_record.read_len); 

    display_buffer(); 
}


// Test for end of file on open file 
void hw125_cont_test_file_end(void) 
{
    int8_t eof_return = hw125_eof(); 

    // Display if end of file has been reached 
    uart_sendstring(USART2, "eof return: "); 
    uart_send_integer(USART2, (int16_t)eof_return); 
    uart_send_new_line(USART2); 
}

#else   // HW125_CONTROLLER_TEST 

// Mount card 
void mount_card(void) 
{
#if FORMAT_EXFAT
    // TODO test to see if this will erase existing data 
    // Format the drive 
    fresult = f_mkfs("", FM_EXFAT, 0, work, sizeof work); 
    if (fresult != FR_OK) uart_sendstring(USART2, "Error in formatting the SD Card.\r\n");
    else uart_sendstring(USART2, "SD Card formatted successfully.\r\n"); 
#endif

    hw125_test_record.fresult = f_mount(&hw125_test_record.file_sys, "", HW125_MOUNT_NOW); 

    if (hw125_test_record.fresult == FR_OK) 
    {
        uart_sendstring(USART2, "\nMounted successfully. Volume type: "); 

        // Check the volume type 
        switch (hw125_get_card_type())
        {
            case HW125_CT_MMC: 
                uart_sendstring(USART2, "MMC V3\r\n");
                break;
            case HW125_CT_SDC1: 
                uart_sendstring(USART2, "SDC V1\r\n");
                break;
            case HW125_CT_SDC2_BLOCK: 
                uart_sendstring(USART2, "SDC V2 block\r\n");
                break;
            case HW125_CT_SDC2_BYTE: 
                uart_sendstring(USART2, "SDC V2 byte\r\n");
                break;
            default: 
                uart_sendstring(USART2, "Unknown\r\n");
                break;
        }
    }
    else 
    {
        uart_sendstring(USART2, "\nError in mounting volume.\r\n");
    }
}


// Unmount card 
void unmount_card(void) 
{
    // Unmount the volume 
    hw125_test_record.fresult = f_unmount(""); 

    // Clear the initialization status so it can be re-mounted 
    // TODO this may be needed for failed mount attempts? 
    disk.is_initialized[0] = CLEAR; 

    if (hw125_test_record.fresult == FR_OK) 
    {
        uart_sendstring(USART2, "\nVolume unmounted successfully.\r\n"); 
    }
    else 
    {
        uart_sendstring(USART2, "\nError in unmounting volume.\r\n");
    }
}


// Check card capacity 
void card_capacity(void)
{
    // These calcs assume 512 bytes/sector 

    // Check free space 
    f_getfree("", &hw125_test_record.fre_clust, &hw125_test_record.pfs);

    // Calculate the total space 
    hw125_test_record.total = (uint32_t)((hw125_test_record.pfs->n_fatent - 2) * 
                                          hw125_test_record.pfs->csize * 0.5);
    sprintf(hw125_test_record.buffer, 
            "\nSD CARD Total Size: \t%lu KB\r\n", 
            hw125_test_record.total);
    uart_sendstring(USART2, hw125_test_record.buffer);
    
    // Calculate the free space 
    hw125_test_record.free_space = (uint32_t)(hw125_test_record.fre_clust * 
                                              hw125_test_record.pfs->csize * 0.5);
    sprintf(hw125_test_record.buffer, 
            "SD CARD Free Space: \t%lu KB\r\n", 
            hw125_test_record.free_space);
    uart_sendstring(USART2, hw125_test_record.buffer);
}


// Check files on the card 
void file_check(void)
{
    // Get and format the directory to check 
    get_input(
        "\nPath: ", 
        hw125_test_record.buffer, &hw125_test_record.read_len, FORMAT_FILE_STRING); 
    
    uart_sendstring(USART2, "\nFiles in path: \r\n"); 

    // Start to search for files 
    hw125_test_record.fresult = f_findfirst(&hw125_test_record.dj, 
                                            &hw125_test_record.fno, 
                                            hw125_test_record.buffer, 
                                            "*"); 

    while ((hw125_test_record.fresult == FR_OK) && hw125_test_record.fno.fname[0]) 
    {
        uart_sendstring(USART2, "\t- "); 
        uart_sendstring(USART2, hw125_test_record.fno.fname); 
        uart_send_new_line(USART2); 
        hw125_test_record.fresult = f_findnext(&hw125_test_record.dj, 
                                               &hw125_test_record.fno); 
    }

    f_closedir(&hw125_test_record.dj);
}


// Make a new directory 
void file_mkdir(void)
{
    // Get and format the directory path string 
    get_input(
        "\nDirectory: ", 
        hw125_test_record.buffer, &hw125_test_record.read_len, FORMAT_FILE_STRING); 

    // Write to the file 
    hw125_test_record.fresult = f_mkdir(hw125_test_record.buffer); 
}


// Open a file 
void file_open(void)
{
    // Get and format the file name 
    get_input(
        "\nFile to open: ", 
        hw125_test_record.file_name_buff, &hw125_test_record.read_len, FORMAT_FILE_STRING); 

    // Get and format the access mode 
    get_input(
        "\nAccess mode: ", 
        hw125_test_record.file_mode_buff, 
        (QWORD *)(&hw125_test_record.access_mode), 
        FORMAT_FILE_MODE); 

    // Open a file (and create if it doesn't exist) 
    hw125_test_record.fresult = f_open(&hw125_test_record.file, 
                                       hw125_test_record.file_name_buff, 
                                       hw125_test_record.access_mode); 
}


// Close the open file 
void file_close(void) 
{
    f_close(&hw125_test_record.file); 
}


// Write to an open file using f_puts 
void file_put_string(void)
{
    // Get and format the file string 
    get_input(
        "\nFile string: ", 
        hw125_test_record.buffer, &hw125_test_record.read_len, FORMAT_FILE_STRING); 

    // Write a string 
    f_puts(hw125_test_record.buffer, &hw125_test_record.file); 
}


// Read from an open file using f_gets 
void file_get_string(void)
{
    // Get and format the read size (bytes) 
    get_input(
        "\nRead size (bytes): ", 
        hw125_test_record.buffer, &hw125_test_record.read_len, FORMAT_FILE_NUM); 

    // Read from the file 
    f_gets(hw125_test_record.buffer, hw125_test_record.read_len, &hw125_test_record.file); 
}


// Write a formatted dtring 
void file_printf(void)
{
    QWORD fmt_value; 

    // Get and format the formatted string integer 
    get_input(
        "\nInteger: ", 
        hw125_test_record.buffer, &fmt_value, FORMAT_FILE_NUM); 
    
    // Get and format the formated string 
    get_input(
        "\nFormatted string: ", 
        hw125_test_record.buffer, &hw125_test_record.read_len, FORMAT_FILE_STRING); 

    // Write the formatted string to the file 
    if (f_printf(&hw125_test_record.file, 
                 hw125_test_record.buffer, 
                 (uint16_t)fmt_value) < 0) 
    {
        uart_sendstring(USART2, "\nfailure\r\n"); 
    }
}


// Write to an open file using f_write 
void file_write(void) 
{
    // Get and format the file string 
    get_input(
        "\nFile string: ", 
        hw125_test_record.buffer, &hw125_test_record.read_len, FORMAT_FILE_STRING); 

    // Write to the file 
    hw125_test_record.fresult = f_write(&hw125_test_record.file, 
                                        hw125_test_record.buffer, 
                                        strlen(hw125_test_record.buffer), 
                                        &hw125_test_record.bw); 

    // Indicate if write failed 
}


// Read from an open file using using f_read 
void file_read(void) 
{
    // Get and format the read size (bytes) 
    get_input(
        "\nRead size (bytes): ", 
        hw125_test_record.buffer, &hw125_test_record.read_len, FORMAT_FILE_NUM); 

    // Read from the file 
    hw125_test_record.fresult = f_read(&hw125_test_record.file, 
                                       hw125_test_record.buffer, 
                                       hw125_test_record.read_len, 
                                       &hw125_test_record.br); 

    // Indicate if the read failed 
}


// Navigate the file 
void file_seek(void) 
{
    // Get and format the file position 
    get_input(
        "\nFile position: ", 
        hw125_test_record.buffer, &hw125_test_record.position, FORMAT_FILE_NUM); 

    // Move to the specified position in the file 
    hw125_test_record.fresult = f_lseek(&hw125_test_record.file, hw125_test_record.position); 
}


// Navigate to the beginning of the file 
void file_rewind(void) 
{
    hw125_test_record.fresult = f_lseek(&hw125_test_record.file, RESET); 
}


// Navigate to the end of the file 
void file_fast_fwd(void) 
{
    hw125_test_record.fresult = f_lseek(&hw125_test_record.file, 
                                        f_size(&hw125_test_record.file)); 
}


// Remove files on card 
void file_remove(void) 
{
    // Get and format the file position 
    get_input(
        "\nFile to remove: ", 
        hw125_test_record.file_name_buff, &hw125_test_record.read_len, FORMAT_FILE_STRING); 

    // Attempt to remove the specified file 
    hw125_test_record.fresult = f_unlink(hw125_test_record.file_name_buff); 

    if (hw125_test_record.fresult != FR_OK) 
    {
        uart_sendstring(USART2, "\r\nFailed to remove "); 
        uart_sendstring(USART2, hw125_test_record.file_name_buff); 
        uart_send_new_line(USART2); 
    }
}

#endif   // HW125_CONTROLLER_TEST

// Get user inputs 
void get_input(
    char *str, 
    char *buff, 
    QWORD *data, 
    format_user_input_t op)
{
    do 
    {
        // Get the info from the user 
        uart_sendstring(USART2, str); 
        while(!uart_data_ready(USART2)); 

        // Retrieve and format the input 
        uart_getstr(USART2, buff, UART_STR_TERM_CARRIAGE); 
    }
    while (!format_input(buff, data, op)); 
}


// Format file input 
uint8_t format_input(
    char *buff, 
    QWORD *data, 
    format_user_input_t op)
{
    uint8_t result = FALSE; 

    if (buff == NULL) return result; 

    switch (op)
    {
        case FORMAT_FILE_STRING: 
            // Replace carriage return from input with a null character 
            for (uint8_t i = 0; i < CMD_SIZE; i++)
            {
                if (*buff == UART_STR_TERM_CARRIAGE)
                {
                    *buff = UART_STR_TERM_NULL; 
                    break; 
                }
                buff++; 
            }

            result = TRUE; 

            break; 
        
        case FORMAT_FILE_MODE: 
            if (str_compare("0x", buff, BYTE_0))
            {
                uint8_t nibble; 
                *data = CLEAR; 
                
                // Check the character validity 
                for (uint8_t i = 2; i < 4; i++) 
                {
                    nibble = buff[i]; 

                    if ((nibble >= ZERO_CHAR) && (nibble <= NINE_CHAR))
                    {
                        nibble -= HEX_TO_NUM_CHAR; 
                    }
                    else if ((nibble >= A_CHAR) && (nibble <= F_CHAR)) 
                    {
                        nibble -= HEX_TO_LET_CHAR; 
                    }
                    else break; 

                    *data |= (nibble << SHIFT_4*(3-i)); 

                    if (i == 3) result = TRUE; 
                }
            }

            break; 

        case FORMAT_FILE_NUM: ; 
            char *buff_copy = buff; 

            while (*buff_copy != UART_STR_TERM_CARRIAGE) 
            {
                if (!((*buff_copy >= ZERO_CHAR) && (*buff_copy <= NINE_CHAR))) break; 
                buff_copy++; 
            }

            if (*buff_copy == UART_STR_TERM_CARRIAGE) 
            {
                *data = atoi(buff); 
                result = TRUE; 
            }

            break; 

        default: 
            break; 
    }

    return result; 
}


// Display the contents of 'buffer' 
void display_buffer(void)
{
    uart_sendstring(USART2, "\r\nbuffer: \r\n\t"); 
    uart_sendstring(USART2, hw125_test_record.buffer); 
    uart_send_new_line(USART2); 
}

//=======================================================================================
