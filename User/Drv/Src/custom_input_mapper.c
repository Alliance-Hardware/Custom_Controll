#include "custom_input_mapper.h"
#include "referee_frame_process.h"
#include "referee_protocol.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_uart.h"
#include "usart.h"
#include <string.h>

/* 模块内部状态：当前鼠标坐标（12位有效） */
static uint16_t current_x = 0;
static uint16_t current_y = 0;

/* 底层发送函数：填充 custom_client_data_t 并调用裁判系统打包发送 */
static void send_packet(custom_client_data_t *data) {
    uint8_t buf[8] = {0};

    // byte0-1: 按键1（低8位） 和 按键2（高8位）
    buf[0] = (uint8_t)(data->key_value & 0xFF);
    buf[1] = (uint8_t)((data->key_value >> 8) & 0xFF);

    // byte2-3: 鼠标X坐标（12位）+ 鼠标左键状态（4位）
    // X坐标低8位 -> byte2
    buf[2] = (uint8_t)(data->x_position & 0xFF);
    // X坐标高4位 和 左键状态合并到 byte3
    buf[3] = (uint8_t)(((data->x_position >> 8) & 0x0F) | (data->mouse_left << 4));

    // byte4-5: 鼠标Y坐标（12位）+ 鼠标右键状态（4位）
    buf[4] = (uint8_t)(data->y_position & 0xFF);
    buf[5] = (uint8_t)(((data->y_position >> 8) & 0x0F) | (data->mouse_right << 4));

    // byte6-7: 保留，已初始化为0

    // 打包成裁判系统帧并发送
    uint8_t *frame = referee_pack_data(0x0306, buf, 8);
    HAL_UART_Transmit(&huart1, frame, 17, HAL_MAX_DELAY);  // 17 = 1(header) + 2(cmd) + 1(len) + 8(data) + 2(checksum) + 3(end) 
}

/* 公共初始化：复位鼠标位置，发送空包清除残存状态 */
void custom_input_init(void) {
    current_x = 0;
    current_y = 0;
    custom_client_data_t data = {0};
    send_packet(&data);
}

/* 移动鼠标（更新内部坐标并发送） */
void send_mouse_move(uint16_t x, uint16_t y) {
    current_x = x & 0x0FFF;   // 限制为12位
    current_y = y & 0x0FFF;
    custom_client_data_t data = {0};
    data.x_position = current_x;
    data.y_position = current_y;
    send_packet(&data);
}

/* 鼠标按键（使用当前保存的坐标） */
void send_mouse_button(uint8_t button, uint8_t state) {
    custom_client_data_t data = {0};
    data.x_position = current_x;
    data.y_position = current_y;
    if (button == MOUSE_LEFT_BUTTON) {
        data.mouse_left = (state == MOUSE_PRESS) ? 1 : 0;
    } else {
        data.mouse_right = (state == MOUSE_PRESS) ? 1 : 0;
    }
    send_packet(&data);
}

/* 完整鼠标点击动作：按下 -> 延时20ms -> 释放 */
void send_mouse_click(uint8_t button) {
    send_mouse_button(button, MOUSE_PRESS);
    HAL_Delay(20);
    send_mouse_button(button, MOUSE_RELEASE);
}

/* 键盘按键按下（仅发送，不自动释放） */
void send_key_down(uint8_t vk_code) {
    custom_client_data_t data = {0};
    data.key_value = vk_code;   // 低字节为键值1，高字节为0
    send_packet(&data);
}

/* 键盘按键释放（发送空包表示无按键） */
void send_key_up(uint8_t vk_code) {
    (void)vk_code;   // 释放时不需要键值，统一发空包
    custom_client_data_t data = {0};
    send_packet(&data);
}

/* 按下并释放一个按键（延时25ms） */
void send_key_press(uint8_t vk_code) {
    send_key_down(vk_code);
    HAL_Delay(25);
    send_key_up(vk_code);
    HAL_Delay(25);
}

/* 发送双键值（组合键，如 Ctrl + C 等）*/
void send_two_keys(uint8_t vk_code1, uint8_t vk_code2) {
    custom_client_data_t data = {0};
    data.key_value = ((uint16_t)vk_code2 << 8) | vk_code1;
    send_packet(&data);
    /* 组合键发送后通常需要手动释放，请调用 send_key_up(0) 或再发一次空包 */
}

/**
 * @brief 鼠标点击操作
 * @param x X coordinate
 * @param y Y coordinate
 */
void send_mouse_sequence(uint16_t x, uint16_t y)
{
    custom_client_data_t custom_client_data = {0};
    uint8_t client_data[8] = {0};
    custom_client_data = (custom_client_data_t){
        .key_value = 0,			//网上任意键值
        .x_position = x,
        .y_position = y,
        .mouse_left = 0,
        .mouse_right = 0
    };
    memcpy(client_data, &custom_client_data, 8);
    HAL_UART_Transmit(&huart1, referee_pack_data(0x0306, client_data, 8), getRefSentDataLen(), 30);
    HAL_Delay(10);
    
    custom_client_data.mouse_left = 1;
    memcpy(client_data, &custom_client_data, 8);
    HAL_UART_Transmit(&huart1, referee_pack_data(0x0306, client_data, 8), getRefSentDataLen(), 30);
    HAL_Delay(10);
    
    custom_client_data.mouse_left = 0;
    memcpy(client_data, &custom_client_data, 8);
    HAL_UART_Transmit(&huart1, referee_pack_data(0x0306, client_data, 8), getRefSentDataLen(), 30);
    HAL_Delay(10);
}

