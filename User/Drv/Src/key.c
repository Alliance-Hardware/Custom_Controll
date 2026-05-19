#include "key.h"
#include "main.h"  // 包含GPIO定义等
#include "stm32f1xx_hal.h"   // 根据实际库调整，若使用标准库可替换为相应头文件

// 按键硬件映射表：GPIO端口，引脚， 有效电平（假设低电平有效）
typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin;
    uint8_t active_level;  // 0=低电平按下, 1=高电平按下
} key_hw_t;

// 根据实际电路修改： KEY_0~KEY_9对应的GPIO
static const key_hw_t key_hw_map[KEY_COUNT] = {
    {KEY0_GPIO_Port, KEY0_Pin, 0},   // 假设宏定义由CubeMX生成
    {KEY1_GPIO_Port, KEY1_Pin, 0},
    {KEY2_GPIO_Port, KEY2_Pin, 0},
    {KEY3_GPIO_Port, KEY3_Pin, 0},
    {KEY4_GPIO_Port, KEY4_Pin, 0},
    {KEY5_GPIO_Port, KEY5_Pin, 0},
    {KEY6_GPIO_Port, KEY6_Pin, 0},
    {KEY7_GPIO_Port, KEY7_Pin, 0},
    {KEY8_GPIO_Port, KEY8_Pin, 0},
        {ENCODER_GPIO_Port, ENCODER_Pin, 0}  // 编码器按键也作为一个按键处理
};

// 每个按键的状态机结构
typedef struct {
    uint8_t raw_state;          // 原始电平（本次扫描）
    uint8_t debounce_cnt;       // 消抖计数器
    uint8_t stable_state;       // 稳定后的状态（0释放，1按下）
    
    uint32_t press_tick;             // 按键按下的时间戳（ms）
    uint32_t last_repeat_tick;       // 上次重复触发的时间戳
    bool is_long_press_triggered;    // 是否已经触发过长按事件

    // 以下为双击专用（启用双击的按键才会使用）
    uint32_t last_release_tick;      // 上次释放的时间戳（ms）
    bool double_click_pending;       // 是否正在等待第二次按下（双击待确认）
    bool is_double_click_expected;   // 第二次按下已发生
} KeyState_t;

static KeyState_t keys[KEY_COUNT];

// 全局回调
static KeyEventCallback_t event_callback = NULL;

// 组合键检测相关
static uint32_t combo_mask = 0;         // 当前按下的组合键掩码（bit i表示第i键按下）
static uint32_t combo_start_tick = 0;   // 组合键开始记录的时间戳（第一个键按下）
static bool combo_pending = false;      // 是否正在等待组合键

// ========== 新增：独立的时间基准 ==========
static volatile uint32_t key_ticks_ms = 0;   // 由 Key_UpdateTick() 累加

// 供外部定时器中断调用的时间更新函数
void Key_UpdateTick(void)
{
    key_ticks_ms += KEY_SCAN_INTERVAL_MS;
}

// 获取当前按键模块内部时间戳（用于内部函数）
static inline uint32_t Key_GetTick(void)
{
    return key_ticks_ms;
}

// 判断是否启用双击检测（根据按键ID）
static inline bool is_double_click_enabled(KeyId_t id) {
    return (KEY_DOUBLE_CLICK_ENABLE_MASK & (1 << id)) != 0;
}

