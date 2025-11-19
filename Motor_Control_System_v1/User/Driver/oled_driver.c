#include "oled_driver.h"

/**
 * @brief 使用类似printf的方式显示字符串
 */
int Oled_Printf(uint8_t x, uint8_t y, const char *format, ...)
{
	char buffer[128];
	va_list arg;
	int len;

	va_start(arg, format);
	len = vsnprintf(buffer, sizeof(buffer), format, arg);
	va_end(arg);

	OLED_ShowStr(x, y, buffer, 8);

	return len;
}

// ============================= UI绘制辅助函数实现 =============================

/**
 * @brief 绘制字符串(封装OLED_ShowStr)
 * @param x X坐标(0-127)
 * @param y Y坐标(行号0-3)
 * @param str 字符串
 */
void OLED_ShowString(uint8_t x, uint8_t y, uint8_t *str)
{
    OLED_ShowStr(x, y, (char *)str, 8);  // 使用8x8字体
}

/**
 * @brief 绘制单个像素点(简化版,实际需要按页写入)
 * @param x X坐标(0-127)
 * @param y Y坐标(0-31)
 */
void OLED_DrawPoint(uint8_t x, uint8_t y)
{
    // OLED是按页(8像素高)组织的,这里简化处理
    // 实际绘制需要读-改-写操作,这里仅用于绘制分隔线
    uint8_t page = y / 8;
    uint8_t bit_mask = 1 << (y % 8);

    OLED_Set_Position(x, page);
    OLED_Write_data(bit_mask);
}
