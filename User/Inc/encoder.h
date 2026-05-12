#ifndef __ENCODER_H
#define __ENCODER_H

#include <stdint.h>
#include <stdbool.h>

// 初始化编码器（TIM3，通道1和2）
void encoder_init(void);

// 获取当前编码器计数值（相对上次清零后的增量，有符号）
int16_t encoder_get_count(void);

// 获取自初始化以来的总计数（有符号，可能会溢出）
int32_t encoder_get_total_count(void);

// 清零当前计数值（不影响总计数）
void encoder_reset_count(void);

// 获取旋转方向：1=右旋，-1=左旋，0=不动（基于最近一次读取的变化）
int8_t encoder_get_direction(void);

// 获取编码器原始定时器计数寄存器值（用于调试）
uint16_t encoder_get_raw_tim_count(void);

#endif