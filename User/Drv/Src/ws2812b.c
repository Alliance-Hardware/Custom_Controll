// ws2812b.c
#include "ws2812b.h"
#include <stdint.h>
#include <string.h>

static uint16_t pwm_buffer[NUM_LEDS * 24 + WS_RESET_CNT] = {0};

// 将RGB格式转换为GRB格式并编码成PWM占空比值
static void encode_led_data(uint8_t r, uint8_t g, uint8_t b, uint16_t* buffer, uint16_t offset) {
    // 数据高位先发，顺序为 GRB [10†L10-L11]
    uint32_t grb_data = (g << 16) | (r << 8) | b;
    
    for (int i = 23; i >= 0; i--) {
        uint32_t bit = (grb_data >> i) & 0x01;
        // 根据bit位是0还是1，将对应的PWM占空比存入数组
        buffer[offset + (23 - i)] = bit ? WS_BIT1 : WS_BIT0;
    }
}

void ws2812b_init(){
    // 初始化TIM1为PWM模式，配置DMA等（在main.c中完成）
    // 这里不需要额外的初始化代码，因为我们直接使用HAL库的TIM和DMA功能
    ws2812b_clear_all(); // 初始化时清除所有灯珠
    for (int i = 0; i < NUM_LEDS; i++) {
        ws2812b_set_color(i, 0, 0, 0); // 初始化所有灯珠为关闭状态
    }
    ws2812b_show(); // 刷新显示
}

// 设置指定索引灯珠的颜色 (索引从0开始)
void ws2812b_set_color(uint8_t led_index, uint8_t r, uint8_t g, uint8_t b) {
    if (led_index >= NUM_LEDS) return;
    encode_led_data(r, g, b, pwm_buffer, led_index * 24);
}

// 将所有灯珠的数据发送刷新
void ws2812b_show(void) {
    // 首先，在数组末尾填充复位信号的低电平计数值 [11†L11-L12]
    for (int i = 0; i < WS_RESET_CNT; i++) {
        pwm_buffer[NUM_LEDS * 24 + i] = 0;
    }

    // 启动DMA传输，将pwm_buffer中的数据连续发送到TIM1->CCR1寄存器
    // 注意：数据传输量为 NUM_LEDS * 24 + WS_RESET_CNT，确保整个数据帧及复位信号都被发送
    HAL_TIM_PWM_Start_DMA(TIM_HTIM, TIM_CHANNEL, (uint32_t*)pwm_buffer, NUM_LEDS * 24 + WS_RESET_CNT);
}

// 清除所有灯珠 (将所有灯珠的RGB值设为0)
void ws2812b_clear_all(void) {
    memset(pwm_buffer, 0, sizeof(pwm_buffer));
    HAL_TIM_PWM_Start_DMA(TIM_HTIM, TIM_CHANNEL, (uint32_t*)pwm_buffer, NUM_LEDS * 24 + WS_RESET_CNT);
}