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
// Macros 

// File system 
#define BUFF_SIZE 255 

// User interface 
#define HW125_NUM_DRIVER_CMDS 17            // Number of driver test commands for the user 
#define HW125_NUM_CONT_CMDS 18              // Number of controller test commands for the user 
#define CMD_SIZE 50                         // Max user command string length 

// Controller testing 
#define HW125_NUM_USER_CMDS 10              // Number of defined user commands for controller test 
#define HW125_MAX_SETTER_ARGS 1             // Maximum arguments of all function pointer below 

//=======================================================================================


//=======================================================================================
// Enums 

// User input type 
typedef enum {
    FORMAT_FILE_STRING, 
    FORMAT_FILE_MODE, 
    FORMAT_FILE_NUM 
} format_user_input_t; 

//=======================================================================================


//=======================================================================================
// Prototypes 

// User command functions 
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
void display_buffer(void);            // Display the contents of 'buffer' 

// Get user inputs 
void get_input(
    char *str, 
    char *buff, 
    uint8_t buff_len, 
    QWORD *data, 
    format_user_input_t op);

// Format user input 
uint8_t format_input(
    char *buff, 
    QWORD *data, 
    format_user_input_t op);

//=======================================================================================


//=======================================================================================
// Global variables 

// Data record 
typedef struct hw125_test_record_s 
{
    // Peripherals 
    SPI_TypeDef *spi;
    USART_TypeDef *uart;
    TIM_TypeDef *tim;

    // User data 
    BYTE access_mode;                     // File access mode (byte) 
    QWORD position;                       // File position (byte num) 
    QWORD read_len;                       // Read data size (bytes) 
    BYTE cmd_index;                       // For indixing function pointers 

    // User and data buffers 
    char cmd_buff[CMD_SIZE];              // Stores user commands 
    char buffer[BUFF_SIZE];               // To store the data that we can read or write
    char file_name_buff[CMD_SIZE];        // Stores file names input by the user 
    char file_mode_buff[CMD_SIZE];        // Stores file access modes input by the user 
    
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
} 
hw125_test_record_t; 

// Command pointers 
typedef struct hw125_user_cmds_s 
{
    char user_cmds[CMD_SIZE];              // Stores the defined user input commands 
    void (*fatfs_func_ptrs_t)(void);       // Pointer to FatFs file operation function 
}
hw125_user_cmds_t;

// Data record instance 
static hw125_test_record_t hw125_data; 

// FatFs layer disk status - used for clearing the init status for re-mounting 
extern Disk_drvTypeDef disk;

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

//=======================================================================================


//=======================================================================================
// Setup code

void hw125_test_init()
{
    // Initialize data 
    hw125_data.spi = SPI2;
    hw125_data.uart = USART2;
    hw125_data.tim = TIM9;

    //==================================================
    // Peripherals 

    // Initialize GPIO ports 
    gpio_port_init(); 
    
    // Initialize timers 
    tim_9_to_11_counter_init(
        hw125_data.tim, 
        TIM_84MHZ_1US_PSC, 
        0xFFFF,  // Max ARR value 
        TIM_UP_INT_DISABLE); 
    tim_enable(hw125_data.tim); 

    // UART2 for serial terminal communication 
    uart_init(
        hw125_data.uart, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_PARAM_DISABLE,    // Word length 
        CLEAR_BIT,             // STOP bits 
        UART_FRAC_42_9600, 
        UART_MANT_42_9600, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE); 

    // SPI and slave select pin for SD card 
    spi_init(
        hw125_data.spi, 
        GPIOB,   // SCK pin GPIO port 
        PIN_10,  // SCK pin 
        GPIOB,   // Data (MISO/MOSI) pin GPIO port 
        PIN_14,  // MISO pin 
        PIN_15,  // MOSI pin 
        SPI_BR_FPCLK_8, 
        SPI_CLOCK_MODE_0); 
    spi_ss_init(GPIOB, PIN_12); 

    //==================================================

    //==================================================
    // SD card init 

    // SD card user initialization 
    hw125_user_init(hw125_data.spi, GPIOB, GPIOX_PIN_12); 
    
    //==================================================

    //==================================================
    // Setup 

    // Short delay to let the system set up 
    tim_delay_ms(hw125_data.tim, 500); 

    //==================================================
} 

