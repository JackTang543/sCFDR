#include "main.hpp"



#include "fatfs\ff.h"
#include "stdio.h"
#include "string.h"

FATFS fs;
FIL fil;
char buffer[100];
UINT byteswritten;
FRESULT res;

#include "sightseerUtils/sUtils.h"

int main(){
    HAL_Init();
    sBSP_RCC_Init();

    sBSP_UART_Debug_Init(115200);
    sBSP_UART_DR1_Init(115200);



    sBSP_GPIO_InitAll();
    sBSP_SDIO_Init();

    extern SD_HandleTypeDef hsd;

    sBSP_UART_Debug_Printf("Micro SD Card Test...\r\n");
/* 检测SD卡是否正常（处于数据传输模式的传输状态） */
if(HAL_SD_GetCardState(&hsd) == HAL_SD_CARD_TRANSFER)
{      
    sBSP_UART_Debug_Printf("Initialize SD card successfully!\r\n");
    // 打印SD卡基本信息
    sBSP_UART_Debug_Printf(" SD card information! \r\n");
    sBSP_UART_Debug_Printf(" CardCapacity  : %llu \r\n", (unsigned long long)hsd.SdCard.BlockSize * hsd.SdCard.BlockNbr);// 显示容量
    sBSP_UART_Debug_Printf(" CardBlockSize : %d \r\n", hsd.SdCard.BlockSize);   // 块大小
    sBSP_UART_Debug_Printf(" LogBlockNbr   : %d \r\n", hsd.SdCard.LogBlockNbr);	// 逻辑块数量
	sBSP_UART_Debug_Printf(" LogBlockSize  : %d \r\n", hsd.SdCard.LogBlockSize);// 逻辑块大小
    sBSP_UART_Debug_Printf(" RCA           : %d \r\n", hsd.SdCard.RelCardAdd);  // 卡相对地址
    sBSP_UART_Debug_Printf(" CardType      : %d \r\n", hsd.SdCard.CardType);    // 卡类型
    // 读取并打印SD卡的CID信息
    HAL_SD_CardCIDTypeDef sdcard_cid;
    HAL_SD_GetCardCID(&hsd,&sdcard_cid);
    sBSP_UART_Debug_Printf(" ManufacturerID: %d \r\n",sdcard_cid.ManufacturerID);
}
else
{
    sBSP_UART_Debug_Printf("SD card init fail!\r\n" );
}


// //写入512bytes 0xAB,从逻辑块0开始,然后读出
// uint8_t buffer[512];
// memset(buffer, 0xAD, sizeof(buffer));

// {
//     HAL_SD_CardStateTypeDef status = HAL_SD_GetCardState(&hsd);
//     if(status != HAL_SD_CARD_TRANSFER){
//         sBSP_UART_Debug_Printf("write SD卡未准备好,状态=%d\r\n",status);
//     }
// }

// HAL_SD_Erase(&hsd, 1, 1);
// {
//     HAL_SD_CardStateTypeDef status = HAL_SD_GetCardState(&hsd);
//     if(status != HAL_SD_CARD_TRANSFER){
//         sBSP_UART_Debug_Printf("erase SD卡未准备好,状态=%d\r\n",status);
//         while(status != HAL_SD_CARD_TRANSFER){
//             status = HAL_SD_GetCardState(&hsd);
//             sBSP_UART_Debug_Printf("erase 等待SD卡准备好,状态=%d\r\n",status);
//         }
//     }
// }
// HAL_SD_WriteBlocks(&hsd, buffer, 1,1,100);
// memset(buffer, 0, sizeof(buffer));

// {
//     HAL_SD_CardStateTypeDef status = HAL_SD_GetCardState(&hsd);
//     if(status != HAL_SD_CARD_TRANSFER){
//         sBSP_UART_Debug_Printf("read SD卡未准备好,状态=%d\r\n",status);
//         while(status != HAL_SD_CARD_TRANSFER){
//             status = HAL_SD_GetCardState(&hsd);
//             sBSP_UART_Debug_Printf("等待SD卡准备好,状态=%d\r\n",status);
//         }
//     }
// }
// HAL_SD_ReadBlocks(&hsd, buffer, 1,1,100);
// sBSP_UART_Debug_Printf("Read data from SD card:\r\n");
// for(int i = 0; i < sizeof(buffer); i++){
//     sBSP_UART_Debug_Printf("%02X ", buffer[i]);
// }


    // 写入Fatfs,一个txt文档:test.txt,内容:Hello,this is a test file.
    FATFS fs;
    FIL fil;
    char buffer[100];
    UINT byteswritten;
    FRESULT res;

    res = f_mount(&fs,"0:",1);

    if(res != FR_OK){
        log_error("挂载文件系统失败,错误码:%d",res);
    }else{
        log_info("挂载文件系统成功");
    }

    log_info("格式化文件系统中...");
        res = f_mkfs("0:",0,malloc(0x4000),0x4000);
        if(res != FR_OK){
            log_error("格式化文件系统失败,错误码:%d",res);
        }else{
            log_info("格式化文件系统成功");
        }


    res = f_open(&fil, "test.txt", FA_CREATE_ALWAYS | FA_WRITE);
    if(res != FR_OK){
        sBSP_UART_Debug_Printf("Open file failed!\r\n");
    }else{
        sBSP_UART_Debug_Printf("Open file success!\r\n");
    }
    sprintf(buffer, "Hello, this is a test file.\r\n");
    res = f_write(&fil, buffer, strlen(buffer), &byteswritten);
    if(res != FR_OK){
        sBSP_UART_Debug_Printf("Write file failed!\r\n");
    }else{
        sBSP_UART_Debug_Printf("Write file success!\r\n");
    }
    res = f_close(&fil);
    if(res != FR_OK){
        sBSP_UART_Debug_Printf("Close file failed!\r\n");
    }else{
        sBSP_UART_Debug_Printf("Close file success!\r\n");
    }





    while(1){
        //sBSP_UART_Debug_Printf("Hello World!\r\n");
        sBSP_GPIO_SetLED(1);
        HAL_Delay(100);
        sBSP_GPIO_SetLED(0);
        HAL_Delay(100);
        
    }
}












