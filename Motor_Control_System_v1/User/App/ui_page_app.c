#include "ui_page_app.h"
#include "ui_menu_app.h"
#include "ui_animation_app.h"
#include "oled_driver.h"
#include "motor_app.h"
#include <stdio.h>
#include <string.h>

// ============================= 外部变量引用 =============================
extern MenuState g_menu_state;
extern const MenuItem g_main_menu_items[];

// ============================= 页面状态变量 =============================

// UI刷新控制
typedef struct {
    uint8_t update_counter;  // 更新计数器,用于限制刷新频率
    float last_rpm;          // 上次显示的转速
} UIRefreshState;

static UIRefreshState ui_refresh = {0, 0.0f};

// ============================= 辅助函数 =============================

/**
 * @brief 绘制居中文本
 * @param y Y坐标(行号0-3)
 * @param text 文本内容
 */
static void draw_centered_text(uint8_t y, const char *text)
{
    uint8_t len = strlen(text);
    uint8_t x = (128 - len * 6) / 2;  // 假设字符宽度为6像素
    OLED_ShowString(x, y, (uint8_t *)text);
}

// ============================= 页面更新函数 =============================

/**
 * @brief 更新指定页面的逻辑
 * @param page 页面ID
 * @note 该函数每10ms被调用一次,使用计数器限制刷新频率
 */
void UI_Page_Update(PageState page)
{
    MotorState* motor = MotorApp_GetState();

    // 系统信息页面需要持续刷新编码器数值
    if (page == PAGE_SYSTEM_INFO) {
        // 每200ms更新一次编码器显示
        if (++ui_refresh.update_counter >= 20) {
            ui_refresh.update_counter = 0;
            g_menu_state.need_redraw = true;
        }
        return;
    }

    // 其他页面:只有在电机运行时才需要定期刷新显示
    if (!motor->is_running) {
        return;
    }

    // 各功能页面的更新逻辑
    switch (page) {
        case PAGE_CIRCLE_CONTROL:
            // 圈数控制模式: 需要持续刷新圈数显示
            if (++ui_refresh.update_counter >= 20) {  // 200ms刷新一次
                ui_refresh.update_counter = 0;
                g_menu_state.need_redraw = true;  // 强制刷新显示圈数
            }
            break;

        case PAGE_BASIC_RUN:
        case PAGE_SPEED_GEAR:
        case PAGE_ACCELERATION:
        case PAGE_TRAPEZOID:
            // 每200ms更新一次显示(20次计数)
            if (++ui_refresh.update_counter >= 20) {  // 20*10ms=200ms
                ui_refresh.update_counter = 0;

                // 获取当前转速
                float current_rpm = MotorApp_GetCurrentRPM();

                // 只有转速变化超过0.5rpm时才刷新(避免小波动导致频繁刷新)
                if (current_rpm - ui_refresh.last_rpm > 0.5f ||
                    ui_refresh.last_rpm - current_rpm > 0.5f) {
                    ui_refresh.last_rpm = current_rpm;
                    g_menu_state.need_redraw = true;
                }
            }
            break;

        case PAGE_SETTINGS:
            // TODO: 后续添加各页面的更新逻辑
            break;
        default:
            break;
    }
}

/**
 * @brief 处理指定页面的按键事件
 * @param page 页面ID
 * @param key_event 按键事件
 */
