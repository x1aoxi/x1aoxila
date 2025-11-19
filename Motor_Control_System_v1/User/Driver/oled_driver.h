#ifndef __OLED_DRIVER_H__
#define __OLED_DRIVER_H__

#include "MyDefine.h"

int Oled_Printf(uint8_t x, uint8_t y, const char *format, ...);

// ============================= UI绘制辅助函数 =============================

/**
 * @brief 绘制字符串(封装OLED_ShowStr)
 * @param x X坐标(0-127)
 * @param y Y坐标(行号0-3)
 * @param str 字符串
 */
void OLED_ShowString(uint8_t x, uint8_t y, uint8_t *str);

/**
 * @brief 绘制单个像素点
 * @param x X坐标(0-127)
 * @param y Y坐标(0-31)
 */
void OLED_DrawPoint(uint8_t x, uint8_t y);

#endif
