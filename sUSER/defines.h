#pragma once
#ifdef __cplusplus
extern "C" {
#endif


//attribute:手动设置对齐
#define ALIGN_ATTR(__BYTES) __attribute__((aligned(__BYTES)))
//中断服务函数标识
#define ISR_ATTR       __attribute__((used))

#define APPNAME                        "sCFDR"
#define HARDWARE_VERSION               "V1.0"
#define SOFTWARE_VERSION               "V1.0"


#define ADC_VIN_Pin GPIO_PIN_0
#define ADC_VIN_GPIO_Port GPIOA
#define ADC_3V3_Pin GPIO_PIN_1
#define ADC_3V3_GPIO_Port GPIOA
#define DEBUG_TX_Pin GPIO_PIN_2
#define DEBUG_TX_GPIO_Port GPIOA
#define DEBUG_RX_Pin GPIO_PIN_3
#define DEBUG_RX_GPIO_Port GPIOA
#define TF_DETECT_Pin GPIO_PIN_4
#define TF_DETECT_GPIO_Port GPIOA
#define KEY_Pin GPIO_PIN_0
#define KEY_GPIO_Port GPIOB
#define LED_Pin GPIO_PIN_1
#define LED_GPIO_Port GPIOB
#define DR1_RX_Pin GPIO_PIN_10
#define DR1_RX_GPIO_Port GPIOA
#define DR2_TX_Pin GPIO_PIN_11
#define DR2_TX_GPIO_Port GPIOA
#define DR2_RX_Pin GPIO_PIN_12
#define DR2_RX_GPIO_Port GPIOA
#define DR1_TX_Pin GPIO_PIN_15
#define DR1_TX_GPIO_Port GPIOA









#ifdef __cplusplus
}
#endif