#include "ui_animation_app.h"
#include <math.h>

// ============================= 全局变量 =============================
Animation g_scroll_animation = {
    .type = ANIM_TYPE_SCROLL,
    .start_pos = 0,
    .end_pos = 0,
    .current_pos = 0,
    .duration_ms = 200,     // 滚动动画持续200ms
    .elapsed_ms = 0,
    .is_running = false
};

Animation g_fade_animation = {
    .type = ANIM_TYPE_FADE,
    .start_pos = 0,
    .end_pos = 255,
    .current_pos = 255,
    .duration_ms = 300,     // 淡入淡出持续300ms
    .elapsed_ms = 0,
    .is_running = false
};

// ============================= 缓动函数 =============================

/**
 * @brief 三次缓动函数(Ease In Out Cubic)
 * @param t 进度(0.0 ~ 1.0)
 * @return 缓动后的进度(0.0 ~ 1.0)
 */
static float ease_in_out_cubic(float t)
{
    if (t < 0.5f) {
        return 4.0f * t * t * t;
    } else {
        float f = (2.0f * t - 2.0f);
        return 1.0f + 0.5f * f * f * f;
    }
}

/**
 * @brief 二次缓动函数(Ease Out Quad)
 * @param t 进度(0.0 ~ 1.0)
 * @return 缓动后的进度(0.0 ~ 1.0)
 */
static float ease_out_quad(float t)
{
    return t * (2.0f - t);
}

// ============================= 动画更新函数 =============================

/**
 * @brief 更新单个动画的状态
 * @param anim 动画结构体指针
 * @param delta_ms 时间增量(毫秒)
 */
static void update_animation(Animation *anim, uint8_t delta_ms)
{
    if (!anim->is_running) return;

    // 累加已过时间
    anim->elapsed_ms += delta_ms;

    // 计算进度(0.0 ~ 1.0)
    float progress = (float)anim->elapsed_ms / (float)anim->duration_ms;
    if (progress > 1.0f) {
        progress = 1.0f;
    }

    // 根据动画类型应用不同的缓动函数
    float eased_progress;
    if (anim->type == ANIM_TYPE_SCROLL) {
        eased_progress = ease_out_quad(progress);  // 滚动使用二次缓动
    } else {
        eased_progress = ease_in_out_cubic(progress);  // 淡入淡出使用三次缓动
    }

    // 计算当前位置
    int16_t delta = anim->end_pos - anim->start_pos;
    anim->current_pos = anim->start_pos + (int16_t)(delta * eased_progress);

    // 检查动画是否结束
    if (progress >= 1.0f) {
        anim->is_running = false;
        anim->current_pos = anim->end_pos;
        anim->elapsed_ms = 0;
    }
}

// ============================= 函数实现 =============================

/**
 * @brief 动画系统初始化
 */
void UI_Animation_Init(void)
{
    // 初始化滚动动画
    g_scroll_animation.type = ANIM_TYPE_SCROLL;
    g_scroll_animation.start_pos = 0;
    g_scroll_animation.end_pos = 0;
    g_scroll_animation.current_pos = 0;
    g_scroll_animation.duration_ms = 200;
    g_scroll_animation.elapsed_ms = 0;
    g_scroll_animation.is_running = false;

    // 初始化淡入淡出动画
    g_fade_animation.type = ANIM_TYPE_FADE;
    g_fade_animation.start_pos = 0;
    g_fade_animation.end_pos = 255;
    g_fade_animation.current_pos = 255;
    g_fade_animation.duration_ms = 300;
    g_fade_animation.elapsed_ms = 0;
    g_fade_animation.is_running = false;
}

/**
 * @brief 更新动画状态(每帧调用)
 * @param delta_ms 时间增量(毫秒)
 */
void UI_Animation_Update(uint8_t delta_ms)
{
    // 更新滚动动画
    update_animation(&g_scroll_animation, delta_ms);

    // 更新淡入淡出动画
    update_animation(&g_fade_animation, delta_ms);
}

/**
 * @brief 启动滚动动画
 * @param offset 滚动偏移量(正数向上,负数向下)
 */
void UI_Animation_StartScroll(int16_t offset)
{
    // 如果正在滚动,先完成当前动画
    if (g_scroll_animation.is_running) {
        g_scroll_animation.start_pos = g_scroll_animation.current_pos;
    } else {
        g_scroll_animation.start_pos = 0;
    }

    // 设置新的目标位置
    g_scroll_animation.end_pos = g_scroll_animation.start_pos + offset;

    // 重置时间和启动动画
    g_scroll_animation.elapsed_ms = 0;
    g_scroll_animation.is_running = true;
}

/**
 * @brief 启动淡入淡出动画
 */
void UI_Animation_StartFade(void)
{
    // 淡出(从255到0)
    g_fade_animation.start_pos = 255;
    g_fade_animation.end_pos = 0;
    g_fade_animation.current_pos = 255;
    g_fade_animation.elapsed_ms = 0;
    g_fade_animation.is_running = true;
}

/**
 * @brief 检查是否正在滚动
 * @return true:正在滚动, false:未滚动
 */
bool UI_Animation_IsScrolling(void)
{
    return g_scroll_animation.is_running;
}

/**
 * @brief 获取当前滚动偏移量
 * @return 滚动偏移量(像素)
 */
int16_t UI_Animation_GetScrollOffset(void)
{
    // 如果正在滚动,返回当前位置
    if (g_scroll_animation.is_running) {
        return g_scroll_animation.current_pos;
    }

    // 如果滚动结束,逐渐回到0
    if (g_scroll_animation.current_pos != 0) {
        // 快速衰减到0
        if (g_scroll_animation.current_pos > 0) {
            g_scroll_animation.current_pos -= 2;
            if (g_scroll_animation.current_pos < 0) {
                g_scroll_animation.current_pos = 0;
            }
        } else {
            g_scroll_animation.current_pos += 2;
            if (g_scroll_animation.current_pos > 0) {
                g_scroll_animation.current_pos = 0;
            }
        }
    }

    return g_scroll_animation.current_pos;
}

/**
 * @brief 检查是否正在淡入淡出
 * @return true:正在淡入淡出, false:未淡入淡出
 */
bool UI_Animation_IsFading(void)
{
    return g_fade_animation.is_running;
}

/**
 * @brief 获取淡入淡出透明度(0-255)
 * @return 透明度值
 */
uint8_t UI_Animation_GetFadeAlpha(void)
{
    return (uint8_t)g_fade_animation.current_pos;
}
