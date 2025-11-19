#ifndef __UI_PAGE_APP_H
#define __UI_PAGE_APP_H

#include "stm32f4xx.h"
#include "ui_menu_app.h"

// ============================= 函数声明 =============================

/**
 * @brief 更新指定页面的逻辑
 * @param page 页面ID
 */
void UI_Page_Update(PageState page);

/**
 * @brief 处理指定页面的按键事件
 * @param page 页面ID
 * @param key_event 按键事件
 */
void UI_Page_KeyHandler(PageState page, KeyEvent key_event);

// ============================= 各页面绘制函数 =============================

/**
 * @brief 绘制启动画面
 */
void UI_Page_DrawSplash(void);

/**
 * @brief 绘制主菜单
 */
void UI_Page_DrawMainMenu(void);

/**
 * @brief 绘制基本运行模式页面
 */
void UI_Page_DrawBasicRun(void);

/**
 * @brief 绘制三档转速模式页面
 */
void UI_Page_DrawSpeedGear(void);

/**
 * @brief 绘制加速度测试页面
 */
void UI_Page_DrawAcceleration(void);

/**
 * @brief 绘制梯形曲线运动页面
 */
void UI_Page_DrawTrapezoid(void);

/**
 * @brief 绘制精准圈数控制页面
 */
void UI_Page_DrawCircleControl(void);

/**
 * @brief 绘制系统信息页面
 */
void UI_Page_DrawSystemInfo(void);

/**
 * @brief 绘制参数设置页面
 */
void UI_Page_DrawSettings(void);

#endif // __UI_PAGE_APP_H
