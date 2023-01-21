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

#else   // HW125_CONTROLLER_TEST 

//==================================================
// FatFs functions 

void mount_card(void);                // Mount the SD card 
void unmount_card(void);              // Unmount the SD card 
void card_capacity(void);             // Card Capacity 
void file_check(void);                // Check files on card 
void file_open(void);                 // Open a file 
void file_close(void);                // Close the open file 
void file_put_string(void);           // Write to an open file using f_puts 
void file_seek(void);                 // Navigate the file 
void file_get_string(void);           // Read from an open file using f_gets 
void open_write_read(void);           // f_write and f_read
void update_file(void);               // Update an existing file 
void file_remove(void);               // Remove files from the drive 

//==================================================

//==================================================
// UI functions 

// To find the size of data in the buffer 
int buff_size(char *buff); 

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

#endif   // HW125_CONTROLLER_TEST 

//=======================================================================================


//=======================================================================================
// Global variables 

#if HW125_CONTROLLER_TEST 

#else   // HW125_CONTROLLER_TEST 

FATFS   file_sys;                  // File system 
FIL     file;                      // File 
FRESULT fresult;                   // Store the result of each operation 
char    buffer[BUFF_SIZE];         // To store the data that we can read or write
char    file_name_buff[CMD_SIZE];  // 
char    file_mode_buff[CMD_SIZE];  // 
UINT    br, bw;                    // Stores f_read and f_write byte counters 
BYTE    access_mode;               // File access mode byte 
QWORD   position;                  // 
QWORD   read_len;                  // 
BYTE    cmd_index;                 // 

#if FORMAT_EXFAT 

// Format drive variables 
BYTE    work[512];           // 

#endif   // FORMAT_EXFAT

// Capacity related variables 
FATFS    *pfs; 
DWORD    fre_clust; 
uint32_t total, free_space; 

// For f_findfirst
DIR dj;                     // Directory object 
FILINFO fno;                // File information 

// Data record 
typedef struct hw125_test_record_s 
{
    FATFS   file_sys;                  // File system 
    FIL     file;                      // File 
    FRESULT fresult;                   // Store the result of each operation 
    char    buffer[BUFF_SIZE];         // To store the data that we can read or write
    char    file_name_buff[CMD_SIZE];  // 
    char    file_mode_buff[CMD_SIZE];  // 
    UINT    br, bw;                    // Stores f_read and f_write byte counters 
    BYTE    access_mode;               // File access mode byte 
    QWORD   position;                  // 
    QWORD   read_len;                  // 

    // Capacity related variables 
    FATFS    *pfs; 
    DWORD    fre_clust; 
    uint32_t total, free_space; 

    // For f_findfirst
    DIR dj;                     // Directory object 
    FILINFO fno;                // File information 

#if FORMAT_EXFAT 

    // Format drive variables 
    BYTE    work[512];           // 

#endif   // FORMAT_EXFAT
} 
hw125_test_record_t; 

// Data record instance 
static hw125_test_record_t hw125_test_record; 

// User commands 
static char *usr_cmd_table[HW125_NUM_DRIVER_CMDS] = 
{
    "f_mount",        // mount_card
    "f_unmount",      // unmount_card
    "capacity",       // card_capacity
    "file_check",     // file_check
    "f_open",         // file_open
    "f_close",        // file_close
    "f_puts",         // file_put_string
    "f_gets",         // file_get_string
    "f_lseek",        // file_seek
    // "f_write", 
    // "f_read", 
    "f_unlink",       // 
    "read_buffer"     // display_buffer
}; 

