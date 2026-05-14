#include "encoder.h"
#include "stm32f1xx_hal.h"
#include "tim.h"
#include <stdint.h>

static int32_t total_overflow = 0;   // 处理16位计数器溢出
static int16_t last_count = 0;       // 上次读取的计数值
static int32_t total_max = 300;      // 最大计数值
static int32_t total_min = 0;        // 最小计数值

// TIM3 初始化（编码器模式）
void encoder_init(void) {
    // 使能计数器
    HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);
}

// 获取当前编码器计数值（有符号16位，自动处理溢出）
int16_t encoder_get_count(void) {
    int16_t current = (int16_t)TIM4->CNT;
    int16_t delta = current - last_count;

    // 更新总计数，确保不小于 total_min且不大于 total_max
    total_overflow += delta;
    if (total_overflow < total_min) {
        total_overflow = total_min;
    }else if (total_overflow > total_max) {
        total_overflow = total_max;
    }

    last_count = current;
    return delta;
}

// 获取自初始化以来的总计数（32位有符号）
int32_t encoder_get_total_count(void) {
    return total_overflow;
}

// 清零当前计数值（重置 TIM3->CNT 为 0）
void encoder_reset_count(void) {
    __HAL_TIM_DISABLE_IT(&htim4, TIM_IT_UPDATE);   // 禁用溢出中断（未使用）
    TIM4->CNT = 0;
    last_count = 0;
    total_overflow = 0;
    __HAL_TIM_ENABLE_IT(&htim4, TIM_IT_UPDATE);
}

// 获取原始计数值（0~65535）
uint16_t encoder_get_raw_tim_count(void) {
    return TIM4->CNT;
}

// 获取旋转方向：基于最近一次增量
int8_t encoder_get_direction(void) {
    int16_t delta = encoder_get_count();   // 此函数会更新 last_count 和 total_overflow
    if (delta > 0) return 1;
    if (delta < 0) return -1;
    return 0;
}