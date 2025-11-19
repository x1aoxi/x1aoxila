#ifndef __UI_ANIMATION_APP_H
#define __UI_ANIMATION_APP_H

#include "stm32f4xx.h"
#include <stdbool.h>

// ============================= 动画类型枚举 =============================
typedef enum {
    ANIM_TYPE_NONE = 0,
    ANIM_TYPE_SCROLL,       // 滚动动画
    ANIM_TYPE_FADE,         // 淡入淡出动画
    ANIM_TYPE_CURSOR        // 光标移动动画
} AnimationType;

// ============================= 动画状态结构 =============================
typedef struct {
    AnimationType type;      // 动画类型
    int16_t start_pos;       // 起始位置
    int16_t end_pos;         // 结束位置
    int16_t current_pos;     // 当前位置
    uint16_t duration_ms;    // 动画总时长(毫秒)
    uint16_t elapsed_ms;     // 已过时间(毫秒)
    bool is_running;         // 是否正在运行
} Animation;

// ============================= 全局变量 =============================
extern Animation g_scroll_animation;
extern Animation g_fade_animation;

// ============================= 函数声明 =============================

/**
 * @brief 动画系统初始化
 */
void UI_Animation_Init(void);

/**
 * @brief 更新动画状态(每帧调用)
 * @param delta_ms 时间增量(毫秒)
 */
void UI_Animation_Update(uint8_t delta_ms);

/**
 * @brief 启动滚动动画
 * @param offset 滚动偏移量(正数向上,负数向下)
 */
void UI_Animation_StartScroll(int16_t offset);

/**
 * @brief 启动淡入淡出动画
 */
void UI_Animation_StartFade(void);

/**
 * @brief 检查是否正在滚动
 * @return true:正在滚动, false:未滚动
 */
bool UI_Animation_IsScrolling(void);

/**
 * @brief 获取当前滚动偏移量
 * @return 滚动偏移量(像素)
 */
int16_t UI_Animation_GetScrollOffset(void);

/**
 * @brief 检查是否正在淡入淡出
 * @return true:正在淡入淡出, false:未淡入淡出
 */
bool UI_Animation_IsFading(void);

/**
 * @brief 获取淡入淡出透明度(0-255)
 * @return 透明度值
 */
uint8_t UI_Animation_GetFadeAlpha(void);

#endif // __UI_ANIMATION_APP_H
