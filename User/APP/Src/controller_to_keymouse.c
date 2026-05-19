#include "controller_to_keymouse.h"
#include "custom_input_mapper.h"

typedef struct {
    uint16_t x_pos;
    uint16_t y_pos;
} Position;

const Position key_positions[8] = {
    {1060, 560},  //10弹
    {1110, 560},  //20弹
    {1160, 560},  //50弹
    {1210, 560},  //100弹
    {860, 560},   //-10弹
    {810, 560},   //-20弹
    {760, 560},   //-50弹
    {710, 560}    //-100弹
};

const Position common_positions[2] = {
    {900, 680},   
    {860, 560}  //确认
};

/**
 * @brief 购买指定数量弹药的通用流程
 * @param num  要购买的弹药数量
 */
void buy_ammo_generic(uint16_t num){
    // 根据 num 选择对应的坐标
    uint8_t index = 0xFF;
    switch (num) {
        case 10:   index = 0; break;
        case 20:   index = 1; break;
        case 50:   index = 2; break;
        case 100:  index = 3; break;
        case -10:  index = 4; break;
        case -20:  index = 5; break;
        case -50:  index = 6; break;
        case -100: index = 7; break;
        default:   return; // 无效数量，直接返回
    }
    
    // 点击对应位置
    send_mouse_sequence(key_positions[index].x_pos, key_positions[index].y_pos);      
}

/**
 * @brief 购买指定数量弹药（正数，10~300，步长10）
 * @param target 目标数量（10,20,...,300）
 */
void buy_ammo(uint16_t target){
    if (target < 10 || target > 300 || target % 10 != 0) {
        return; // 无效参数
    }

    send_key_press(VK_I);  // 发送按键操作（如打开购买界面）

    // 按从大到小的顺序分解数量：100, 50, 20, 10
    uint16_t remaining = target;
    while (remaining > 0) {
        if (remaining >= 100) {
            buy_ammo_generic(100);
            remaining -= 100;
        } else if (remaining >= 50) {
            buy_ammo_generic(50);
            remaining -= 50;
        } else if (remaining >= 20) {
            buy_ammo_generic(20);
            remaining -= 20;
        } else if (remaining >= 10) {
            buy_ammo_generic(10);
            remaining -= 10;
        }
    }

    // 购买完成后确认（例如点击“确认”按钮）
    purchase_confirm();  // 你原有的确认序列函数
}

/**
 * @brief 买单确认操作：依次点击确认界面上的两个按钮
 *        坐标分别为 (960, 670) 和 (860, 560)
 */
void purchase_confirm(void)
{
    send_mouse_sequence(common_positions[0].x_pos, common_positions[0].y_pos);
    send_mouse_sequence(common_positions[1].x_pos, common_positions[1].y_pos);
    send_key_press(VK_ESCAPE);  // 发送ESC键以关闭界面
}