// FatFs function pointers 
static fatfs_func_ptrs_t func_table[HW125_NUM_DRIVER_CMDS] = 
{
    &mount_card, 
    &unmount_card, 
    &card_capacity, 
    &file_check, 
    &file_open, 
    &file_close, 
    &file_put_string, 
    &file_get_string, 
    &file_seek, 
    // &file_write, 
    // &file_read, 
    &file_remove, 
    &display_buffer 
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
    hw125_controller_init(); 

    // State machine test 
    state_machine_init(HW125_NUM_USER_CMDS); 

#endif   // HW125_CONTROLLER_TEST 
    
    //==================================================

    //==================================================
    // Setup 

#if HW125_CONTROLLER_TEST 

#else   // HW125_CONTROLLER_TEST

    // uint8_t card_type = hw125_init(GPIOB_PIN_9);

    // // Check the SD card initialization status 
    // switch (card_type)
    // {
    //     case HW125_CT_UNKNOWN:  // Error occured 
    //         uart_sendstring(USART2, "Error: Unknown card\r\n");
    //         break;
    //     case HW125_CT_MMC:  // MMC V3 
    //         uart_sendstring(USART2, "MMC V3\r\n");
    //         break;
    //     case HW125_CT_SDC1:  // SDC V1 
    //         uart_sendstring(USART2, "SDC V1\r\n");
    //         break;
    //     case HW125_CT_SDC2_BLOCK:  // SDC V2 block 
    //         uart_sendstring(USART2, "SDC V2 block\r\n");
    //         break;
    //     case HW125_CT_SDC2_BYTE:  // SDC V2 byte 
    //         uart_sendstring(USART2, "SDC V2 byte\r\n");
    //         break;
    //     default:  // Unknown response 
    //         uart_sendstring(USART2, "Unknown response\r\n");
    //         break;
    // }

    // Short delay to let the system set up 
    tim_delay_ms(TIM9, 500); 

#endif   // HW125_CONTROLLER_TEST

    //==================================================
} 


// Test code 
void hw125_test_app()
{
#if HW125_CONTROLLER_TEST 

#else   // HW125_CONTROLLER_TEST 

    // Local variables 
    // static uint8_t count = 1; 

    //==================================================
    // Get user command 

    cmd_index = 0xFF; 

    // Look for a user command 
    get_input(
        "\r\nFatFs operation >>> ", 
        buffer, &read_len, FORMAT_FILE_STRING); 

    // Compare the input to the defined user commands 
    for (uint8_t i = 0; i < HW125_NUM_DRIVER_CMDS; i++) 
    {
        if (str_compare(buffer, usr_cmd_table[i], BYTE_0)) 
        {
            cmd_index = i; 
            break; 
        }
    }

    // Use the index to call the function as needed 
    if (cmd_index != 0xFF) 
    {
        (func_table[cmd_index])(); 
    } 

    //==================================================

    // // Run once - code is here to put everything in the same file 
    // if (count)
    // {
    //     //==================================================
    //     // Mount the card and check the contents 

    //     // Mount the SD card 
    //     mount_card(); 

    //     // Card Capacity 
    //     card_capacity(); 

    //     // Check files on card 
    //     file_check(); 

    //     //==================================================
        
        
    //     //==================================================
    //     // Open a file, write to it then check the contents 
        
    //     // Open a file 
    //     file_open(); 
        
    //     // Write to open file using f_puts 
    //     file_put_string(); 

    //     // Move to the beginning of the open file 
    //     file_seek(); 

    //     // Read the string from the open file using f_gets 
    //     // file_get_string(f_size(&file)); 
    //     file_get_string(); 

    //     // Display the contents of 'buffer' 
    //     display_buffer(); 

    //     // Close the open file 
    //     file_close(); 

    //     // Check files on card 
    //     file_check(); 

    //     //==================================================


    //     //==================================================
    //     // Open a second file, write to it and check the contents 

    //     // f_write and f_read
    //     // open_write_read(); 

    //     // Check files on card 
    //     // file_check(); 

    //     //==================================================


    //     //==================================================
    //     // Open the second file, append data and check the contents 

    //     // Update an existing file 
    //     // update_file(); 

    //     // Check files on card 
    //     // file_check(); 
 
    //     //==================================================


    //     //==================================================
    //     // Remove the files and unmount the drive 

    //     // Remove files from the drive 
    //     file_remove(); 

    //     // Check files on card 
    //     file_check(); 
 
    //     // Unmount the SD card 
    //     unmount_card(); 

    //     //==================================================
        
    //     count = 0; 
    // }

    // Delay 
    tim_delay_ms(TIM9, 1);

#endif   // HW125_CONTROLLER_TEST
}

