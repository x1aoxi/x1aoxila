#ifndef __UI_MENU_APP_H
#define __UI_MENU_APP_H

#include "stm32f4xx.h"
#include <stdbool.h>

// ============================= 椤甸潰鏋氫妇 =============================
typedef enum {
    PAGE_SPLASH,            // 鍚�鍔ㄧ敾闈�
    PAGE_MAIN_MENU,         // 涓昏彍鍗�
    PAGE_BASIC_RUN,         // 鍩烘湰杩愯�屾ā寮�
    PAGE_SPEED_GEAR,        // 涓夋。杞�閫熸ā寮�
    PAGE_ACCELERATION,      // 鍔犻€熷害娴嬭瘯
    PAGE_TRAPEZOID,         // 姊�褰㈡洸绾胯繍鍔�
    PAGE_CIRCLE_CONTROL,    // 绮惧噯鍦堟暟鎺у埗
    PAGE_SYSTEM_INFO,       // 绯荤粺淇℃伅
    PAGE_SETTINGS,          // 鍙傛暟璁剧疆
    PAGE_COUNT              // 椤甸潰鎬绘暟
} PageState;

// ============================= 鑿滃崟椤圭粨鏋� =============================
typedef struct {
    const char *title;           // 鑿滃崟椤规爣棰�
    PageState target_page;       // 璺宠浆鐨勭洰鏍囬〉闈�
    const char *icon;            // 鍥炬爣瀛楃��(鍙�閫�)
} MenuItem;

// ============================= 菜单状态 =============================
typedef struct {
    PageState current_page;      // 当前页面
    PageState previous_page;     // 上一个页面(用于返回)
    uint8_t selected_index;      // 当前选中项索引
    uint8_t menu_item_count;     // 菜单项数量
    int16_t scroll_offset;       // 滚动偏移量(像素)
    bool is_animating;           // 是否正在播放动画
    uint32_t splash_timer;       // 启动画面计时器
    bool need_redraw;            // 是否需要重绘屏幕(事件驱动刷新)
} MenuState;

// ============================= 鎸夐敭浜嬩欢鏋氫妇 =============================
typedef enum {
    KEY_EVENT_NONE = 0,
    KEY_EVENT_UP,           // KEY1 - 鍚戜笂/鍑忓皯
    KEY_EVENT_DOWN,         // KEY2 - 鍚戜笅/澧炲姞
    KEY_EVENT_CONFIRM,      // KEY3 - 纭�璁�/鍚�鍔�
    KEY_EVENT_BACK,         // KEY4 - 杩斿洖/閫€鍑�
    KEY_EVENT_LONG_PRESS    // 闀挎寜浜嬩欢
} KeyEvent;

// ============================= 鍏ㄥ眬鍙橀噺 =============================
extern MenuState g_menu_state;
extern const MenuItem g_main_menu_items[];

// ============================= 鍑芥暟澹版槑 =============================

/**
 * @brief UI鑿滃崟绯荤粺鍒濆�嬪寲
 */
void UI_Menu_Init(void);

/**
 * @brief UI鑿滃崟鐘舵€佹洿鏂�(10ms鍛ㄦ湡璋冪敤)
 */
void UI_Menu_Update(void);

/**
 * @brief UI鑿滃崟缁樺埗
 */
void UI_Menu_Draw(void);

/**
 * @brief 鎸夐敭浜嬩欢澶勭悊
 * @param key_event 鎸夐敭浜嬩欢
 */
void UI_Menu_KeyHandler(KeyEvent key_event);

/**
 * @brief 鍒囨崲鍒版寚瀹氶〉闈�
 * @param target_page 鐩�鏍囬〉闈�
 */
void UI_Menu_SwitchPage(PageState target_page);

/**
 * @brief 杩斿洖涓婁竴椤�
 */
void UI_Menu_GoBack(void);

#endif // __UI_MENU_APP_H
