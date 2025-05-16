#include "sBSP_GPIO.h"


#include "defines.h"


void sBSP_GPIO_InitAll(){
    /*LED*/
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitStruct.Pin = LED_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);


}

void sBSP_GPIO_SetLED(bool level){
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, level?GPIO_PIN_SET:GPIO_PIN_RESET);
}