//=======================================================================================


//=======================================================================================
// Test functions 

#if HW125_CONTROLLER_TEST 

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

    fresult = f_mount(&file_sys, "", HW125_MOUNT_NOW); 

    if (fresult == FR_OK) 
    {
        uart_sendstring(USART2, "\nSD Card mounted successfully.\r\n"); 
    }
    else 
    {
        uart_sendstring(USART2, "\nError in mounting SD Card.\r\n");
    }
}


// Unmount card 
void unmount_card(void) 
{
    fresult = f_unmount(""); 

    if (fresult == FR_OK) 
    {
        uart_sendstring(USART2, "\nSD unmounted successfully.\r\n"); 
    }
    else 
    {
        uart_sendstring(USART2, "\nError in unmounting SD Card.\r\n");
    }
}


// Check card capacity 
void card_capacity(void)
{
    // These calcs assume 512 bytes/sector 

    // Check free space 
    f_getfree("", &fre_clust, &pfs);

    // Calculate the total space 
    total = (uint32_t)((pfs->n_fatent - 2) * pfs->csize * 0.5);
    sprintf(buffer, "\nSD CARD Total Size: \t%lu KB\r\n", total);
    uart_sendstring(USART2, buffer);
    memset((void *)buffer, CLEAR, BUFF_SIZE); 
    
    // Calculate the free space 
    free_space = (uint32_t)(fre_clust * pfs->csize * 0.5);
    sprintf(buffer, "SD CARD Free Space: \t%lu KB\r\n", free_space);
    uart_sendstring(USART2, buffer);
    memset((void *)buffer, CLEAR, BUFF_SIZE); 
}


// Check files on the card 
void file_check(void)
{
    uart_sendstring(USART2, "\nCurrent files on drive: \r\n"); 

    // Start to search for files 
    fresult = f_findfirst(&dj, &fno, "", "*"); 

    while (fresult == FR_OK && fno.fname[0]) 
    {
        uart_sendstring(USART2, "\t- "); 
        uart_sendstring(USART2, fno.fname); 
        uart_send_new_line(USART2); 
        fresult = f_findnext(&dj, &fno);         // Search for next item 
    }

    f_closedir(&dj);
}


// Open a file 
void file_open(void)
{
    // Get and format the file name 
    get_input(
        "\nFile to open: ", 
        file_name_buff, &read_len, FORMAT_FILE_STRING); 

    // Get and format the access mode 
    get_input(
        "\nAccess mode: ", 
        file_mode_buff, (QWORD *)(&access_mode), FORMAT_FILE_MODE); 

    // Open a file (and create if it doesn't exist) 
    fresult = f_open(&file, file_name_buff, access_mode); 
}


// Close the open file 
void file_close(void) 
{
    f_close(&file); 
}


// Write to an open file using f_puts 
void file_put_string(void)
{
    // Get and format the file string 
    get_input(
        "\nFile string: ", 
        buffer, &read_len, FORMAT_FILE_STRING); 

    // Write a string 
    f_puts(buffer, &file); 
}


// Navigate the file 
void file_seek(void) 
{
    // Get and format the file position 
    get_input(
        "\nFile position: ", 
        buffer, &position, FORMAT_FILE_NUM); 

    // Move to the specified position in the file 
    fresult = f_lseek(&file, position); 
}


// Read from an open file using f_gets 
void file_get_string(void)
{
    // Get and format the read size (bytes) 
    get_input(
        "\nRead size (bytes): ", 
        buffer, &read_len, FORMAT_FILE_NUM); 

    // Read from the file 
    f_gets(buffer, read_len, &file); 
}


