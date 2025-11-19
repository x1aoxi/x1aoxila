#include "oled_app.h"
#include "ui_menu_app.h"  // 引入UI菜单系统

void Oled_Init(void)
{
    OLED_Init();
    OLED_Clear();

    // 初始化UI菜单系统
    UI_Menu_Init();
}

extern MOTOR left_motor;
extern MOTOR right_motor;

extern Encoder left_encoder;
extern Encoder right_encoder;

void Oled_Task(void)
{
    // ============================= UI菜单系统模式 =============================

    // 更新UI菜单状态
    UI_Menu_Update();

    // 绘制UI菜单
    UI_Menu_Draw();

    // ============================= 调试输出(保留) =============================

    // 通过串口输出编码器数据(用于调试)
    static uint16_t uart_counter = 0;
    if (++uart_counter >= 10) {  // 每100ms输出一次(10ms*10)
        uart_counter = 0;
        Uart_Printf(DEBUG_UART, "L:%.2frpm %.2fcm/s, R:%.2frpm %.2fcm/s\r\n",
                    left_encoder.rpm, left_encoder.speed_cm_s,
                    right_encoder.rpm, right_encoder.speed_cm_s);
    }

    // ============================= 旧版本显示代码(已注释,备用) =============================

    // /* 测试灰度传感器 */
    // Oled_Printf(0, 0, "%d-%d-%d-%d-%d-%d-%d-%d",
    //             (gray_digtal>>0)&0x01,(gray_digtal>>1)&0x01,(gray_digtal>>2)&0x01,(gray_digtal>>3)&0x01,
    //             (gray_digtal>>4)&0x01,(gray_digtal>>5)&0x01,(gray_digtal>>6)&0x01,(gray_digtal>>7)&0x01);

    // /* 测试电机 */
    // Oled_Printf(0, 0, " left:%d  ", left_motor.speed);
    // Oled_Printf(0, 2, "right:%d  ", right_motor.speed);

    // /* 测试编码器 */
    // Oled_Printf(0, 0, "L_rpm:%.2fcm/s ", left_encoder.rpm);
    // Oled_Printf(0, 1, " left:%.2fcm/s ", left_encoder.speed_cm_s);
    // Oled_Printf(0, 2, "R_rpm:%.2fcm/s ", right_encoder.rpm);
    // Oled_Printf(0, 3, "right:%.2fcm/s ", right_encoder.speed_cm_s);
}
