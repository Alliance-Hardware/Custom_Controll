// ws2812b.h
#ifndef __WS2812B_H
#define __WS2812B_H

#include "main.h"
#include "tim.h" // 引用定时器句柄

// 配置
#define NUM_LEDS    9           // 灯珠数量
#define TIM_HTIM    &htim1      // 定时器句柄
#define TIM_CHANNEL TIM_CHANNEL_1 // PWM通道

// 核心时序参数 (根据之前计算)
#define WS_BIT0         30          // 0码高电平计数值 (~0.41us)
#define WS_BIT1         60          // 1码高电平计数值 (~0.83us)
#define WS_RESET_CNT    64          // 复位信号低电平周期 (~80us)

// 函数声明
void ws2812b_init(void);
void ws2812b_set_color(uint8_t led_index, uint8_t r, uint8_t g, uint8_t b);
void ws2812b_show(void);
void ws2812b_clear_all(void);

#endif