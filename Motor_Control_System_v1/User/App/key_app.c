#include "key_app.h"
#include "ui_menu_app.h"  // 引入UI菜单系统

void Key_Init()
{
    Ebtn_Init();
}

void Key_Task()
{
    ebtn_process(HAL_GetTick());
}

// ============================= UI按键事件转换 =============================

/**
 * @brief 将Ebtn事件转换为UI按键事件
 * @param key_id 按键ID (1-4)
 * @param evt Ebtn事件类型
 * @return UI按键事件
 */
KeyEvent Key_ConvertToUIEvent(uint16_t key_id, ebtn_evt_t evt)
{
    // 只处理单击事件(ONCLICK)
    if (evt == EBTN_EVT_ONCLICK) {
        switch (key_id) {
            case 1:
                return KEY_EVENT_UP;        // KEY1 -> 向上/减少
            case 2:
                return KEY_EVENT_DOWN;      // KEY2 -> 向下/增加
            case 3:
                return KEY_EVENT_CONFIRM;   // KEY3 -> 确认/启动
            case 4:
                return KEY_EVENT_BACK;      // KEY4 -> 返回/退出
            default:
                return KEY_EVENT_NONE;
        }
    }

    // 长按事件(KEEPALIVE)转换为长按
    if (evt == EBTN_EVT_KEEPALIVE) {
        return KEY_EVENT_LONG_PRESS;
    }

    return KEY_EVENT_NONE;
}

// ============================= 按键事件回调函数 =============================

extern MOTOR left_motor;
extern MOTOR right_motor;

/* 按键事件处理的回调函数 */
void my_handle_key_event(struct ebtn_btn *btn, ebtn_evt_t evt) {
    uint16_t key_id = btn->key_id;
    uint16_t click_cnt = ebtn_click_get_count(btn);

    // ============================= UI菜单系统按键处理 =============================

    // 将按键事件转换为UI事件
    KeyEvent ui_event = Key_ConvertToUIEvent(key_id, evt);

    // 传递给UI菜单系统处理
    if (ui_event != KEY_EVENT_NONE) {
        UI_Menu_KeyHandler(ui_event);
    }

    // ============================= LED指示(保留原有功能) =============================

    // 按键按下时切换LED状态(用于调试)
    if (evt == EBTN_EVT_ONCLICK && click_cnt == 1) {
        led_buf[key_id - 1] ^= 1;
    }

    // ============================= 调试输出 =============================

    if (evt == EBTN_EVT_ONPRESS) {
        Uart_Printf(DEBUG_UART, "Key%d Down\r\n", (int)key_id);
    }
}
