#ifndef __HX711_H__
#define __HX711_H__

#include "proj.h"

#define HX711_SCL_0_H()         HAL_GPIO_WritePin(HX711_CLK_0_GPIO_Port, HX711_CLK_0_Pin, GPIO_PIN_SET)
#define HX711_SCL_0_L()         HAL_GPIO_WritePin(HX711_CLK_0_GPIO_Port, HX711_CLK_0_Pin, GPIO_PIN_RESET)
#define HX711_SDA_0_R()         HAL_GPIO_ReadPin(HX711_DIO_0_GPIO_Port, HX711_DIO_0_Pin)

#define HX711_SCL_1_H()         HAL_GPIO_WritePin(HX711_CLK_1_GPIO_Port, HX711_CLK_1_Pin, GPIO_PIN_SET)
#define HX711_SCL_1_L()         HAL_GPIO_WritePin(HX711_CLK_1_GPIO_Port, HX711_CLK_1_Pin, GPIO_PIN_RESET)
#define HX711_SDA_1_R()         HAL_GPIO_ReadPin(HX711_DIO_1_GPIO_Port, HX711_DIO_1_Pin)

extern u32 Hx711_Read0(void);
extern u32 Hx711_Read1(void);



#endif
