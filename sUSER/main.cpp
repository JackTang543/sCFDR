#include "main.hpp"

#include "fatfs\ff.h"
#include "stdio.h"
#include "string.h"

FATFS fs;
FIL fil;
char buffer[512];
UINT byteswritten;
FRESULT res;

#include "sightseerUtils/sUtils.h"





void uart_recied(char* pReciData,uint16_t length){
    FRESULT fr;
    DWORD   fsz;
    UINT    bw;

    res = f_open(&fil, "dr1_recoder.txt",FA_WRITE | FA_OPEN_APPEND);
    if(res != FR_OK) {
        sBSP_UART_Debug_Printf("Open file failed!\r\n");
    } else {
        sBSP_UART_Debug_Printf("Open file success!\r\n");
    }


    /* ---------- 1. 检查文件尾是否已有换行 ---------- */
    fsz = f_size(&fil);

    if (fsz > 0) {
        char last;
        /* 把文件指针挪到最后 1 字节 */
        fr = f_lseek(&fil, fsz - 1);
        if (fr == FR_OK) {
            f_read(&fil, &last, 1, &bw);
            if (bw == 1 && last != '\n') {
                /* 尾部没有 \n，则补一个（Win 下可写 "\r\n"） */
                f_lseek(&fil, f_size(&fil));     // 回到文件末尾
                f_write(&fil, "\r\n", 2, &bw);
            }
        }
    }

    /* ---------- 2. 写入串口缓冲 ---------- */
    f_lseek(&fil, f_size(&fil));           // 确保在 EOF
    f_write(&fil, pReciData, length, &bw);

    /* ---------- 3. 立即刷新或延迟刷新 ---------- */
    f_sync(&fil);    // 或者你每 N 次 / N ms 调用一次

    f_close(&fil);

    // sBSP_UART_DR1_RecvBegin(uart_recied);
    sBSP_UART_Debug_RecvBegin(uart_recied);



}






// 定义一个 uint8_t 类型的环形缓冲，容量 64
static RingBuffer<uint8_t, 256> ring;


extern"C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
    if(huart->Instance == USART2) {
        // 读取接收到的数据
        uint8_t data;
        HAL_UART_Receive(huart, &data, 1, HAL_MAX_DELAY);

        // 将数据写入环形缓冲
        if (!ring.push(data)) {
            // 缓冲区满时可以选择丢弃数据或处理溢出
            sBSP_UART_Debug_Printf("Ring buffer full, data lost!\r\n");
        }else{
            // 如果需要，可以在这里处理接收到的数据
            // 例如，打印到调试串口
            sBSP_UART_Debug_Printf("Received: %c\r\n", data);
        }
    }
}

extern UART_HandleTypeDef uart2;

int main() {
    HAL_Init();
    sBSP_RCC_Init();

    // sBSP_UART_Debug_Init(115200);
    // sBSP_UART_DR1_Init(115200);

    //打开USART2时钟
    SET_BIT(RCC->APB1ENR, RCC_APB1ENR_USART2EN);
    

    sBSP_GPIO_InitAll();
    // sBSP_SDIO_Init();


    log_printfln("Hello World! This is a test for RingBuffer and UART.\r\n");
    

    // HAL_UART_Receive_IT(&uart2, (uint8_t*)&ring.front(), 1); // 启动串口接收中断

    // extern SD_HandleTypeDef hsd;

    // sBSP_UART_Debug_Printf("Micro SD Card Test...\r\n");
    // /* 检测SD卡是否正常（处于数据传输模式的传输状态） */
    // if(HAL_SD_GetCardState(&hsd) == HAL_SD_CARD_TRANSFER) {
    //     sBSP_UART_Debug_Printf("Initialize SD card successfully!\r\n");
    //     // 打印SD卡基本信息
    //     sBSP_UART_Debug_Printf(" SD card information! \r\n");
    //     sBSP_UART_Debug_Printf(" CardCapacity  : %llu \r\n", (unsigned long long)hsd.SdCard.BlockSize * hsd.SdCard.BlockNbr); // 显示容量
    //     sBSP_UART_Debug_Printf(" CardBlockSize : %d \r\n", hsd.SdCard.BlockSize);                                             // 块大小
    //     sBSP_UART_Debug_Printf(" LogBlockNbr   : %d \r\n", hsd.SdCard.LogBlockNbr);                                           // 逻辑块数量
    //     sBSP_UART_Debug_Printf(" LogBlockSize  : %d \r\n", hsd.SdCard.LogBlockSize);                                          // 逻辑块大小
    //     sBSP_UART_Debug_Printf(" RCA           : %d \r\n", hsd.SdCard.RelCardAdd);                                            // 卡相对地址
    //     sBSP_UART_Debug_Printf(" CardType      : %d \r\n", hsd.SdCard.CardType);                                              // 卡类型
    //     // 读取并打印SD卡的CID信息
    //     HAL_SD_CardCIDTypeDef sdcard_cid;
    //     HAL_SD_GetCardCID(&hsd, &sdcard_cid);
    //     sBSP_UART_Debug_Printf(" ManufacturerID: %d \r\n", sdcard_cid.ManufacturerID);
    // } else {
    //     sBSP_UART_Debug_Printf("SD card init fail!\r\n");
    // }

    // // 写入Fatfs,一个txt文档:test.txt,内容:Hello,this is a test file.
    // FATFS fs;
    // FIL fil;
    // char buffer[100];
    // UINT byteswritten;
    // FRESULT res;

    // res = f_mount(&fs, "0:", 1);

    // if(res != FR_OK) {
    //     log_error("挂载文件系统失败,错误码:%d", res);
    //     log_info("格式化文件系统中...");
    //     void* work = malloc(0x1000);
    //     res = f_mkfs("0:", 0, work, 0x1000);
    //     if(res != FR_OK) {
    //         log_error("格式化文件系统失败,错误码:%d", res);
    //     } else {
    //         log_info("格式化文件系统成功");
    //     }
    //     free(work);
    // } else {
    //     log_info("挂载文件系统成功");
    // }


    // res = f_open(&fil, "test.txt", FA_CREATE_ALWAYS | FA_WRITE);
    // if(res != FR_OK) {
    //     sBSP_UART_Debug_Printf("Open file failed!\r\n");
    // } else {
    //     sBSP_UART_Debug_Printf("Open file success!\r\n");
    // }
    // sprintf(buffer, "Hello, this is a test file.\r\n");
    // res = f_write(&fil, buffer, strlen(buffer), &byteswritten);
    // if(res != FR_OK) {
    //     sBSP_UART_Debug_Printf("Write file failed!\r\n");
    // } else {
    //     sBSP_UART_Debug_Printf("Write file success!\r\n");
    // }
    // res = f_close(&fil);
    // if(res != FR_OK) {
    //     sBSP_UART_Debug_Printf("Close file failed!\r\n");
    // } else {
    //     sBSP_UART_Debug_Printf("Close file success!\r\n");
    // }

    


    // // sBSP_UART_DR1_RecvBegin(uart_recied);
    // sBSP_UART_Debug_RecvBegin(uart_recied);


    while(1) {
        // sBSP_UART_Debug_Printf("Hello World!\r\n");
        sBSP_GPIO_SetLED(1);
        HAL_Delay(100);
        sBSP_GPIO_SetLED(0);
        HAL_Delay(100);

        if(ring.size() > 5){
            uint8_t data;
            // 从环形缓冲中读取数据
            while (ring.pop(data)) {
                // 处理读取到的数据
                sBSP_UART_Debug_Printf("%s", data);
            }
        }
    }
}
