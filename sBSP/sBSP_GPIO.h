#pragma once
#ifdef __cplusplus
extern "C" {
#endif


#include "stm32f4xx_hal.h"
#include "stdbool.h"




void sBSP_GPIO_InitAll();

void sBSP_GPIO_SetLED(bool level);






#ifdef __cplusplus
}
#endif