// 更新单个按键状态机
static void UpdateKeyState(KeyId_t id) {
    const key_hw_t* hw = &key_hw_map[id];
    GPIO_PinState state = HAL_GPIO_ReadPin(hw->port, hw->pin);
    uint8_t raw = (state == GPIO_PIN_SET) ? 1 : 0;
    // 根据有效电平转换为逻辑值：1表示按下，0表示释放
    keys[id].raw_state = (hw->active_level == 0) ? (1 - raw) : raw;

    uint32_t now = Key_GetTick();   // 使用内部时间戳

    // 消抖处理
    if (keys[id].raw_state == keys[id].stable_state) {
        keys[id].debounce_cnt = 0;
    } else {
        keys[id].debounce_cnt++;
        if (keys[id].debounce_cnt >= KEY_DEBOUNCE_TICKS) {
            // 状态翻转
            keys[id].stable_state = keys[id].raw_state;
            keys[id].debounce_cnt = 0;
            
            // 触发释放事件或按下事件（用于组合键检测和长按计时）
            if (keys[id].raw_state == 1) { // 按下（假设按下为高电平，根据实际电路可调整）
                // 如果当前处于双击等待状态（说明第二次按下开始）
                if (keys[id].double_click_pending) {
                    keys[id].double_click_pending = false;
                    keys[id].is_double_click_expected = true;   // 标记第二次按下已发生
                }
                keys[id].press_tick = now;
                keys[id].last_repeat_tick = now;
                keys[id].is_long_press_triggered = false;
            } else { // 释放
                // 如果之前没有触发过长按，则认为是短按
                if (!keys[id].is_long_press_triggered && event_callback) {
                    if (keys[id].is_double_click_expected) {
                        // 第二次释放 -> 双击
                        keys[id].is_double_click_expected = false;
                        if (event_callback) {
                            event_callback(KEY_EVENT_DOUBLE_CLICK, id);
                        }
                    } else if (is_double_click_enabled(id)) {
                        // 启用双击，第一次释放 -> 等待第二次按下
                        keys[id].last_release_tick = now;
                        keys[id].double_click_pending = true;
                    } else {
                        // 未启用双击 -> 立即触发短按
                        if (event_callback) {
                            event_callback(KEY_EVENT_PRESS, id);
                        }
                    }
                }
                // 无论长短按，释放事件总是触发
                if (event_callback) {
                    event_callback(KEY_EVENT_RELEASE, id);
                }
            }
        }
    }
    
    // 长按检测（仅在稳态按下时）
    if (keys[id].stable_state == 1) {
        uint32_t press_duration = now - keys[id].press_tick;
        if (!keys[id].is_long_press_triggered && press_duration >= KEY_LONG_PRESS_MS) {
            keys[id].is_long_press_triggered = true;
            if (event_callback) {
                event_callback(KEY_EVENT_LONG_PRESS, id);
            }
        }
        // 长按重复触发（每隔一定时间）
        if (keys[id].is_long_press_triggered && (now - keys[id].last_repeat_tick) >= KEY_REPEAT_INTERVAL_MS) {
            keys[id].last_repeat_tick = now;
            if (event_callback) {
                event_callback(KEY_EVENT_REPEAT, id);
            }
        }
    }

    // 双击等待超时检测（仅对启用双击的按键有效）
    if (keys[id].double_click_pending && is_double_click_enabled(id)) {
        if ((now - keys[id].last_release_tick) >= KEY_DOUBLE_CLICK_TIMEOUT_MS) {
            // 超时：确认是单击
            keys[id].double_click_pending = false;
            if (event_callback) {
                event_callback(KEY_EVENT_PRESS, id);
            }
        }
    }
}

// 组合键检测（应在所有按键状态更新后，主循环中调用）
static void CheckCombo(void) {
    uint32_t now_ms = Key_GetTick();   // 使用内部时间戳

    // 构建当前按下的按键掩码
    uint32_t current_mask = 0;
    for (int i = 0; i < KEY_COUNT; i++) {
        if (keys[i].stable_state == 1) {
            current_mask |= (1 << i);
        }
    }
    
    if (current_mask == 0) {
        // 无按键按下，清除组合键等待状态
        combo_pending = false;
        combo_mask = 0;
        return;
    }
    
    // 有按键按下
    if (!combo_pending) {
        // 开始新的组合键检测
        combo_pending = true;
        combo_mask = current_mask;
        combo_start_tick = now_ms;
    } else {
        // 更新组合键掩码（取并集）
        combo_mask |= current_mask;
        // 如果组合键掩码包含多个位，并且超时未到，可以触发组合键事件
        if (__builtin_popcount(combo_mask) >= 2) {   // GNU扩展，计算bit数; 若不用可自行替换
            // 触发组合键事件
            if (event_callback) {
                event_callback(KEY_EVENT_COMBO, combo_mask);
            }
            combo_pending = false;  // 已触发，不再重复
            combo_mask = 0;
        } else if ((now_ms - combo_start_tick) >= KEY_COMBO_TIMEOUT_MS) {
            // 超时，只按下一个键，由普通按键事件处理，不触发组合键
            combo_pending = false;
            combo_mask = 0;
        }
    }
}

// 定时器扫描函数（应在定时器中断中每隔KEY_SCAN_INTERVAL_MS调用一次）
void Key_TimerScan(void) {
    
    for (uint8_t i = 0; i < KEY_COUNT; i++) {
        UpdateKeyState(i);
    }
}

// 后台处理（主循环调用，处理组合键超时等）
void Key_Process(void) {
    uint32_t now = HAL_GetTick();
    if (combo_pending && ((now - combo_start_tick) >= KEY_COMBO_TIMEOUT_MS)) {
        combo_pending = false;
        combo_mask = 0;
    }
}

// 获取按键稳定状态
uint8_t Key_GetState(KeyId_t id) {
    if (id >= KEY_COUNT) return 0;
    return keys[id].stable_state;
}

// 注册回调
void Key_RegisterCallback(KeyEventCallback_t cb) {
    event_callback = cb;
}

// 主初始化
void Key_Init(void) {
    // 清零按键状态
    for (int i = 0; i < KEY_COUNT; i++) {
        keys[i].stable_state = 0;
        keys[i].debounce_cnt = 0;
        keys[i].press_tick = 0;
        keys[i].last_repeat_tick = 0;
        keys[i].is_long_press_triggered = false;
        keys[i].last_release_tick = 0;
        keys[i].double_click_pending = false;
        keys[i].is_double_click_expected = false;
    }
    combo_mask = 0;
    combo_pending = false;
}