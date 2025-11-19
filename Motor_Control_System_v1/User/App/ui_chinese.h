#ifndef __UI_CHINESE_H__
#define __UI_CHINESE_H__

#include "stm32f4xx.h"

// ============================= 中文菜单索引定义 =============================
// 使用索引方式引用oledfont.h中的中文字库

// 如需中文显示,请按以下步骤操作:
// 1. 使用"PCtoLCD2002"软件生成16x16中文字模
// 2. 将字模数据添加到oledfont.h的oled_Hzk数组中
// 3. 在下方定义对应的索引号

// 示例中文字符索引(需在oledfont.h中添加对应字模)
#define HZ_IDX_JI     0   // "基"
#define HZ_IDX_BEN    1   // "本"
#define HZ_IDX_YUN    2   // "运"
#define HZ_IDX_XING   3   // "行"
#define HZ_IDX_MO     4   // "模"
#define HZ_IDX_SHI    5   // "式"
// ... 更多中文字符索引

// ============================= 函数声明 =============================

/**
 * @brief 显示中文菜单项(需要完整字库支持)
 * @param x X坐标
 * @param y Y坐标
 * @param menu_index 菜单索引(0-6)
 */
void UI_ShowChineseMenuItem(uint8_t x, uint8_t y, uint8_t menu_index);

#endif // __UI_CHINESE_H__
