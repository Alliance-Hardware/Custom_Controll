#ifndef __CUSTOM_INPUT_MAPPER_H
#define __CUSTOM_INPUT_MAPPER_H

#include <stdint.h>

/* 鼠标按键定义 */
#define MOUSE_LEFT_BUTTON   0
#define MOUSE_RIGHT_BUTTON  1

/* 鼠标按键状态 */
#define MOUSE_PRESS         1
#define MOUSE_RELEASE       0

/* Windows 虚拟键码（仅列出需要的） */
// 键盘主要按键
#define VK_BACK			0x08
#define VK_TAB			0x09
#define VK_CLEAR		0x0C
#define VK_RETURN		0x0D
#define VK_SHIFT		0x10
#define VK_CONTROL		0x11
#define VK_MENU			0x12
#define VK_PAUSE		0x13
#define VK_CAPITAL		0x14
#define VK_ESCAPE		0x1B
#define VK_SPACE		0x20
#define VK_PRIOR		0x21
#define VK_NEXT			0x22
#define VK_END			0x23
#define VK_HOME			0x24
#define VK_LEFT			0x25
#define VK_UP			0x26
#define VK_RIGHT		0x27
#define VK_DOWN			0x28
#define VK_SELECT		0x29
#define VK_PRINT		0x2A
#define VK_EXECUTE		0x2B
#define VK_SNAPSHOT		0x2C
#define VK_INSERT		0x2D
#define VK_DELETE		0x2E
#define VK_HELP			0x2F

// 数字 0-9（顶部）
#define VK_0			0x30
#define VK_1			0x31
#define VK_2			0x32
#define VK_3			0x33
#define VK_4			0x34
#define VK_5			0x35
#define VK_6			0x36
#define VK_7			0x37
#define VK_8			0x38
#define VK_9			0x39

// 字母 A-Z
#define VK_A			0x41
#define VK_B			0x42
#define VK_C			0x43
#define VK_D			0x44
#define VK_E			0x45
#define VK_F			0x46
#define VK_G			0x47
#define VK_H			0x48
#define VK_I			0x49
#define VK_J			0x4A
#define VK_K			0x4B
#define VK_L			0x4C
#define VK_M			0x4D
#define VK_N			0x4E
#define VK_O			0x4F
#define VK_P			0x50
#define VK_Q			0x51
#define VK_R			0x52
#define VK_S			0x53
#define VK_T			0x54
#define VK_U			0x55
#define VK_V			0x56
#define VK_W			0x57
#define VK_X			0x58
#define VK_Y			0x59
#define VK_Z			0x5A

// 功能键 F1-F24
#define VK_F1			0x70
#define VK_F2			0x71
#define VK_F3			0x72
#define VK_F4			0x73
#define VK_F5			0x74
#define VK_F6			0x75
#define VK_F7			0x76
#define VK_F8			0x77
#define VK_F9			0x78
#define VK_F10			0x79
#define VK_F11			0x7A
#define VK_F12			0x7B
#define VK_F13			0x7C
#define VK_F14			0x7D
#define VK_F15			0x7E
#define VK_F16			0x7F
#define VK_F17			0x80
#define VK_F18			0x81
#define VK_F19			0x82
#define VK_F20			0x83
#define VK_F21			0x84
#define VK_F22			0x85
#define VK_F23			0x86
#define VK_F24			0x87

// 锁定与修饰键
#define VK_NUMLOCK		0x90
#define VK_SCROLL		0x91
#define VK_LSHIFT		0xA0
#define VK_RSHIFT		0xA1
#define VK_LCONTROL		0xA2
#define VK_RCONTROL		0xA3
#define VK_LMENU		0xA4
#define VK_RMENU		0xA5


/* 初始化模块（清空鼠标坐标，发送复位包） */
void custom_input_init(void);

/* 鼠标移动（坐标范围 0~4095） */
void send_mouse_move(uint16_t x, uint16_t y);

/* 鼠标按键动作（button: MOUSE_LEFT_BUTTON / MOUSE_RIGHT_BUTTON, state: MOUSE_PRESS / MOUSE_RELEASE） */
void send_mouse_button(uint8_t button, uint8_t state);

/* 鼠标点击（按下后自动释放，延时 20ms） */
void send_mouse_click(uint8_t button);

/* 键盘按键按下（vk_code: 虚拟键码） */
void send_key_down(uint8_t vk_code);

/* 键盘按键释放（发送空包） */
void send_key_up(uint8_t vk_code);

/* 便捷函数：按下 + 延时 50ms + 释放 */
void send_key_press(uint8_t vk_code);

/* 发送两个键值（组合键，高位字节为键值2，低位字节为键值1） */
void send_two_keys(uint8_t vk_code1, uint8_t vk_code2);


// void send_mouse_sequence(uint16_t x, uint16_t y);
// void send_common_sequence(void);

#endif /* __CUSTOM_INPUT_MAPPER_H__ */