// Open with f_write and f_read 
void open_write_read(void)
{
    // Create/open a second file 
    fresult = f_open(&file, "test_file_2.txt", HW125_MODE_OAWR); 

    // Create text to write to the file 
    strcpy(buffer, "This file was written to using f_write and read using r_read.\n"); 

    // Write the text 
    fresult = f_write(&file, buffer, buff_size(buffer), &bw); 

    // Close the file 
    uart_sendstring(USART2, "test_file_2.txt written to.\r\n"); 
    f_close(&file); 

    // Clear the buffer so we can clearly see the contents read from the file 
    memset((void *)buffer, CLEAR, BUFF_SIZE);  

    // Open the file 
    fresult = f_open(&file, "test_file_2.txt", FA_READ); 

    if (fresult == FR_OK) 
    {
        uart_sendstring(USART2, "test_file_2.txt successfully opened.\r\n"); 
    }

    // Read the data from the file 
    uart_sendstring(USART2, "test_file_2.txt contents: \r\n"); 
    uart_sendstring(USART2, "\t"); 
    f_read(&file, buffer, f_size(&file), &br); 
    uart_sendstring(USART2, buffer); 
    uart_send_new_line(USART2); 
    uart_send_new_line(USART2); 

    // Close the file 
    f_close(&file); 
    memset((void *)buffer, CLEAR, BUFF_SIZE);  
}


// Update an existing file 
void update_file(void)
{
    // Open the file to edit 
    fresult = f_open(&file, "test_file_2.txt", HW125_MODE_OAWR); 

    // Move to the end of the file to append data - could also use HW125_MODE_AA 
    fresult = f_lseek(&file, f_size(&file)); 

    if (fresult == FR_OK) 
    {
        uart_sendstring(USART2, "test_file_2.txt will be appended\r\n"); 
    }

    // Write to the end of the file 
    fresult = f_puts("Appended data.\r\n", &file); 

    // Close the file 
    f_close(&file); 
    memset((void *)buffer, CLEAR, BUFF_SIZE);  

    // Open the file to read the contents 
    fresult = f_open(&file, "test_file_2.txt", FA_READ); 

    // Read the data on the file 
    fresult = f_read(&file, buffer, f_size(&file), &br); 

    if (fresult == FR_OK) 
    {
        uart_sendstring(USART2, "test_file_2.txt updated data: \r\n\t"); 
    }

    uart_sendstring(USART2, buffer); 
    uart_send_new_line(USART2); 

    // Close the file 
    f_close(&file); 
    memset((void *)buffer, CLEAR, BUFF_SIZE);  
}


// Remove files on card 
void file_remove(void) 
{
    // Get and format the file position 
    get_input(
        "\nFile to remove: ", 
        file_name_buff, &read_len, FORMAT_FILE_STRING); 

    // Attempt to remove the specified file 
    // fresult = f_unlink("/test_file.txt"); 
    fresult = f_unlink(file_name_buff); 

    if (fresult != FR_OK) 
    {
        uart_send_new_line(USART2); 
        uart_sendstring(USART2, "Failed to remove "); 
        uart_sendstring(USART2, file_name_buff); 
        uart_send_new_line(USART2); 
        // uart_sendstring(USART2, "test_file.txt removed successfully.\r\n"); 
    }

    // fresult = f_unlink("/test_file_2.txt"); 

    // if (fresult == FR_OK) 
    // {
    //     uart_sendstring(USART2, "test_file_2.txt removed successfully.\r\n"); 
    // }
}


// To find the size of data in the buffer 
int buff_size(char *buff)
{
    int i = 0; 
    while(*buff++ != '\0') i++; 
    return i; 
}


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
    uart_sendstring(USART2, buffer); 
    uart_send_new_line(USART2); 
}

#endif   // HW125_CONTROLLER_TEST

//=======================================================================================
