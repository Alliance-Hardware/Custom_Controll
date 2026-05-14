#include "oled.h"
#include "oledfont.h"   // 包含字库

uint8_t OLED_GRAM[144][4];
uint8_t x_offset;   // 屏幕偏移

// 反显
void OLED_ColorTurn(uint8_t i)
{
    if(i == 0)  OLED_WR_Byte(0xA6, OLED_CMD); // 正常
    if(i == 1)  OLED_WR_Byte(0xA7, OLED_CMD); // 反色
}

// 屏幕旋转
void OLED_DisplayTurn(uint8_t i)
{
    if(i == 0) {
        x_offset = 4;
        OLED_WR_Byte(0xC8, OLED_CMD); // 正常
        OLED_WR_Byte(0xA1, OLED_CMD);
    }
    if(i == 1) {
        x_offset = 0;
        OLED_WR_Byte(0xC0, OLED_CMD); // 反转
        OLED_WR_Byte(0xA0, OLED_CMD);
    }
}

// 写一个字节（模拟SPI）
void OLED_WR_Byte(uint8_t dat, uint8_t cmd)
{
    uint8_t i;
    if(cmd) OLED_DC_Set();
    else    OLED_DC_Clr();

    OLED_CS_Clr();  // 片选使能

    for(i = 0; i < 8; i++) {
        OLED_SCL_Clr();
        if(dat & 0x80)  OLED_SDA_Set();
        else            OLED_SDA_Clr();
        OLED_SCL_Set();
        dat <<= 1;
    }

    OLED_CS_Set();
    OLED_DC_Set();
}

// 开启显示
void OLED_DisPlay_On(void)
{
    OLED_WR_Byte(0x8D, OLED_CMD);
    OLED_WR_Byte(0x14, OLED_CMD);
    OLED_WR_Byte(0xAF, OLED_CMD);
}

// 关闭显示
void OLED_DisPlay_Off(void)
{
    OLED_WR_Byte(0x8D, OLED_CMD);
    OLED_WR_Byte(0x10, OLED_CMD);
    OLED_WR_Byte(0xAE, OLED_CMD);
}

// 刷新显存
void OLED_Refresh(void)
{
    uint8_t i, n;
    for(i = 0; i < 4; i++) {
        OLED_WR_Byte(0xB0 + i, OLED_CMD);          // 行地址
        OLED_WR_Byte(x_offset, OLED_CMD);          // 低列地址
        OLED_WR_Byte(0x10, OLED_CMD);              // 高列地址
        for(n = 0; n < 128; n++) {
            OLED_WR_Byte(OLED_GRAM[n][i], OLED_DATA);
        }
    }
}

// 清屏
void OLED_Clear(void)
{
    uint8_t i, n;
    for(i = 0; i < 4; i++) {
        for(n = 0; n < 128; n++) {
            OLED_GRAM[n][i] = 0;
        }
    }
    OLED_Refresh();
}

// 画点
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t t)
{
    uint8_t i, m, n;
    i = y / 8;
    m = y % 8;
    n = 1 << m;
    if(t) {
        OLED_GRAM[x][i] |= n;
    } else {
        OLED_GRAM[x][i] = ~OLED_GRAM[x][i];
        OLED_GRAM[x][i] |= n;
        OLED_GRAM[x][i] = ~OLED_GRAM[x][i];
    }
}

// 显示字符
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t size1, uint8_t mode)
{
    uint8_t i, m, temp, size2;
    uint8_t x0 = x, y0 = y;

    if(size1 == 8)      size2 = 6;
    else                size2 = (size1 / 8 + ((size1 % 8) ? 1 : 0)) * (size1 / 2);

    chr -= ' ';
    for(i = 0; i < size2; i++) {
        if(size1 == 8)          temp = asc2_0806[chr][i];
        else if(size1 == 12)    temp = asc2_1206[chr][i];
        else if(size1 == 16)    temp = asc2_1608[chr][i];
        else if(size1 == 24)    temp = asc2_2412[chr][i];
        else return;

        for(m = 0; m < 8; m++) {
            if(temp & 0x01)     OLED_DrawPoint(x, y, mode);
            else                OLED_DrawPoint(x, y, !mode);
            temp >>= 1;
            y++;
        }
        x++;
        if((size1 != 8) && ((x - x0) == size1 / 2)) {
            x = x0;
            y0 = y0 + 8;
        }
        y = y0;
    }
}

// 显示字符串
void OLED_ShowString(uint8_t x, uint8_t y, uint8_t *chr, uint8_t size1, uint8_t mode)
{
    while((*chr >= ' ') && (*chr <= '~')) {
        OLED_ShowChar(x, y, *chr, size1, mode);
        if(size1 == 8)  x += 6;
        else            x += size1 / 2;
        chr++;
    }
}

// 幂运算（内部使用）
static uint32_t OLED_Pow(uint8_t m, uint8_t n)
{
    uint32_t result = 1;
    while(n--) result *= m;
    return result;
}

// 显示数字
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size1, uint8_t mode)
{
    uint8_t t, temp, m = 0;
    if(size1 == 8) m = 2;
    for(t = 0; t < len; t++) {
        temp = (num / OLED_Pow(10, len - t - 1)) % 10;
        if(temp == 0)   OLED_ShowChar(x + (size1 / 2 + m) * t, y, '0', size1, mode);
        else            OLED_ShowChar(x + (size1 / 2 + m) * t, y, temp + '0', size1, mode);
    }
}

