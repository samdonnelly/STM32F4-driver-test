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

// To find the size of data in the buffer 
int buff_size(char *buff); 

// Mount the SD card 
void mount_card(void); 

// Card Capacity 
void card_capacity(void); 
 
// f_puts and f_gets (wrapper functions of f_read and f_write) 
void open_puts_gets(void); 

// Check files on card 
void file_check(void); 

// f_write and f_read
void open_write_read(void); 

// Update an existing file 
void update_file(void); 

// Remove files from the drive 
void remove_files(void); 

// Unmount the SD card 
void unmount_card(void); 

// Format file name 
void format_file_name(
    char *buff); 

#endif   // HW125_CONTROLLER_TEST 

//=======================================================================================


//=======================================================================================
// FATFS variables 

FATFS   file_sys;            // File system 
FIL     file;                // File 
FRESULT fresult;             // Store the result of each operation 
char    buffer[BUFF_SIZE];   // To store the data that we can read or write
char    cmd_buff[CMD_SIZE];  // 
UINT    br, bw;              // Stores the counter to the read and write in the file 

// Format drive variables 
BYTE    work[512];           // 

// Capacity related variables 
FATFS    *pfs; 
DWORD    fre_clust; 
uint32_t total, free_space; 

// For f_findf`irst
DIR dj;                     // Directory object 
FILINFO fno;                // File information 

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

#endif   // HW125_CONTROLLER_TEST

    //==================================================
} 


// Test code 
void hw125_test_app()
{
#if HW125_CONTROLLER_TEST 

#else   // HW125_CONTROLLER_TEST 

    // Local variables 
    static uint8_t count = 1; 

    // Run once - code is here to put everything in the same file 
    if (count)
    {
        // Short delay to let the system set up 
        tim_delay_ms(TIM9, 500); 

#if FORMAT_EXFAT
        // TODO test to see if this will erase existing data 
        // Format the drive 
        fresult = f_mkfs("", FM_EXFAT, 0, work, sizeof work); 
        if (fresult != FR_OK) uart_sendstring(USART2, "Error in formatting the SD Card.\r\n");
        else uart_sendstring(USART2, "SD Card formatted successfully.\r\n"); 
#endif
        // Mount the SD card 
        mount_card(); 

        // Card Capacity 
        card_capacity(); 

        // Check files on card 
        file_check(); 

        // f_puts and f_gets (wrapper functions of f_read and f_write) 
        open_puts_gets(); 

        // Check files on card 
        file_check(); 

        // f_write and f_read
        // open_write_read(); 

        // Check files on card 
        // file_check(); 

        // Update an existing file 
        // update_file(); 

        // Check files on card 
        // file_check(); 
 
        // Remove files from the drive 
        remove_files(); 

        // Check files on card 
        file_check(); 
 
        // Unmount the SD card 
        unmount_card(); 

        count = 0; 
    }

    // Delay 
    tim_delay_ms(TIM9, 1);

#endif   // HW125_CONTROLLER_TEST
}

//=======================================================================================


//=======================================================================================
// Test functions 

#if HW125_CONTROLLER_TEST 

#else   // HW125_CONTROLLER_TEST 

// To find the size of data in the buffer 
int buff_size(char *buff)
{
    int i = 0; 
    while(*buff++ != '\0') i++; 
    return i; 
}


// Mount card 
void mount_card(void) 
{
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

    // uart_send_new_line(USART2); 
}


// Open a file 
void file_open(void)
{
    // Get the file name from the user 
    uart_sendstring(USART2, "\nFile to open: "); 
    while(!uart_data_ready(USART2)); 

    // Retrieve and format the file name 
    uart_getstr(USART2, cmd_buff, UART_STR_TERM_CARRIAGE); 
    format_file_name(cmd_buff); 

    // Get the file access mode from the user 

    // Retrieve and choose a mode 

    // Open a file (and create if it doesn't exist) 
    fresult = f_open(&file, cmd_buff, HW125_MODE_OAWR); 
}


// Write to an open file using f_puts 
void file_put_string(void)
{
    // Get the file string from the user 
    uart_sendstring(USART2, "\nFile string: "); 
    while(!uart_data_ready(USART2)); 

    // Retrieve and format the file string 
    uart_getstr(USART2, buffer, UART_STR_TERM_CARRIAGE); 
    format_file_name(buffer); 

    // Write a string 
    f_puts(buffer, &file); 
}


// Navigate the file 
void file_seek(
    QWORD position) 
{
    // Get user input of where to go to in the file 

    // Move to the beginning of the file 
    fresult = f_lseek(&file, position);  
}


// Read from an open file using f_gets 
void file_get_string(
    uint16_t len)
{
    // Get user input on how much to read 

    // Read from the file 
    f_gets(buffer, len, &file); 
}


// Open and use puts and gets 
void open_puts_gets(void)
{
    // Open file 
    file_open(); 
    
    // Write to open file 
    file_put_string(); 
    
    //==================================================

    // // Close the file 
    // fresult = f_close(&file); 

    // // Check the status of the operation 
    // uart_sendstring(USART2, cmd_buff); 

    // if (fresult == FR_OK) 
    // {
    //     uart_sendstring(USART2, " was successfully written.\r\n\n"); 
    // }
    // else 
    // {
    //     uart_sendstring(USART2, " was NOT successfully written.\r\n\n"); 
    // }

    // Open the file again to read the data 
    // fresult = f_open(&file, cmd_buff, FA_READ); 

    //==================================================
    // Read from the same file 

    // Move to the beginning of the file 
    file_seek(RESET); 

    // Read the string from the file 
    file_get_string(f_size(&file)); 

    // Display the data 
    uart_send_new_line(USART2); 
    uart_sendstring(USART2, cmd_buff); 
    uart_sendstring(USART2, " has been read and the string inside says: \r\n"); 
    uart_sendstring(USART2, buffer); 
    uart_send_new_line(USART2); 

    //==================================================
    
    // Close the file 
    f_close(&file); 
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
void remove_files(void) 
{
    // fresult = f_unlink("/test_file.txt"); 
    fresult = f_unlink(cmd_buff); 

    if (fresult == FR_OK) 
    {
        uart_send_new_line(USART2); 
        uart_sendstring(USART2, cmd_buff); 
        // uart_sendstring(USART2, "test_file.txt removed successfully.\r\n"); 
        uart_sendstring(USART2, " removed successfully.\r\n"); 
    }

    // fresult = f_unlink("/test_file_2.txt"); 

    // if (fresult == FR_OK) 
    // {
    //     uart_sendstring(USART2, "test_file_2.txt removed successfully.\r\n"); 
    // }
}


// Format file input 
void format_file_name(
    char *buff)
{
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
}

#endif   // HW125_CONTROLLER_TEST

//=======================================================================================
