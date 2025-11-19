#include "ui_menu_app.h"
#include "ui_page_app.h"
#include "ui_animation_app.h"
#include "oled_driver.h"
#include <string.h>

// ============================= 全局变量 =============================
MenuState g_menu_state = {
    .current_page = PAGE_SPLASH,
    .previous_page = PAGE_SPLASH,
    .selected_index = 0,
    .menu_item_count = 0,
    .scroll_offset = 0,
    .is_animating = false,
    .splash_timer = 0,
    .need_redraw = true          // 初始化时需要绘制一次
};

// ============================= 主菜单项定义 =============================
// 如需中文显示,需在oledfont.h中添加完整中文字库并使用OLED_Show_Chinese()
const MenuItem g_main_menu_items[] = {
    {"1.Basic Run",      PAGE_BASIC_RUN,      ">"},  // 基本运行模式
    {"2.Speed Gear",     PAGE_SPEED_GEAR,     ">"},  // 三档转速模式
    {"3.Acceleration",   PAGE_ACCELERATION,   ">"},  // 加速度测试
    {"4.Trapezoid",      PAGE_TRAPEZOID,      ">"},  // 梯形曲线运动
    {"5.Circle Ctrl",    PAGE_CIRCLE_CONTROL, ">"},  // 精准圈数控制
    {"6.System Info",    PAGE_SYSTEM_INFO,    ">"},  // 系统信息
    {"7.Settings",       PAGE_SETTINGS,       ">"},  // 参数设置
};

#define MAIN_MENU_ITEM_COUNT (sizeof(g_main_menu_items) / sizeof(MenuItem))

// ============================= 私有函数声明 =============================
static void handle_splash_page(void);
static void handle_main_menu_page(void);

// ============================= 函数实现 =============================

/**
 * @brief UI菜单系统初始化
 */
void UI_Menu_Init(void)
{
    // 初始化菜单状态
    g_menu_state.current_page = PAGE_SPLASH;
    g_menu_state.previous_page = PAGE_SPLASH;
    g_menu_state.selected_index = 0;
    g_menu_state.menu_item_count = MAIN_MENU_ITEM_COUNT;
    g_menu_state.scroll_offset = 0;
    g_menu_state.is_animating = false;
    g_menu_state.splash_timer = 0;

    // 初始化动画系统
    UI_Animation_Init();
}

/**
 * @brief UI菜单状态更新(10ms周期调用)
 */
void UI_Menu_Update(void)
{
    // 更新动画系统
    UI_Animation_Update(10);  // 10ms间隔

    // 根据当前页面执行相应逻辑
    switch (g_menu_state.current_page) {
        case PAGE_SPLASH:
            handle_splash_page();
            break;

        case PAGE_MAIN_MENU:
            handle_main_menu_page();
            break;

        case PAGE_BASIC_RUN:
        case PAGE_SPEED_GEAR:
        case PAGE_ACCELERATION:
        case PAGE_TRAPEZOID:
        case PAGE_CIRCLE_CONTROL:
        case PAGE_SYSTEM_INFO:
        case PAGE_SETTINGS:
            // 功能页面的更新逻辑由各页面模块处理
            UI_Page_Update(g_menu_state.current_page);
            break;

        default:
            break;
    }
}

/**
 * @brief UI菜单绘制(事件驱动,只在need_redraw=true时刷新)
 */