void UI_Page_KeyHandler(PageState page, KeyEvent key_event)
{
    MotorState* motor = MotorApp_GetState();

    // KEY4统一处理为返回主菜单
    if (key_event == KEY_EVENT_BACK) {
        // 如果正在运行,先停止
        if (MotorApp_IsRunning()) {
            MotorApp_Stop();
            ui_refresh.update_counter = 0;
            ui_refresh.last_rpm = 0.0f;
        }
        UI_Menu_GoBack();
        return;
    }

    // 各功能页面的按键处理
    switch (page) {
        case PAGE_BASIC_RUN:
            // KEY1: 切换方向
            if (key_event == KEY_EVENT_UP) {
                MotorDirection new_dir = (motor->direction == MOTOR_DIR_FORWARD) ?
                                         MOTOR_DIR_REVERSE : MOTOR_DIR_FORWARD;
                MotorApp_BasicRun_SetDirection(new_dir);
                g_menu_state.need_redraw = true;
            }
            // KEY3: 启动/停止
            else if (key_event == KEY_EVENT_CONFIRM) {
                if (MotorApp_IsRunning()) {
                    MotorApp_Stop();
                    ui_refresh.update_counter = 0;
                    ui_refresh.last_rpm = 0.0f;
                } else {
                    MotorApp_SetMode(MOTOR_MODE_BASIC_RUN);
                    MotorApp_Start();
                    ui_refresh.update_counter = 0;
                }
                g_menu_state.need_redraw = true;
            }
            break;

        case PAGE_SPEED_GEAR:
            // KEY1: 切换档位(减少)
            if (key_event == KEY_EVENT_UP) {
                MotorApp_SpeedGear_DecreaseGear();
                g_menu_state.need_redraw = true;
            }
            // KEY2: 切换档位(增加)
            else if (key_event == KEY_EVENT_DOWN) {
                MotorApp_SpeedGear_IncreaseGear();
                g_menu_state.need_redraw = true;
            }
            // KEY3: 启动/停止
            else if (key_event == KEY_EVENT_CONFIRM) {
                if (MotorApp_IsRunning()) {
                    MotorApp_Stop();
                    ui_refresh.update_counter = 0;
                    ui_refresh.last_rpm = 0.0f;
                } else {
                    MotorApp_SetMode(MOTOR_MODE_SPEED_GEAR);
                    MotorApp_Start();
                    ui_refresh.update_counter = 0;
                }
                g_menu_state.need_redraw = true;
            }
            break;

        case PAGE_ACCELERATION:
            // KEY1: 切换加速度模式(Low ↔ High)
            if (key_event == KEY_EVENT_UP) {
                MotorApp_Acceleration_ToggleMode();
                g_menu_state.need_redraw = true;
            }
            // KEY3: 启动/停止
            else if (key_event == KEY_EVENT_CONFIRM) {
                if (MotorApp_IsRunning()) {
                    MotorApp_Stop();
                    ui_refresh.update_counter = 0;
                    ui_refresh.last_rpm = 0.0f;
                } else {
                    MotorApp_SetMode(MOTOR_MODE_ACCELERATION);
                    MotorApp_Start();
                    ui_refresh.update_counter = 0;
                }
                g_menu_state.need_redraw = true;
            }
            break;

        case PAGE_TRAPEZOID:
            // KEY3: 启动/停止
            if (key_event == KEY_EVENT_CONFIRM) {
                if (MotorApp_IsRunning()) {
                    MotorApp_Stop();
                    ui_refresh.update_counter = 0;
                    ui_refresh.last_rpm = 0.0f;
                } else {
                    MotorApp_SetMode(MOTOR_MODE_TRAPEZOID);
                    MotorApp_Start();
                    ui_refresh.update_counter = 0;
                }
                g_menu_state.need_redraw = true;
            }
            break;

        case PAGE_CIRCLE_CONTROL:
            // KEY1: 减少圈数
            if (key_event == KEY_EVENT_UP) {
                MotorApp_CircleControl_DecreaseTarget();
                g_menu_state.need_redraw = true;
            }
            // KEY2: 增加圈数
            else if (key_event == KEY_EVENT_DOWN) {
                MotorApp_CircleControl_IncreaseTarget();
                g_menu_state.need_redraw = true;
            }
            // KEY3: 启动/停止
            else if (key_event == KEY_EVENT_CONFIRM) {
                if (MotorApp_IsRunning()) {
                    MotorApp_Stop();
                    ui_refresh.update_counter = 0;
                    ui_refresh.last_rpm = 0.0f;
                } else {
                    MotorApp_SetMode(MOTOR_MODE_CIRCLE_CONTROL);
                    MotorApp_Start();
                    ui_refresh.update_counter = 0;
                }
                g_menu_state.need_redraw = true;
            }
            break;

        case PAGE_SYSTEM_INFO:
        case PAGE_SETTINGS:
            // TODO: 后续添加各页面的按键处理逻辑
            break;
        default:
            break;
    }
}

// ============================= 页面绘制函数 =============================

/**
 * @brief 绘制启动画面
 */
void UI_Page_DrawSplash(void)
{
    draw_centered_text(0, "===============");
    draw_centered_text(1, "DC Motor Control");
    draw_centered_text(2, "  System v1.0");
    draw_centered_text(3, "Press Any Key...");
}

/**
 * @brief 绘制主菜单(适配32像素高度,每屏显示3项)
 */
