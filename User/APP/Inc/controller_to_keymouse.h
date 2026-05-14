#ifndef CONTROLLER_TO_KEYMOUSE_H
#define CONTROLLER_TO_KEYMOUSE_H

#include <stdint.h>

/**
 * @brief 购买指定数量弹药的通用流程
 * @param num  要购买的弹药数量
 */
void buy_ammo_generic(uint16_t num);

/**
 * @brief 买单确认操作：依次点击确认界面上的两个按钮
 *        坐标分别为 (960, 670) 和 (860, 560)
 */
void purchase_confirm(void);

#endif // CONTROLLER_TO_KEYMOUSE_H