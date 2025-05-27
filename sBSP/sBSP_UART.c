#include "sBSP_UART.h"

#include "stdio.h"
#include "stdarg.h"
#include "string.h"

#include "defines.h"

/*DR1串口 UART1*/
UART_HandleTypeDef uart1;
static const uint32_t uart1_blocking_ms = 100;
// printf格式化
static char uart1_fmt_buf[512];
// 串口接收缓冲
static char uart1_recv_buf[512];
// 保存用户传入的接收完成回调
static sBSP_UART_RecvEndCb_t uart1_recv_end_cb;

/*Debug串口 UART2*/
UART_HandleTypeDef uart2;
static const uint32_t uart2_blocking_ms = 100;
// printf格式化
static char uart2_fmt_buf[512];
// 串口接收缓冲
static char uart2_recv_buf[512];
// 保存用户传入的接收完成回调
static sBSP_UART_RecvEndCb_t uart2_recv_end_cb;
// //DMA
// DMA_HandleTypeDef hdma_usart1_rx;
// DMA_HandleTypeDef hdma_usart1_tx;

int sBSP_UART_Debug_Init(uint32_t bandrate) {
    uart2.Instance          = USART2;
    uart2.Init.BaudRate     = bandrate;
    uart2.Init.WordLength   = UART_WORDLENGTH_8B;
    uart2.Init.StopBits     = UART_STOPBITS_1;
    uart2.Init.Parity       = UART_PARITY_NONE;
    uart2.Init.Mode         = UART_MODE_TX_RX;
    uart2.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    uart2.Init.OverSampling = UART_OVERSAMPLING_16;

    if(HAL_UART_Init(&uart2) != HAL_OK) {
        return -1;
    }
    return 0;
}

void sBSP_UART_Debug_Printf(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsprintf(uart2_fmt_buf, fmt, ap);
    va_end(ap);

    HAL_UART_Transmit(&uart2, (uint8_t*)uart2_fmt_buf, (uint16_t)(strlen(uart2_fmt_buf)), uart2_blocking_ms);
}

void sBSP_UART_Debug_SendByte(uint8_t byte) {
    HAL_UART_Transmit(&uart2, &byte, 1, uart2_blocking_ms);
}

void sBSP_UART_Debug_SendBytes(uint8_t* pData, uint16_t length) {
    HAL_UART_Transmit(&uart2, pData, length, uart2_blocking_ms);
}

void sBSP_UART_Debug_RecvBegin(sBSP_UART_RecvEndCb_t recv_cb) {
    assert_param(recv_cb != NULL);
    uart2_recv_end_cb = recv_cb;

    if(HAL_UARTEx_ReceiveToIdle_IT(&uart2, (uint8_t*)uart2_recv_buf, sizeof(uart2_recv_buf)) != HAL_OK) {
        sBSP_UART_Debug_Printf("串口2:空闲中断IT接收出错");
    }

    // if(HAL_UARTEx_ReceiveToIdle_DMA(&uart2, (uint8_t*)uart2_recv_buf, sizeof(uart2_recv_buf)) != HAL_OK){
    //     sBSP_UART_Debug_Printf("串口2:空闲中断DMA接收出错");
    // }
}

int sBSP_UART_DR1_Init(uint32_t bandrate) {
    uart1.Instance          = USART1;
    uart1.Init.BaudRate     = bandrate;
    uart1.Init.WordLength   = UART_WORDLENGTH_8B;
    uart1.Init.StopBits     = UART_STOPBITS_1;
    uart1.Init.Parity       = UART_PARITY_NONE;
    uart1.Init.Mode         = UART_MODE_TX_RX;
    uart1.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    uart1.Init.OverSampling = UART_OVERSAMPLING_16;
    if(HAL_UART_Init(&uart1) != HAL_OK) {
        return -1;
    }
    return 0;
}

void sBSP_UART_DR1_RecvBegin(sBSP_UART_RecvEndCb_t recv_cb) {
    assert_param(recv_cb != NULL);
    uart1_recv_end_cb = recv_cb;

    if(HAL_UARTEx_ReceiveToIdle_IT(&uart1, (uint8_t*)uart1_recv_buf, sizeof(uart1_recv_buf)) != HAL_OK) {
        sBSP_UART_Debug_Printf("串口1:空闲中断IT接收出错");
    }

    // if(HAL_UARTEx_ReceiveToIdle_DMA(&uart2, (uint8_t*)uart2_recv_buf, sizeof(uart2_recv_buf)) != HAL_OK){
    //     sBSP_UART_Debug_Printf("串口1:空闲中断DMA接收出错");
    // }
}


void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef* huart, uint16_t Size) {
    if(huart->Instance == USART1) {
        assert_param(uart1_recv_end_cb != NULL);
        uart1_recv_end_cb(uart1_recv_buf, Size);
    } else if(huart->Instance == USART2) {
        assert_param(uart2_recv_end_cb != NULL);
        uart2_recv_end_cb(uart2_recv_buf, Size);
    }
}

// override the weak definitions provided by the HAL lib
void HAL_UART_MspInit(UART_HandleTypeDef* huart) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if(huart->Instance == USART1) {
        __HAL_RCC_USART1_CLK_ENABLE();

        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**USART1 GPIO Configuration
        PA10     ------> USART1_RX
        PA15     ------> USART1_TX
        */
        GPIO_InitStruct.Pin       = DR1_RX_Pin | DR1_TX_Pin;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_PULLUP;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(USART1_IRQn);
    } else if(huart->Instance == USART2) {
        __HAL_RCC_USART2_CLK_ENABLE();

        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**USART2 GPIO Configuration
        PA2     ------> USART2_TX
        PA3     ------> USART2_RX
        */
        GPIO_InitStruct.Pin       = DEBUG_TX_Pin | DEBUG_RX_Pin;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_PULLUP;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        
        HAL_NVIC_SetPriority(USART2_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(USART2_IRQn);
    }
}
