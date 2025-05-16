#pragma once
#ifdef __cplusplus
extern "C" {
#endif


#include "stm32f4xx_hal.h"
#include "stdbool.h"



typedef void(*sBSP_UART_RecvEndCb_t)(char* pReciData,uint16_t length);

int sBSP_UART_Debug_Init(uint32_t bandrate);
void sBSP_UART_Debug_RecvBegin(sBSP_UART_RecvEndCb_t recv_cb);
void sBSP_UART_Debug_Printf(const char *fmt,...);
void sBSP_UART_Debug_SendBytes(uint8_t* pData,uint16_t length);


int sBSP_UART_DR1_Init(uint32_t bandrate);






#ifdef __cplusplus
}
#endif