//=======================================================================================


//=======================================================================================
// Test code 

void hw125_test_app()
{
    // Look for a user command 
    get_input(
        "\r\nOperation >>> ", 
        hw125_data.cmd_buff,  
        CMD_SIZE, 
        &hw125_data.read_len, 
        FORMAT_FILE_STRING); 

    // Compare the input to the defined user commands 
    for (uint8_t i = CLEAR; i < HW125_NUM_DRIVER_CMDS; i++)
    {
        if (str_compare(hw125_data.cmd_buff, cmd_table[i].user_cmds, BYTE_0)) 
        {
            (cmd_table[i].fatfs_func_ptrs_t)();
            break; 
        }
    }

    // Delay 
    tim_delay_ms(hw125_data.tim, 1);
}

//=======================================================================================


//=======================================================================================
// User command functions 

// Mount card 
void mount_card(void)
{
#if FORMAT_EXFAT
    
    // Test to see if this will erase existing data 
    
    // Format the drive 
    fresult = f_mkfs("", FM_EXFAT, 0, work, sizeof work); 
    if (fresult != FR_OK)
    {
        uart_send_str(hw125_data.uart, "Error in formatting the SD Card.\r\n");
    }
    else
    {
        uart_send_str(hw125_data.uart, "SD Card formatted successfully.\r\n"); 
    }

#endif

    hw125_data.fresult = f_mount(&hw125_data.file_sys, "", HW125_MOUNT_NOW); 

    if (hw125_data.fresult == FR_OK) 
    {
        uart_send_str(hw125_data.uart, "\nMounted successfully. Volume type: "); 

        // Check the volume type 
        switch (hw125_get_card_type())
        {
            case HW125_CT_MMC: 
                uart_send_str(hw125_data.uart, "MMC V3\r\n");
                break;
            case HW125_CT_SDC1: 
                uart_send_str(hw125_data.uart, "SDC V1\r\n");
                break;
            case HW125_CT_SDC2_BLOCK: 
                uart_send_str(hw125_data.uart, "SDC V2 block\r\n");
                break;
            case HW125_CT_SDC2_BYTE: 
                uart_send_str(hw125_data.uart, "SDC V2 byte\r\n");
                break;
            default: 
                uart_send_str(hw125_data.uart, "Unknown\r\n");
                break;
        }
    }
    else 
    {
        uart_send_str(hw125_data.uart, "\nError in mounting volume.\r\n");
    }
}


// Unmount card 
void unmount_card(void) 
{
    // Unmount the volume 
    hw125_data.fresult = f_unmount(""); 

    // Clear the initialization status so it can be re-mounted 
    disk.is_initialized[0] = CLEAR; 

    if (hw125_data.fresult == FR_OK) 
    {
        uart_send_str(hw125_data.uart, "\nVolume unmounted successfully.\r\n"); 
    }
    else 
    {
        uart_send_str(hw125_data.uart, "\nError in unmounting volume.\r\n");
    }
}


// Check card capacity 
void card_capacity(void)
{
    // These calcs assume 512 bytes/sector 

    // Check free space 
    f_getfree("", &hw125_data.fre_clust, &hw125_data.pfs);

    // Calculate the total space 
    hw125_data.total = (uint32_t)((hw125_data.pfs->n_fatent - 2) * hw125_data.pfs->csize / 2);
    sprintf(hw125_data.buffer, 
            "\nSD CARD Total Size: \t%lu KB\r\n", 
            hw125_data.total);
    uart_send_str(hw125_data.uart, hw125_data.buffer);
    
    // Calculate the free space 
    hw125_data.free_space = (uint32_t)(hw125_data.fre_clust * hw125_data.pfs->csize / 2);
    sprintf(hw125_data.buffer, 
            "SD CARD Free Space: \t%lu KB\r\n", 
            hw125_data.free_space);
    uart_send_str(hw125_data.uart, hw125_data.buffer);
}


