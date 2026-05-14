#ifndef __OLED_H
#define __OLED_H

#include "main.h"
#include <stdlib.h>

// ---------- 引脚定义 ----------
// 根据实际接线修改（使用PA0~PA4）
#define OLED_SCL_GPIO_PIN     GPIO_PIN_5
#define OLED_SDA_GPIO_PIN     GPIO_PIN_7
#define OLED_RES_GPIO_PIN     GPIO_PIN_4
#define OLED_DC_GPIO_PIN      GPIO_PIN_3
#define OLED_CS_GPIO_PIN      GPIO_PIN_2
#define OLED_GPIO_PORT        GPIOA

// 宏定义：置位/复位（HAL 风格）
#define OLED_SCL_Set()   HAL_GPIO_WritePin(OLED_GPIO_PORT, OLED_SCL_GPIO_PIN, GPIO_PIN_SET)
#define OLED_SCL_Clr()   HAL_GPIO_WritePin(OLED_GPIO_PORT, OLED_SCL_GPIO_PIN, GPIO_PIN_RESET)
#define OLED_SDA_Set()   HAL_GPIO_WritePin(OLED_GPIO_PORT, OLED_SDA_GPIO_PIN, GPIO_PIN_SET)
#define OLED_SDA_Clr()   HAL_GPIO_WritePin(OLED_GPIO_PORT, OLED_SDA_GPIO_PIN, GPIO_PIN_RESET)
#define OLED_RES_Set()   HAL_GPIO_WritePin(OLED_GPIO_PORT, OLED_RES_GPIO_PIN, GPIO_PIN_SET)
#define OLED_RES_Clr()   HAL_GPIO_WritePin(OLED_GPIO_PORT, OLED_RES_GPIO_PIN, GPIO_PIN_RESET)
#define OLED_DC_Set()    HAL_GPIO_WritePin(OLED_GPIO_PORT, OLED_DC_GPIO_PIN, GPIO_PIN_SET)
#define OLED_DC_Clr()    HAL_GPIO_WritePin(OLED_GPIO_PORT, OLED_DC_GPIO_PIN, GPIO_PIN_RESET)
#define OLED_CS_Set()    HAL_GPIO_WritePin(OLED_GPIO_PORT, OLED_CS_GPIO_PIN, GPIO_PIN_SET)
#define OLED_CS_Clr()    HAL_GPIO_WritePin(OLED_GPIO_PORT, OLED_CS_GPIO_PIN, GPIO_PIN_RESET)

#define OLED_CMD         0
#define OLED_DATA        1

// 外部函数
void OLED_Init(void);
void OLED_WR_Byte(uint8_t dat, uint8_t cmd);
void OLED_Refresh(void);
void OLED_Clear(void);
void OLED_ColorTurn(uint8_t i);
void OLED_DisplayTurn(uint8_t i);
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t t);
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t size1, uint8_t mode);
void OLED_ShowString(uint8_t x, uint8_t y, uint8_t *chr, uint8_t size1, uint8_t mode);
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size1, uint8_t mode);
void OLED_ShowChinese(uint8_t x, uint8_t y, uint8_t num, uint8_t size1, uint8_t mode);
void OLED_ShowPicture(uint8_t x, uint8_t y, uint8_t sizex, uint8_t sizey, uint8_t BMP[], uint8_t mode);
void OLED_ScrollDisplay(uint8_t num, uint8_t space, uint8_t mode);

#endif