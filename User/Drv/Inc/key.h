#ifndef __KEY_H
#define __KEY_H

#include "stdint.h"
#include "stdbool.h"

// 按键数量
#define KEY_COUNT       10   // 按键数量

#define KEY_DOUBLE_CLICK_ENABLE_MASK   (1 << 9)   // 仅按键ID=9（编码器按键）启用双击

// 双击检测窗口（ms）
#define KEY_DOUBLE_CLICK_TIMEOUT_MS    300

// 扫描周期（ms），建议5~10ms
#define KEY_SCAN_INTERVAL_MS    5

// 消抖参数：连续多少次相同状态认为稳定
#define KEY_DEBOUNCE_TICKS      3   // 3 * 5ms = 15ms 消抖时间

// 长按判定时间（ms）
#define KEY_LONG_PRESS_MS       1000

// 长按重复触发间隔（ms）
#define KEY_REPEAT_INTERVAL_MS  200

// 组合键等待超时（ms）：按下第一个键后，等待第二个键的时间
#define KEY_COMBO_TIMEOUT_MS    200

// 按键事件类型
typedef enum {
    KEY_EVENT_PRESS,        // 短按（按下并释放，且未达到长按时间）
    KEY_EVENT_LONG_PRESS,   // 长按（按住超过长按时间）
    KEY_EVENT_REPEAT,       // 长按重复触发（每隔一段时间自动触发）
    KEY_EVENT_RELEASE,      // 释放（任意释放）
    KEY_EVENT_COMBO,        // 组合键（由多个按键同时触发）
    KEY_EVENT_DOUBLE_CLICK  // 新增：双击事件
} KeyEvent_t;

// 按键ID（0~8）
typedef uint8_t KeyId_t;

// 回调函数类型：当发生指定按键事件时调用
typedef void (*KeyEventCallback_t)(KeyEvent_t event, uint32_t param);
// param对于普通按键事件为KeyId，对于组合键事件为组合键掩码（bitmask）

// 初始化按键硬件（GPIO和定时器）
void Key_Init(void);

// 注册按键事件回调（用于处理具体业务，如串口发送、RGB控制）
void Key_RegisterCallback(KeyEventCallback_t cb);

// 后台任务（放在主循环中调用，用于处理组合键超时等）
void Key_Process(void);

// 定时器中断服务函数（需要放在定时器中断中调用）
void Key_TimerScan(void);

// 获取按键当前稳定状态（0=释放，1=按下）
uint8_t Key_GetState(KeyId_t id);

/**
 * @brief 更新时间基准（由定时器中断周期性调用）
 * @note  定时器中断周期应为 KEY_SCAN_INTERVAL_MS (5ms)
 *        每次中断调用该函数，使内部时间戳增加 KEY_SCAN_INTERVAL_MS
 */
void Key_UpdateTick(void);

#endif