// Check files on the card 
void file_check(void)
{
    // Get and format the directory to check 
    get_input(
        "\nPath: ", 
        hw125_data.buffer, 
        BUFF_SIZE, 
        &hw125_data.read_len, 
        FORMAT_FILE_STRING); 
    
    uart_send_str(hw125_data.uart, "\nFiles in path: \r\n"); 

    // Start to search for files 
    hw125_data.fresult = f_findfirst(&hw125_data.dj, &hw125_data.fno, hw125_data.buffer, "*"); 

    while ((hw125_data.fresult == FR_OK) && hw125_data.fno.fname[0]) 
    {
        uart_send_str(hw125_data.uart, "\t- "); 
        uart_send_str(hw125_data.uart, hw125_data.fno.fname); 
        uart_send_new_line(hw125_data.uart); 
        hw125_data.fresult = f_findnext(&hw125_data.dj, &hw125_data.fno); 
    }

    f_closedir(&hw125_data.dj);
}


// Make a new directory 
void file_mkdir(void)
{
    // Get and format the directory path string 
    get_input(
        "\nDirectory: ", 
        hw125_data.buffer, 
        BUFF_SIZE, 
        &hw125_data.read_len, 
        FORMAT_FILE_STRING); 

    // Write to the file 
    hw125_data.fresult = f_mkdir(hw125_data.buffer); 
}


// Open a file 
void file_open(void)
{
    // Get and format the file name 
    get_input(
        "\nFile to open: ", 
        hw125_data.file_name_buff, 
        CMD_SIZE, 
        &hw125_data.read_len, 
        FORMAT_FILE_STRING); 

    // Get and format the access mode 
    get_input(
        "\nAccess mode: ", 
        hw125_data.file_mode_buff, 
        CMD_SIZE, 
        (QWORD *)(&hw125_data.access_mode), 
        FORMAT_FILE_MODE); 

    // Open a file (and create if it doesn't exist) 
    hw125_data.fresult = f_open(&hw125_data.file, hw125_data.file_name_buff, hw125_data.access_mode); 
}


// Close the open file 
void file_close(void) 
{
    f_close(&hw125_data.file); 
}


// Write to an open file using f_puts 
void file_put_string(void)
{
    // Get and format the file string 
    get_input(
        "\nFile string: ", 
        hw125_data.buffer, 
        BUFF_SIZE, 
        &hw125_data.read_len, 
        FORMAT_FILE_STRING); 

    // Write a string 
    f_puts(hw125_data.buffer, &hw125_data.file); 
}


// Read from an open file using f_gets 
void file_get_string(void)
{
    // Get and format the read size (bytes) 
    get_input(
        "\nRead size (bytes): ", 
        hw125_data.buffer, 
        BUFF_SIZE, 
        &hw125_data.read_len, 
        FORMAT_FILE_NUM); 

    // Read from the file 
    f_gets(hw125_data.buffer, hw125_data.read_len, &hw125_data.file); 
}


// Write a formatted dtring 
void file_printf(void)
{
    QWORD fmt_value; 

    // Get and format the formatted string integer 
    get_input(
        "\nInteger: ", 
        hw125_data.buffer, 
        BUFF_SIZE, 
        &fmt_value, 
        FORMAT_FILE_NUM); 
    
    // Get and format the formated string 
    get_input(
        "\nFormatted string: ", 
        hw125_data.buffer, 
        BUFF_SIZE, 
        &hw125_data.read_len, 
        FORMAT_FILE_STRING); 

    // Write the formatted string to the file 
    if (f_printf(&hw125_data.file, hw125_data.buffer, (uint16_t)fmt_value) < 0) 
    {
        uart_send_str(hw125_data.uart, "\nfailure\r\n"); 
    }
}


