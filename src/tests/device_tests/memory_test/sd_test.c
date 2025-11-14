/**
 * @file sd_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief SD card driver test 
 * 
 * @details Setup 
 *          - Hardware 
 *            * STM32F4 microcontroller with a serial connection to a PC. 
 *            * An SD card connected to the STM32F4 via SPI 
 *          - Software 
 *            * Serial monitor on a PC to allow the exchange of info with the STM32F4. 
 *          
 *          Configuration 
 *          - TIM 
 *            * A generic blocking timer is configured for the SD card driver. 
 *          - UART 
 *            * UART is configured to allow for user input at the serial terminal and 
 *              feedback output. 
 *          - SPI 
 *            * SPI is configured to communicate with the SD card. 
 *          - Interrupts 
 *            * An interrupt is configured to detect UART RX idle line for capturing 
 *              user inputs at the serial terminal. 
 *          
 *          Dependencies 
 *          - STM32F4 driver library 
 *            * This library provides an interface to the device and the peripherals 
 *              used in the test. This includes both the SD card driver and the FatFs 
 *              library. 
 *          
 *          Procedure 
 *          - A prompt is provided in the serial terminal where the user can input 
 *            commands. If an input matches one of the commands in the command table 
 *            then the associated command function will be called, otherwise the input 
 *            is discarded. The commands correspond to various FatFs functions which 
 *            carry out file system operations. This allows the user to manually control 
 *            files on the connected SD card. Feedback from the file system operations 
 *            is provided in the serial terminal as needed (ex. when reading a line from 
 *            an open file). See the FatFs documentation for more details on what each 
 *            FatFs function does. 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//=======================================================================================
// Includes 

#include "sd_test.h"
#include "stm32f4xx_it.h"
#include "includes_drivers.h"

//=======================================================================================


//=======================================================================================
// Macros 

#define DATA_BUFF_SIZE 250     // Data buffer size 
#define FORMAT_BUFF_SIZE 512   // Size of buffer for formatting the volume 
#define CMD_SIZE 50            // Max user command string length 
#define NUM_USER_CMDS 21       // Number of driver test commands for the user 
#define FORMAT_ATTEMPTS 20     // Number of attempts at formatting the volume before timeout 

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
void file_mkfs(void);       // Format the volume 
void mount_card(void);      // Mount the volume 
void unmount_card(void);    // Unmount the volume 
void card_capacity(void);   // Card Capacity 
void file_check(void);      // Check existance of a file or sub-directory 
void file_remove(void);     // Remove files from the drive 
void file_mkdir(void);      // Make a new directory 
void file_chdir(void);      // Change the current directory 
void file_getcwd(void);     // Retrieve the current directory and drive 
void file_opendir(void);    // Open a directory 
void file_closedir(void);   // Close an open directory 
void file_find(void);       // Find files on card 
void file_open(void);       // Open a file 
void file_close(void);      // Close the open file 
void file_read(void);       // Read from an open file using using f_read 
void file_write(void);      // Write to an open file using f_write 
void file_seek(void);       // Navigate the file 
void file_rewind(void);     // Navigate to the beginning of the file 
void file_fast_fwd(void);   // Navigate to the end of the file 
void file_gets(void);       // Read from an open file using f_gets 
void file_puts(void);       // Write to an open file using f_puts 

// Command control 
void cmd_select(void);                                              // Select command based on user input 
void cmd_reset(void);                                               // Return to default state at the end of the command dispatch 
void string_display(const char *str_format, const char *str_arg);   // Display string with string argument 
void number_display(const char *str_format, const DWORD num_arg);   // Display string with number argument 
void fault_display(void);                                           // Display the error code related to the FATFS operation 
void feedback_display(const char *string);                          // Display a string for the user to see 

// Format user input 
uint8_t format_input(char *buff, DWORD *data, format_user_input_t op);

//=======================================================================================


//=======================================================================================
// Global variables 

// Data record 
typedef struct sd_test_record_s 
{
    // Peripherals 
    SPI_TypeDef *spi;
    GPIO_TypeDef *gpio;
    USART_TypeDef *uart;
    DMA_Stream_TypeDef *dma_stream;
    TIM_TypeDef *tim;

    // Serial interface data 
    uint8_t cb[DATA_BUFF_SIZE];              // Circular buffer populated by DMA 
    cb_index_t cb_index;                     // Circular buffer indexing info 
    dma_index_t dma_index;                   // DMA transfer indexing info 
    uint8_t data_in_buff[DATA_BUFF_SIZE];    // Buffer that stores latest UART input 
    uint8_t data_out_buff[DATA_BUFF_SIZE];   // Buffer that stores outgoing data 
    DWORD data_in_num;                       // Number fetched from data input 
    
    // State tracking 
    uint8_t state_seq;                       // Contorls action within a state/command 
    void (*state_func_ptr)(void);            // Dispatches to a chosen command 
    
    // File variables 
    FATFS file_sys;                          // File system 
    FIL file;                                // File 
    FRESULT fresult;                         // Store the result of each operation 
    UINT br, bw;                             // Stores f_read and f_write byte counters 
    DIR dj;                                  // Directory object 
    FILINFO fno;                             // File information 
} 
sd_test_record_t;

// Data record instance 
static sd_test_record_t sd_test_data;


// Command pointers 
typedef struct sd_user_cmds_s 
{
    char user_cmds[CMD_SIZE];           // Stores the defined user input commands 
    void (*sd_func_ptrs_t)(void);       // Pointer to FatFs file operation function 
}
sd_user_cmds_t;

// User commands 
static const sd_user_cmds_t cmd_table[NUM_USER_CMDS] = 
{
    // Volume Management and System Configuration commands 
    {"mount",     &mount_card},
    {"unmount",   &unmount_card},
    {"mkfs",      &file_mkfs},
    {"getfree",   &card_capacity},
    // File and Directory Management commands 
    {"stat",      &file_check},
    {"unlink",    &file_remove},
    {"mkdir",     &file_mkdir},
    {"chdir",     &file_chdir},
    {"getcwd",    &file_getcwd},
    // Directory Access commands 
    {"opendir",   &file_opendir},
    {"closedir",  &file_closedir},
    {"find",      &file_find},
    // File Access commands 
    {"openfile",  &file_open},
    {"closefile", &file_close},
    {"read",      &file_read},
    {"write",     &file_write},
    {"lseek",     &file_seek},
    {"rewind",    &file_rewind},
    {"fastfwd",   &file_fast_fwd},
    {"gets",      &file_gets},
    {"puts",      &file_puts}
};

//=======================================================================================


//=======================================================================================
// Setup code

void sd_test_init()
{
    // Initialize data 
    sd_test_data.spi = SPI2;
    sd_test_data.gpio = GPIOB;
    sd_test_data.uart = USART2;
    sd_test_data.dma_stream = DMA1_Stream5;
    sd_test_data.tim = TIM9;

    memset((void *)sd_test_data.cb, CLEAR, sizeof(sd_test_data.cb));
    sd_test_data.cb_index.cb_size = DATA_BUFF_SIZE;
    sd_test_data.cb_index.head = CLEAR;
    sd_test_data.cb_index.tail = CLEAR;
    sd_test_data.dma_index.data_size = CLEAR;
    sd_test_data.dma_index.ndt_old = dma_ndt_read(sd_test_data.dma_stream);
    sd_test_data.dma_index.ndt_new = CLEAR;
    memset((void *)sd_test_data.data_in_buff, CLEAR, sizeof(sd_test_data.data_in_buff));
    memset((void *)sd_test_data.data_out_buff, CLEAR, sizeof(sd_test_data.data_out_buff));

    sd_test_data.state_seq = CLEAR;

    //==================================================
    // General setup 

    // Initialize GPIO ports 
    gpio_port_init();
    
    // Initialize timers 
    tim_9_to_11_counter_init(
        sd_test_data.tim, 
        TIM_84MHZ_1US_PSC, 
        0xFFFF,  // Max ARR value 
        TIM_UP_INT_DISABLE);
    tim_enable(sd_test_data.tim);

    //==================================================

    //==================================================
    // UART 
    
    // UART2 init - Serial terminal 
    uart_init(
        sd_test_data.uart, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_PARAM_DISABLE,    // Word length 
        CLEAR_BIT,             // STOP bits 
        UART_FRAC_42_9600, 
        UART_MANT_42_9600, 
        UART_PARAM_DISABLE, 
        UART_PARAM_ENABLE);
        
    // UART2 interrupt init - Serial terminal - IDLE line (RX) interrupts 
    uart_interrupt_init(
        sd_test_data.uart, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE, 
        UART_PARAM_ENABLE, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE);
    
    //==================================================

    //==================================================
    // SPI 
    
    // SPI and slave select pin for SD card 
    spi_init(
        sd_test_data.spi, 
        sd_test_data.gpio,   // SCK pin GPIO port 
        PIN_10,              // SCK pin 
        sd_test_data.gpio,   // Data (MISO/MOSI) pin GPIO port 
        PIN_14,              // MISO pin 
        PIN_15,              // MOSI pin 
        SPI_BR_FPCLK_8, 
        SPI_CLOCK_MODE_0);
    spi_ss_init(sd_test_data.gpio, PIN_12);
    
    //==================================================

    //==================================================
    // DMA 

    // DMA1 stream init - UART2 - Serial terminal 
    dma_stream_init(
        DMA1, 
        sd_test_data.dma_stream, 
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
        sd_test_data.dma_stream, 
        (uint32_t)(&sd_test_data.uart->DR), 
        (uint32_t)sd_test_data.cb, 
        (uint32_t)NULL, 
        (uint16_t)DATA_BUFF_SIZE);
        
    // Enable DMA streams 
    dma_stream_enable(sd_test_data.dma_stream);   // UART2 - Serial terminal 
    
    //==================================================

    //==================================================
    // Interrupts 

    // Initialize interrupt handler flags 
    int_handler_init();

    // Enable the interrupt handlers 
    nvic_config(USART2_IRQn, EXTI_PRIORITY_0);   // UART2 - Serial terminal (user input) 

    //==================================================

    //==================================================
    // SD card init 

    // SD card user initialization 
    sd_user_init(sd_test_data.spi, sd_test_data.gpio, sd_test_data.tim, GPIOX_PIN_12);
    
    //==================================================

    cmd_reset();
} 

//=======================================================================================


//=======================================================================================
// Test code 

void sd_test_app()
{
    // New serial terminal (user input) data received 
    if (handler_flags.usart2_flag)
    {
        handler_flags.usart2_flag = CLEAR_BIT;

        // Parse the new user message from the circular buffer into the data buffer 
        dma_cb_index(sd_test_data.dma_stream, &sd_test_data.dma_index, &sd_test_data.cb_index);
        cb_parse(sd_test_data.cb, &sd_test_data.cb_index, sd_test_data.data_in_buff);

        // Dispatch using function pointer 
        sd_test_data.state_func_ptr();
    }
}

//=======================================================================================


//=======================================================================================
// User command functions 

// Format the card 
void file_mkfs(void)
{
    uint8_t timer = FORMAT_ATTEMPTS;
    BYTE work[FORMAT_BUFF_SIZE];

    do
    {
        sd_test_data.fresult = f_mkfs("", NULL, work, sizeof(work));   // Use default config 
        (sd_test_data.fresult == FR_OK) ? feedback_display("SD Card formatted successfully.\r\n") : 
                                          feedback_display("Error formatting volume.\r\n");
    }
    while (sd_test_data.fresult != FR_OK && --timer);

    cmd_reset();
}


// Mount card 
void mount_card(void)
{
    sd_test_data.fresult = f_mount(&sd_test_data.file_sys, "", SD_MOUNT_NOW);

    if (sd_test_data.fresult == FR_OK)
    {
        feedback_display("\nMounted successfully. Volume type: ");

        // Check the volume type 
        switch (sd_get_card_type())
        {
            case SD_CT_MMC:
                feedback_display("MMC V3\r\n");
                break;
            case SD_CT_SDC1:
                feedback_display("SDC V1\r\n");
                break;
            case SD_CT_SDC2_BLOCK:
                feedback_display("SDC V2 block\r\n");
                break;
            case SD_CT_SDC2_BYTE:
                feedback_display("SDC V2 byte\r\n");
                break;
            default: 
                feedback_display("Unknown\r\n");
                break;
        }
    }
    else 
    {
        fault_display();
    }

    cmd_reset();
}


// Unmount card 
void unmount_card(void)
{
    // Unmount the volume and clear the initialization status so it can be re-mounted. 
    sd_test_data.fresult = f_unmount("");

    (sd_test_data.fresult == FR_OK) ? feedback_display("\nVolume unmounted successfully.\r\n") : 
                                      feedback_display("\nError in unmounting volume.\r\n");
    cmd_reset();
}


// Check card capacity 
void card_capacity(void)
{
    // These calcs assume 512 bytes/sector 

    FATFS *pfs;        // Pointer to file system object 
    DWORD fre_clust;   // Stores number of free clusters 

    // Check free space 
    f_getfree("", &fre_clust, &pfs);

    // Calculate the total space 
    DWORD total = (uint32_t)((pfs->n_fatent - 2) * pfs->csize / 2);
    number_display("\nVolume Total Size (KB): ", total);
    
    // Calculate the free space 
    DWORD free_space = (uint32_t)(fre_clust * pfs->csize / 2);
    number_display("Volume Free Space (KB): ", free_space);

    cmd_reset();
}


// Check existance of a file or sub-directory 
void file_check(void)
{
    if (sd_test_data.state_seq++ == 0)
    {
        feedback_display("\nFile or directory to check: ");
    }
    else
    {
        format_input((char *)sd_test_data.data_in_buff, &sd_test_data.data_in_num, FORMAT_FILE_STRING);

        // Check for the existance of the specified file or directory 
        sd_test_data.fresult = f_stat((TCHAR *)sd_test_data.data_in_buff, &sd_test_data.fno);

        switch(sd_test_data.fresult)
        {
            case FR_OK:
                string_display("\r\nExists: ", (char *)sd_test_data.data_in_buff);
                break;

            case FR_NO_FILE:
                string_display("\r\nDoes not exists: ", (char *)sd_test_data.data_in_buff);
                break;

            default:
                fault_display();
                break;
        }

        cmd_reset();
    }
}


// Remove files on card 
void file_remove(void)
{
    if (sd_test_data.state_seq++ == 0)
    {
        feedback_display("\nFile to remove: ");
    }
    else
    {
        format_input((char *)sd_test_data.data_in_buff, &sd_test_data.data_in_num, FORMAT_FILE_STRING);

        // Attempt to remove the specified file 
        sd_test_data.fresult = f_unlink((TCHAR *)sd_test_data.data_in_buff);
        (sd_test_data.fresult == FR_OK) ? string_display("\r\nSuccessfully removed: ", (char *)sd_test_data.data_in_buff) : 
                                          fault_display();
        cmd_reset();
    }
}


// Make a new directory 
void file_mkdir(void)
{
    if (sd_test_data.state_seq++ == 0)
    {
        feedback_display("\nDirectory: ");
    }
    else
    {
        format_input((char *)sd_test_data.data_in_buff, &sd_test_data.data_in_num, FORMAT_FILE_STRING);

        // Write to the file 
        sd_test_data.fresult = f_mkdir((TCHAR *)sd_test_data.data_in_buff);
        (sd_test_data.fresult == FR_OK) ? string_display("\r\nNew directory: ", (char *)sd_test_data.data_in_buff) : 
                                          fault_display();
        cmd_reset();
    }
}


// Change the current directory 
void file_chdir(void)
{
    if (sd_test_data.state_seq++ == 0)
    {
        feedback_display("\nDirectory to go to: ");
    }
    else
    {
        format_input((char *)sd_test_data.data_in_buff, &sd_test_data.data_in_num, FORMAT_FILE_STRING);

        // Change the directory 
        sd_test_data.fresult = f_chdir((TCHAR *)sd_test_data.data_in_buff);
        (sd_test_data.fresult == FR_OK) ? string_display("\r\nNow in: ", (char *)sd_test_data.data_in_buff) : 
                                          fault_display();
        cmd_reset();
    }
}


// Retrieve the current directory and drive 
void file_getcwd(void)
{
    if (sd_test_data.file_sys.fs_type == FS_EXFAT)
    {
        feedback_display("\r\nCan't fetch current directory of exFAT volumes. getcwd return root\r\n");
    }

    char path[DATA_BUFF_SIZE];

    // Get the current directory 
    sd_test_data.fresult = f_getcwd(path, DATA_BUFF_SIZE);
    (sd_test_data.fresult == FR_OK) ? string_display("\r\nCurrent directory: ", path) : 
                                      fault_display();
    cmd_reset();
}


// Open a directory 
void file_opendir(void)
{
    if (sd_test_data.state_seq++ == 0)
    {
        feedback_display("\nDirectory to open: ");
    }
    else
    {
        format_input((char *)sd_test_data.data_in_buff, &sd_test_data.data_in_num, FORMAT_FILE_STRING);

        // Open the specified directory 
        sd_test_data.fresult = f_opendir(&sd_test_data.dj, (TCHAR *)sd_test_data.data_in_buff);
        (sd_test_data.fresult == FR_OK) ? string_display("\r\nOpened: ", (char *)sd_test_data.data_in_buff) : 
                                          fault_display();
        cmd_reset();
    }
}


// Close an open directory 
void file_closedir(void)
{
    // Close the current directory 
    sd_test_data.fresult = f_closedir(&sd_test_data.dj);
    (sd_test_data.fresult == FR_OK) ? feedback_display("\r\nDirectory closed.\r\n") : 
                                      fault_display();
    cmd_reset();
}


// Check files on the card 
void file_find(void)
{
    if (sd_test_data.state_seq++ == 0)
    {
        feedback_display("\nPath: ");
    }
    else
    {
        format_input((char *)sd_test_data.data_in_buff, &sd_test_data.data_in_num, FORMAT_FILE_STRING);
        feedback_display("\nFiles in path: \r\n");

        // Start to search for files 
        sd_test_data.fresult = f_findfirst(&sd_test_data.dj, &sd_test_data.fno, sd_test_data.data_in_buff, "*");

        while ((sd_test_data.fresult == FR_OK) && sd_test_data.fno.fname[0])
        {
            uart_send_str(sd_test_data.uart, "\t- ");
            uart_send_str(sd_test_data.uart, sd_test_data.fno.fname);
            uart_send_new_line(sd_test_data.uart);
            sd_test_data.fresult = f_findnext(&sd_test_data.dj, &sd_test_data.fno);
        }

        f_closedir(&sd_test_data.dj);
        cmd_reset();
    }
}


// Open a file 
void file_open(void)
{
    static char path[DATA_BUFF_SIZE];

    if (sd_test_data.state_seq == 0)
    {
        feedback_display("\nFile to open: ");
        sd_test_data.state_seq++;
    }
    else if (sd_test_data.state_seq == 1)
    {
        format_input((char *)sd_test_data.data_in_buff, &sd_test_data.data_in_num, FORMAT_FILE_STRING);
        strcpy(path, (char *)sd_test_data.data_in_buff);
        feedback_display("\nAccess mode: ");
        sd_test_data.state_seq++;
    }
    else
    {
        format_input((char *)sd_test_data.data_in_buff, &sd_test_data.data_in_num, FORMAT_FILE_MODE);

        // Open a file (and create if it doesn't exist) 
        sd_test_data.fresult = f_open(&sd_test_data.file, path, (BYTE)sd_test_data.data_in_num);
        
        if (sd_test_data.fresult == FR_OK)
        {
            string_display("\r\nOpened: ", path);
            string_display("With permissions: ", (char *)sd_test_data.data_in_buff);
        }
        else
        {
            fault_display();
        }

        cmd_reset();
    }
}


// Close the open file 
void file_close(void)
{
    // Close the open file 
    sd_test_data.fresult = f_close(&sd_test_data.file);
    (sd_test_data.fresult == FR_OK) ? feedback_display("\r\nFile closed.\r\n") : 
                                      fault_display();
    cmd_reset();
}


// Read from an open file using using f_read 
void file_read(void)
{
    if (sd_test_data.state_seq++ == 0)
    {
        feedback_display("\nRead size (bytes): ");
    }
    else
    {
        char file_data[DATA_BUFF_SIZE];
        format_input((char *)sd_test_data.data_in_buff, &sd_test_data.data_in_num, FORMAT_FILE_NUM);

        // Read from the file 
        sd_test_data.fresult = f_read(&sd_test_data.file, 
                                      (void *)file_data, 
                                      sd_test_data.data_in_num, 
                                      &sd_test_data.br);
        (sd_test_data.fresult == FR_OK) ? string_display("\r\nFile data: ", file_data) : 
                                          fault_display();
        cmd_reset();
    }
}


// Write to an open file using f_write 
void file_write(void)
{
    if (sd_test_data.state_seq++ == 0)
    {
        feedback_display("\nFile data: ");
    }
    else
    {
        format_input((char *)sd_test_data.data_in_buff, &sd_test_data.data_in_num, FORMAT_FILE_STRING);

        // Write to the file 
        sd_test_data.fresult = f_write(&sd_test_data.file, 
                                       (void *)sd_test_data.data_in_buff, 
                                       strlen(sd_test_data.data_in_buff), 
                                       &sd_test_data.bw);
        (sd_test_data.fresult == FR_OK) ? string_display("\r\nFile data: ", (char *)sd_test_data.data_in_buff) : 
                                          fault_display();
        cmd_reset();
    }
}


// Navigate the file 
void file_seek(void)
{
    if (sd_test_data.state_seq++ == 0)
    {
        feedback_display("\nFile position: ");
    }
    else
    {
        format_input((char *)sd_test_data.data_in_buff, &sd_test_data.data_in_num, FORMAT_FILE_NUM);

        // Move to the specified position in the file 
        sd_test_data.fresult = f_lseek(&sd_test_data.file, sd_test_data.data_in_num);
        (sd_test_data.fresult == FR_OK) ? number_display("\r\nNew file position: ", sd_test_data.data_in_num) : 
                                          fault_display();
        cmd_reset();
    }
}


// Navigate to the beginning of the file 
void file_rewind(void)
{
    sd_test_data.fresult = f_lseek(&sd_test_data.file, RESET_ZERO);
    (sd_test_data.fresult == FR_OK) ? feedback_display("\r\nNow at the beginning of the file.\r\n") : 
                                      fault_display();
    cmd_reset();
}


// Navigate to the end of the file 
void file_fast_fwd(void)
{
    sd_test_data.fresult = f_lseek(&sd_test_data.file, f_size(&sd_test_data.file));
    (sd_test_data.fresult == FR_OK) ? feedback_display("\r\nNow at the end of the file.\r\n") : 
                                      fault_display();
    cmd_reset();
}


// Read from an open file using f_gets 
void file_gets(void)
{
    char file_data[DATA_BUFF_SIZE];

    // Read from the file 
    TCHAR *data_buff = f_gets(file_data, DATA_BUFF_SIZE, &sd_test_data.file);

    if (data_buff != NULL)
    {
        string_display("\r\nFile data: ", file_data);
    }
    else
    {
        if (f_eof(&sd_test_data.file) != 0)
        {
            feedback_display("\r\nEnd of file reached.\r\n");
        }
        else if (f_error(&sd_test_data.file) != 0)
        {
            feedback_display("\r\nHard file error occured.\r\n");
        }
        else
        {
            feedback_display("\r\nUnknown error.\r\n");
        }
    }

    cmd_reset();
}


// Write to an open file using f_puts 
void file_puts(void)
{
    if (sd_test_data.state_seq++ == 0)
    {
        feedback_display("\nFile data: ");
    }
    else
    {
        format_input((char *)sd_test_data.data_in_buff, &sd_test_data.data_in_num, FORMAT_FILE_STRING);

        // Format the input to end with a line break. This separates the provided data 
        // from "f_puts" into their own lines which also allows for each line to be 
        // retrieved individually when using "f_gets". 
        uint8_t buff_size = DATA_BUFF_SIZE + 2;
        char line_of_data[buff_size];
        snprintf(line_of_data, buff_size, "%s\r\n", (char *)sd_test_data.data_in_buff);

        // Write a string to the file 
        int num_chars = f_puts(line_of_data, &sd_test_data.file);
        (num_chars >= 0) ? number_display("\r\nData written to file. Characters written: ", (DWORD)num_chars) : 
                           feedback_display("Write failed.");

        cmd_reset();
    }
}

//=======================================================================================


//=======================================================================================
// Helper functions 

// Select state based on user input 
void cmd_select(void)
{
    uint8_t index = CLEAR;
    format_input((char *)sd_test_data.data_in_buff, &sd_test_data.data_in_num, FORMAT_FILE_STRING);

    // Compare the input to the defined user commands 
    do
    {
        if (str_compare((char *)sd_test_data.data_in_buff, cmd_table[index].user_cmds, BYTE_0)) 
        {
            sd_test_data.state_func_ptr = cmd_table[index].sd_func_ptrs_t;
            sd_test_data.state_func_ptr();
            break;
        }
    }
    while (++index < NUM_USER_CMDS);

    if (index >= NUM_USER_CMDS)
    {
        cmd_reset();
    }
}


// Return to default state at the end of the command dispatch 
void cmd_reset(void)
{
    sd_test_data.state_seq = CLEAR;
    sd_test_data.state_func_ptr = &cmd_select;
    feedback_display("\r\n>>> ");
}


// Display a formatting string with a string argument for the user to see 
void string_display(
    const char *string,
    const char *str_arg)
{
    const char str_format[] = "%s\r\n";
    snprintf(sd_test_data.data_out_buff, DATA_BUFF_SIZE, str_format, str_arg);
    feedback_display(string);
    feedback_display(sd_test_data.data_out_buff);
}


// Display a formatting string with a number argument for the user to see 
void number_display(
    const char *string,
    const DWORD num_arg)
{
    const char str_format[] = "%lu\r\n";
    snprintf(sd_test_data.data_out_buff, DATA_BUFF_SIZE, str_format, num_arg);
    feedback_display(string);
    feedback_display(sd_test_data.data_out_buff);
}


// Display the error code related to the FATFS operation 
void fault_display(void)
{
    snprintf(sd_test_data.data_out_buff,
             DATA_BUFF_SIZE,
             "\r\nProblem occurred --> FRESULT == %u\r\n",
             (uint8_t)sd_test_data.fresult);
    feedback_display(sd_test_data.data_out_buff);
}


// Display a string for the user to see 
void feedback_display(const char *string)
{
    uart_send_str(sd_test_data.uart, string);
}


// Format user input 
uint8_t format_input(
    char *buff, 
    DWORD *data, 
    format_user_input_t op)
{
    uint8_t result = FALSE;

    if ((buff == NULL) || (data == NULL)) 
    {
        return result;
    }

    if (op == FORMAT_FILE_STRING)
    {
        // Replace carriage return from input with a null character 
        for (uint8_t i = CLEAR; i < CMD_SIZE; i++)
        {
            if (*buff == CR_CHAR)
            {
                *buff = NULL_CHAR;
                break;
            }
            buff++;
        }

        result = TRUE;
    }
    else if (op == FORMAT_FILE_MODE)
    {
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
                else
                {
                    break;
                }

                *data |= (nibble << SHIFT_4*(3-i));

                if (i == 3)
                {
                    result = TRUE;
                }
            }
        }
    }
    else if (op == FORMAT_FILE_NUM)
    {
        char *buff_copy = buff;

        while (*buff_copy != CR_CHAR) 
        {
            if (!((*buff_copy >= ZERO_CHAR) && (*buff_copy <= NINE_CHAR)))
            {
                break;
            }
            buff_copy++;
        }

        if (*buff_copy == CR_CHAR) 
        {
            *data = (DWORD)atoi(buff);
            result = TRUE;
        }
    }

    return result;
}

//=======================================================================================