void UI_Menu_Draw(void)
{
    // 只有在需要重绘时才刷新屏幕
    if (!g_menu_state.need_redraw) {
        return;
    }

    // 清屏
    OLED_Clear();

    // 根据当前页面绘制相应内容
    switch (g_menu_state.current_page) {
        case PAGE_SPLASH:
            UI_Page_DrawSplash();
            break;

        case PAGE_MAIN_MENU:
            UI_Page_DrawMainMenu();
            break;

        case PAGE_BASIC_RUN:
            UI_Page_DrawBasicRun();
            break;

        case PAGE_SPEED_GEAR:
            UI_Page_DrawSpeedGear();
            break;

        case PAGE_ACCELERATION:
            UI_Page_DrawAcceleration();
            break;

        case PAGE_TRAPEZOID:
            UI_Page_DrawTrapezoid();
            break;

        case PAGE_CIRCLE_CONTROL:
            UI_Page_DrawCircleControl();
            break;

        case PAGE_SYSTEM_INFO:
            UI_Page_DrawSystemInfo();
            break;

        case PAGE_SETTINGS:
            UI_Page_DrawSettings();
            break;

        default:
            OLED_ShowString(0, 0, (uint8_t *)"Unknown Page");
            break;
    }

    // 标记已完成绘制
    g_menu_state.need_redraw = false;
}

/**
 * @brief 按键事件处理
 * @param key_event 按键事件
 */
void UI_Menu_KeyHandler(KeyEvent key_event)
{
    if (key_event == KEY_EVENT_NONE) return;

    // 启动画面:任意键跳过
    if (g_menu_state.current_page == PAGE_SPLASH) {
        UI_Menu_SwitchPage(PAGE_MAIN_MENU);
        return;
    }

    // 主菜单页面的按键处理
    if (g_menu_state.current_page == PAGE_MAIN_MENU) {
        switch (key_event) {
            case KEY_EVENT_UP:
                // 向上选择
                if (g_menu_state.selected_index > 0) {
                    g_menu_state.selected_index--;
                    g_menu_state.need_redraw = true;  // 标记需要重绘
                }
                break;

            case KEY_EVENT_DOWN:
                // 向下选择
                if (g_menu_state.selected_index < g_menu_state.menu_item_count - 1) {
                    g_menu_state.selected_index++;
                    g_menu_state.need_redraw = true;  // 标记需要重绘
                }
                break;

            case KEY_EVENT_CONFIRM:
                // 确认进入子页面
                UI_Menu_SwitchPage(g_main_menu_items[g_menu_state.selected_index].target_page);
                break;

            case KEY_EVENT_BACK:
                // 主菜单无返回操作
                break;

            default:
                break;
        }
    }
    // 其他页面的按键处理:传递给页面模块
    else {
        UI_Page_KeyHandler(g_menu_state.current_page, key_event);
    }
}

/**
 * @brief 切换到指定页面
 * @param target_page 目标页面
 */
void UI_Menu_SwitchPage(PageState target_page)
{
    if (target_page >= PAGE_COUNT) return;

    // 保存当前页面为上一个页面
    g_menu_state.previous_page = g_menu_state.current_page;

    // 切换到新页面
    g_menu_state.current_page = target_page;

    // 重置选中索引
    g_menu_state.selected_index = 0;

    // 重置滚动偏移
    g_menu_state.scroll_offset = 0;

    // 标记需要重绘
    g_menu_state.need_redraw = true;
}

/**
 * @brief 返回上一页
 */
void UI_Menu_GoBack(void)
{
    // 从功能页面返回主菜单
    if (g_menu_state.current_page != PAGE_MAIN_MENU &&
        g_menu_state.current_page != PAGE_SPLASH) {
        UI_Menu_SwitchPage(PAGE_MAIN_MENU);
    }
}

// ============================= 私有函数实现 =============================

/**
 * @brief 处理启动画面逻辑
 */
static void handle_splash_page(void)
{
    // 启动画面显示2秒后自动跳转到主菜单
    g_menu_state.splash_timer += 10;  // 10ms递增

    if (g_menu_state.splash_timer >= 2000) {  // 2000ms = 2秒
        UI_Menu_SwitchPage(PAGE_MAIN_MENU);
        g_menu_state.splash_timer = 0;
    }
}

/**
 * @brief 处理主菜单页面逻辑
 */
static void handle_main_menu_page(void)
{
    // 主菜单页面的动态更新逻辑
    // 例如: 滚动动画完成检测等

    if (UI_Animation_IsScrolling()) {
        // 正在滚动,更新滚动偏移量
        g_menu_state.scroll_offset = UI_Animation_GetScrollOffset();
    }
}