// Write to an open file using f_write 
void file_write(void) 
{
    // Get and format the file string 
    get_input(
        "\nFile string: ", 
        hw125_data.buffer, 
        BUFF_SIZE, 
        &hw125_data.read_len, 
        FORMAT_FILE_STRING); 

    // Write to the file 
    hw125_data.fresult = f_write(&hw125_data.file, 
                                 hw125_data.buffer, 
                                 strlen(hw125_data.buffer), 
                                 &hw125_data.bw); 

    // Indicate if write failed 
}


// Read from an open file using using f_read 
void file_read(void) 
{
    // Get and format the read size (bytes) 
    get_input(
        "\nRead size (bytes): ", 
        hw125_data.buffer, 
        BUFF_SIZE, 
        &hw125_data.read_len, 
        FORMAT_FILE_NUM); 

    // Read from the file 
    hw125_data.fresult = f_read(&hw125_data.file, 
                                hw125_data.buffer, 
                                hw125_data.read_len, 
                                &hw125_data.br); 

    // Indicate if the read failed 
}


// Navigate the file 
void file_seek(void) 
{
    // Get and format the file position 
    get_input(
        "\nFile position: ", 
        hw125_data.buffer, 
        BUFF_SIZE, 
        &hw125_data.position, 
        FORMAT_FILE_NUM); 

    // Move to the specified position in the file 
    hw125_data.fresult = f_lseek(&hw125_data.file, hw125_data.position); 
}


// Navigate to the beginning of the file 
void file_rewind(void) 
{
    hw125_data.fresult = f_lseek(&hw125_data.file, RESET_ZERO); 
}


// Navigate to the end of the file 
void file_fast_fwd(void) 
{
    hw125_data.fresult = f_lseek(&hw125_data.file, f_size(&hw125_data.file)); 
}


// Remove files on card 
void file_remove(void) 
{
    // Get and format the file position 
    get_input(
        "\nFile to remove: ", 
        hw125_data.file_name_buff, 
        CMD_SIZE, 
        &hw125_data.read_len, 
        FORMAT_FILE_STRING); 

    // Attempt to remove the specified file 
    hw125_data.fresult = f_unlink(hw125_data.file_name_buff); 

    if (hw125_data.fresult != FR_OK) 
    {
        uart_send_str(hw125_data.uart, "\r\nFailed to remove "); 
        uart_send_str(hw125_data.uart, hw125_data.file_name_buff); 
        uart_send_new_line(hw125_data.uart); 
    }
}


// Display the contents of 'buffer' 
void display_buffer(void)
{
    uart_send_str(hw125_data.uart, "\r\nbuffer: \r\n\t"); 
    uart_send_str(hw125_data.uart, hw125_data.buffer); 
    uart_send_new_line(hw125_data.uart); 
}

//=======================================================================================


//=======================================================================================
// Helper functions 

// Get user inputs 
void get_input(
    char *str, 
    char *buff, 
    uint8_t buff_len, 
    QWORD *data, 
    format_user_input_t op)
{
    do 
    {
        // Get the info from the user 
        uart_send_str(hw125_data.uart, str); 
        while(!uart_data_ready(hw125_data.uart)); 
        uart_get_data(hw125_data.uart, buff); 
    }
    while (!format_input(buff, data, op)); 
}


// Format user input 
uint8_t format_input(
    char *buff, 
    QWORD *data, 
    format_user_input_t op)
{
    uint8_t result = FALSE; 

    if (buff == NULL) 
    {
        return result; 
    }

    switch (op)
    {
        case FORMAT_FILE_STRING: 
            // Replace carriage return from input with a null character 
            for (uint8_t i = 0; i < CMD_SIZE; i++)
            {
                if (*buff == CR_CHAR)
                {
                    *buff = NULL_CHAR; 
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
                        nibble -= NUM_TO_CHAR_OFFSET; 
                    }
                    else if ((nibble >= A_UP_CHAR) && (nibble <= F_UP_CHAR)) 
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

            while (*buff_copy != CR_CHAR) 
            {
                if (!((*buff_copy >= ZERO_CHAR) && (*buff_copy <= NINE_CHAR))) break; 
                buff_copy++; 
            }

            if (*buff_copy == CR_CHAR) 
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

//=======================================================================================