// 显示汉字
void OLED_ShowChinese(uint8_t x, uint8_t y, uint8_t num, uint8_t size1, uint8_t mode)
{
    uint8_t m, temp;
    uint8_t x0 = x, y0 = y;
    uint16_t i, size3 = (size1 / 8 + ((size1 % 8) ? 1 : 0)) * size1;
    for(i = 0; i < size3; i++) {
        if(size1 == 16)         temp = Hzk1[num][i];
        else if(size1 == 24)    temp = Hzk2[num][i];
        else if(size1 == 32)    temp = Hzk3[num][i];
        else if(size1 == 64)    temp = Hzk4[num][i];
        else return;

        for(m = 0; m < 8; m++) {
            if(temp & 0x01)     OLED_DrawPoint(x, y, mode);
            else                OLED_DrawPoint(x, y, !mode);
            temp >>= 1;
            y++;
        }
        x++;
        if((x - x0) == size1) {
            x = x0;
            y0 = y0 + 8;
        }
        y = y0;
    }
}

// 滚动显示（简单实现）
void OLED_ScrollDisplay(uint8_t num, uint8_t space, uint8_t mode)
{
    uint8_t i, n, t = 0, m = 0, r;
    while(1) {
        if(m == 0) {
            OLED_ShowChinese(128, 8, t, 16, mode);
            t++;
        }
        if(t == num) {
            for(r = 0; r < 16 * space; r++) {
                for(i = 1; i < 144; i++) {
                    for(n = 0; n < 4; n++) {
                        OLED_GRAM[i-1][n] = OLED_GRAM[i][n];
                    }
                }
                OLED_Refresh();
            }
            t = 0;
        }
        m++;
        if(m == 16) m = 0;
        for(i = 1; i < 144; i++) {
            for(n = 0; n < 4; n++) {
                OLED_GRAM[i-1][n] = OLED_GRAM[i][n];
            }
        }
        OLED_Refresh();
    }
}

// 显示图片
void OLED_ShowPicture(uint8_t x, uint8_t y, uint8_t sizex, uint8_t sizey, uint8_t BMP[], uint8_t mode)
{
    uint16_t j = 0;
    uint8_t i, n, temp, m;
    uint8_t x0 = x, y0 = y;
    sizey = sizey / 8 + ((sizey % 8) ? 1 : 0);
    for(n = 0; n < sizey; n++) {
        for(i = 0; i < sizex; i++) {
            temp = BMP[j++];
            for(m = 0; m < 8; m++) {
                if(temp & 0x01)     OLED_DrawPoint(x, y, mode);
                else                OLED_DrawPoint(x, y, !mode);
                temp >>= 1;
                y++;
            }
            x++;
            if((x - x0) == sizex) {
                x = x0;
                y0 = y0 + 8;
            }
            y = y0;
        }
    }
}

// 微秒延时（简单循环，可根据需要调整）
static void delay_us(uint32_t us)
{
    uint32_t i;
    for(i = 0; i < us * 8; i++) {   // 粗略延时，实际需根据主频校准
        __NOP();
    }
}

// OLED 初始化（HAL 版本）
void OLED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // 使能 GPIOA 时钟
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // 配置 PA0~PA4 为推挽输出，速度 50MHz
    GPIO_InitStruct.Pin = OLED_SCL_GPIO_PIN | OLED_SDA_GPIO_PIN |
                          OLED_RES_GPIO_PIN | OLED_DC_GPIO_PIN |
                          OLED_CS_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(OLED_GPIO_PORT, &GPIO_InitStruct);

    // 复位 OLED
    OLED_RES_Clr();
    HAL_Delay(200);   // 200ms
    OLED_RES_Set();

    // 以下为 SSD1305 初始化序列（与原代码一致）
    OLED_WR_Byte(0xAE, OLED_CMD);   // 关显示
    OLED_WR_Byte(0x04, OLED_CMD);   // 低列地址
    OLED_WR_Byte(0x10, OLED_CMD);   // 高列地址
    OLED_WR_Byte(0x40, OLED_CMD);   // 起始行
    OLED_WR_Byte(0x81, OLED_CMD);   // 对比度
    OLED_WR_Byte(0x80, OLED_CMD);
    OLED_WR_Byte(0xA1, OLED_CMD);   // 段重映射
    OLED_WR_Byte(0xA6, OLED_CMD);   // 正常显示
    OLED_WR_Byte(0xA8, OLED_CMD);   // 复用率
    OLED_WR_Byte(0x1F, OLED_CMD);   // 1/32
    OLED_WR_Byte(0xC8, OLED_CMD);   // COM 扫描方向
    OLED_WR_Byte(0xD3, OLED_CMD);   // 显示偏移
    OLED_WR_Byte(0x00, OLED_CMD);
    OLED_WR_Byte(0xD5, OLED_CMD);   // 时钟分频
    OLED_WR_Byte(0xF0, OLED_CMD);
    OLED_WR_Byte(0xD8, OLED_CMD);   // 区域颜色模式
    OLED_WR_Byte(0x05, OLED_CMD);
    OLED_WR_Byte(0xD9, OLED_CMD);   // 预充电周期
    OLED_WR_Byte(0xC2, OLED_CMD);
    OLED_WR_Byte(0xDA, OLED_CMD);   // COM 引脚配置
    OLED_WR_Byte(0x12, OLED_CMD);
    OLED_WR_Byte(0xDB, OLED_CMD);   // Vcomh
    OLED_WR_Byte(0x08, OLED_CMD);

    OLED_Clear();                   // 清屏
    OLED_WR_Byte(0xAF, OLED_CMD);   // 开显示
}