void UI_Page_DrawMainMenu(void)
{
    // 标题行
    OLED_ShowString(0, 0, (uint8_t *)"Main Menu");

    // 计算显示范围(每屏显示3项,从第1行开始)
    uint8_t visible_count = 3;
    int16_t start_index = g_menu_state.selected_index - 1;
    if (start_index < 0) start_index = 0;
    if (start_index + visible_count > g_menu_state.menu_item_count) {
        start_index = g_menu_state.menu_item_count - visible_count;
        if (start_index < 0) start_index = 0;
    }

    // 绘制菜单项(从第1行开始,占用第1-3行)
    for (uint8_t i = 0; i < visible_count && (start_index + i) < g_menu_state.menu_item_count; i++) {
        uint8_t menu_index = start_index + i;
        uint8_t row = i + 1;  // 第1-3行

        // 绘制光标
        if (menu_index == g_menu_state.selected_index) {
            OLED_ShowString(0, row, (uint8_t *)">");
        } else {
            OLED_ShowString(0, row, (uint8_t *)" ");
        }

        // 绘制菜单项文本
        OLED_ShowString(8, row, (uint8_t *)g_main_menu_items[menu_index].title);
    }

    // 绘制滚动指示器(如果菜单项超过3个)
    if (g_menu_state.menu_item_count > visible_count) {
        if (start_index > 0) {
            OLED_ShowString(120, 1, (uint8_t *)"^");  // 向上箭头
        }
        if (start_index + visible_count < g_menu_state.menu_item_count) {
            OLED_ShowString(120, 3, (uint8_t *)"v");  // 向下箭头
        }
    }
}

/**
 * @brief 绘制基本运行模式页面
 */
void UI_Page_DrawBasicRun(void)
{
    char buf[22];  // 128像素/6=21个字符+结束符
    MotorState* motor = MotorApp_GetState();

    // 第0行:标题
    OLED_ShowString(0, 0, (uint8_t *)"Basic Run  [1/7]");

    // 第1行:状态
    if (motor->is_running) {
        OLED_ShowString(0, 1, (uint8_t *)"Status: Running  ");
    } else {
        OLED_ShowString(0, 1, (uint8_t *)"Status: Stopped  ");
    }

    // 第2行:方向和速度(使用缓存的转速值)
    const char *dir_str = (motor->direction == MOTOR_DIR_FORWARD) ? "FWD" : "REV";
    snprintf(buf, sizeof(buf), "Dir:%s %.1frpm  ", dir_str, ui_refresh.last_rpm);
    OLED_ShowString(0, 2, (uint8_t *)buf);

    // 第3行:操作提示
    OLED_ShowString(0, 3, (uint8_t *)"[1]Dir [3]Run [4]Back");
}

/**
 * @brief 绘制三档转速模式页面
 */
void UI_Page_DrawSpeedGear(void)
{
    char buf[22];
    MotorState* motor = MotorApp_GetState();
    const char* gear_names[] = {"Low", "Mid", "High"};

    // 第0行:标题
    OLED_ShowString(0, 0, (uint8_t *)"Speed Gear [2/7]");

    // 第1行:档位选择(带选中标记)
    snprintf(buf, sizeof(buf), "Gear:[%s]%s %s ",
             motor->current_gear == SPEED_GEAR_LOW ? gear_names[0] : "   ",
             motor->current_gear == SPEED_GEAR_MID ? gear_names[1] : "   ",
             motor->current_gear == SPEED_GEAR_HIGH ? gear_names[2] : "    ");
    OLED_ShowString(0, 1, (uint8_t *)buf);

    // 第2行:目标转速
    snprintf(buf, sizeof(buf), "Target: %.1f rpm  ", motor->target_rpm);
    OLED_ShowString(0, 2, (uint8_t *)buf);

    // 第3行:实际转速(使用缓存值) 或 操作提示
    if (motor->is_running) {
        snprintf(buf, sizeof(buf), "Actual: %.1f rpm  ", ui_refresh.last_rpm);
        OLED_ShowString(0, 3, (uint8_t *)buf);
    } else {
        OLED_ShowString(0, 3, (uint8_t *)"[1/2]Gear [3]Run");
    }
}

/**
 * @brief 绘制加速度测试页面
 */
void UI_Page_DrawAcceleration(void)
{
    char buf[22];
    MotorState* motor = MotorApp_GetState();
    const char* mode_names[] = {"Low", "High"};

    // 第0行:标题
    OLED_ShowString(0, 0, (uint8_t *)"Accel Test [3/7]");

    // 第1行:加速度模式选择
    snprintf(buf, sizeof(buf), "Mode: [%s] %s ",
             motor->accel_mode == ACCEL_MODE_LOW ? mode_names[0] : "   ",
             motor->accel_mode == ACCEL_MODE_HIGH ? mode_names[1] : "    ");
    OLED_ShowString(0, 1, (uint8_t *)buf);

    // 第2行:加速度值
    float accel_value = (motor->accel_mode == ACCEL_MODE_LOW) ? 5.0f : 20.0f;
    snprintf(buf, sizeof(buf), "Accel: %.0f rpm/s  ", accel_value);
    OLED_ShowString(0, 2, (uint8_t *)buf);

    // 第3行:当前转速
    snprintf(buf, sizeof(buf), "Speed: %.1f rpm  ", ui_refresh.last_rpm);
    OLED_ShowString(0, 3, (uint8_t *)buf);
}

/**
 * @brief 绘制梯形曲线运动页面
 */
void UI_Page_DrawTrapezoid(void)
{
    char buf[22];
    MotorState* motor = MotorApp_GetState();
    const char* phase_names[] = {"Idle", "Accel", "Const", "Decel", "Done"};

    // 第0行:标题
    OLED_ShowString(0, 0, (uint8_t *)"Trapezoid  [4/7]");

    // 第1行:当前阶段
    snprintf(buf, sizeof(buf), "Phase:[%s]  ", phase_names[motor->trapezoid_phase]);
    OLED_ShowString(0, 1, (uint8_t *)buf);

    // 第2行:阶段时间
    float elapsed_time = motor->trapezoid_timer * 0.01f;  // 转换为秒
    snprintf(buf, sizeof(buf), "Time: %.1f s    ", elapsed_time);
    OLED_ShowString(0, 2, (uint8_t *)buf);

    // 第3行:当前转速
    snprintf(buf, sizeof(buf), "Speed: %.1f rpm  ", ui_refresh.last_rpm);
    OLED_ShowString(0, 3, (uint8_t *)buf);
}

/**
 * @brief 绘制精准圈数控制页面
 */
void UI_Page_DrawCircleControl(void)
{
    char buf[22];
    MotorState* motor = MotorApp_GetState();

    // 第0行:标题
    OLED_ShowString(0, 0, (uint8_t *)"Circle Ctrl[5/7]");

    // 第1行:目标圈数(可调节,1-20)
    snprintf(buf, sizeof(buf), "Target: [%02d] C   ", motor->target_circles);
    OLED_ShowString(0, 1, (uint8_t *)buf);

    // 第2行:当前圈数
    snprintf(buf, sizeof(buf), "Current: %.2f C  ", motor->current_circles);
    OLED_ShowString(0, 2, (uint8_t *)buf);

    // 第3行:剩余圈数 或 操作提示
    if (motor->is_running) {
        snprintf(buf, sizeof(buf), "Remain: %.2f C  ", motor->remain_circles);
        OLED_ShowString(0, 3, (uint8_t *)buf);
    } else {
        // 停止状态显示操作提示
        if (motor->circle_state == CIRCLE_FINISHED) {
            OLED_ShowString(0, 3, (uint8_t *)"Status: Done!   ");
        } else {
            OLED_ShowString(0, 3, (uint8_t *)"[1/2]Set [3]Run ");
        }
    }
}

/**
 * @brief 绘制系统信息页面
 */
void UI_Page_DrawSystemInfo(void)
{
    char buf[22];
    extern Encoder left_encoder;
    extern Encoder right_encoder;

    OLED_ShowString(0, 0, (uint8_t *)"Encoder Test[6/7]");

    // 显示左轮编码器累计脉冲
    snprintf(buf, sizeof(buf), "Left: %d     ", (int)left_encoder.total_count);
    OLED_ShowString(0, 1, (uint8_t *)buf);

    // 显示右轮编码器累计脉冲
    snprintf(buf, sizeof(buf), "Right:%d     ", (int)right_encoder.total_count);
    OLED_ShowString(0, 2, (uint8_t *)buf);

    // 操作提示
    OLED_ShowString(0, 3, (uint8_t *)"Rotate 1 circle!");
}

/**
 * @brief 绘制参数设置页面
 */
void UI_Page_DrawSettings(void)
{
    OLED_ShowString(0, 0, (uint8_t *)"Settings   [7/7]");
    OLED_ShowString(0, 1, (uint8_t *)">PID_Kp: 10.0");
    OLED_ShowString(0, 2, (uint8_t *)" PID_Ki: 0.1");
    OLED_ShowString(0, 3, (uint8_t *)" PID_Kd: 0.0");
}
