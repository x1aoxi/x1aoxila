#ifndef __MOTOR_APP_H__
#define __MOTOR_APP_H__

#include "MyDefine.h"

// ============================= 运动模式定义 =============================

/**
 * @brief 电机运动模式
 */
typedef enum {
    MOTOR_MODE_IDLE = 0,           // 空闲(停止)
    MOTOR_MODE_BASIC_RUN,          // 基本运行模式
    MOTOR_MODE_SPEED_GEAR,         // 三档转速模式
    MOTOR_MODE_ACCELERATION,       // 加速度测试模式
    MOTOR_MODE_TRAPEZOID,          // 梯形曲线模式
    MOTOR_MODE_CIRCLE_CONTROL      // 精准圈数控制模式
} MotorMode;

/**
 * @brief 电机方向
 */
typedef enum {
    MOTOR_DIR_FORWARD = 0,         // 正转
    MOTOR_DIR_REVERSE = 1          // 反转
} MotorDirection;

/**
 * @brief 速度档位
 */
typedef enum {
    SPEED_GEAR_LOW = 0,            // 低速档 (30rpm)
    SPEED_GEAR_MID = 1,            // 中速档 (50rpm)
    SPEED_GEAR_HIGH = 2            // 高速档 (80rpm)
} SpeedGear;

/**
 * @brief 加速度模式
 */
typedef enum {
    ACCEL_MODE_LOW = 0,            // 低加速度 (5 rpm/s)
    ACCEL_MODE_HIGH = 1            // 高加速度 (20 rpm/s)
} AccelMode;

/**
 * @brief 梯形曲线运动阶段
 */
typedef enum {
    TRAPEZOID_IDLE = 0,            // 空闲状态
    TRAPEZOID_ACCEL,               // 加速阶段
    TRAPEZOID_CONST,               // 恒速阶段
    TRAPEZOID_DECEL,               // 减速阶段
    TRAPEZOID_FINISHED             // 完成(已停止)
} TrapezoidPhase;

/**
 * @brief 圈数控制运行状态
 */
typedef enum {
    CIRCLE_IDLE = 0,               // 空闲状态
    CIRCLE_RUNNING,                // 运行中
    CIRCLE_FINISHED                // 完成
} CircleState;

// ============================= 电机状态结构体 =============================

/**
 * @brief 电机运动状态
 */
typedef struct {
    MotorMode mode;                // 当前运动模式
    uint8_t is_running;            // 运行状态: 0=停止, 1=运行

    // Basic Run 模式参数
    MotorDirection direction;      // 运动方向
    float basic_speed;             // 基础速度(rpm)

    // Speed Gear 模式参数
    SpeedGear current_gear;        // 当前档位
    float target_rpm;              // 目标转速(rpm)

    // Acceleration 模式参数
    AccelMode accel_mode;          // 加速度模式(Low/High)
    float accel_target_rpm;        // 加速目标转速(持续增长)

    // Trapezoid 模式参数
    TrapezoidPhase trapezoid_phase;  // 当前运动阶段
    uint32_t trapezoid_timer;        // 阶段计时器(单位:10ms)
    float trapezoid_current_rpm;     // 当前目标转速

    // Circle Control 模式参数
    CircleState circle_state;        // 运行状态
    uint8_t target_circles;          // 目标圈数(1-20)
    int32_t start_total_count;       // 启动时编码器累计值
    float current_circles;           // 当前已转圈数(用于显示)
    float remain_circles;            // 剩余圈数(用于显示)

    // 实时反馈数据
    float current_rpm;             // 当前平均转速(rpm)
    float left_rpm;                // 左轮转速
    float right_rpm;               // 右轮转速

} MotorState;

// ============================= 外部接口函数 =============================

void Motor_Init(void);
void Motor_Task(void);

// 模式控制接口
void MotorApp_SetMode(MotorMode mode);
void MotorApp_Start(void);
void MotorApp_Stop(void);

// Basic Run 模式接口
void MotorApp_BasicRun_SetDirection(MotorDirection dir);
void MotorApp_BasicRun_SetSpeed(float speed_rpm);

// Speed Gear 模式接口
void MotorApp_SpeedGear_SetGear(SpeedGear gear);
void MotorApp_SpeedGear_IncreaseGear(void);
void MotorApp_SpeedGear_DecreaseGear(void);

// Acceleration 模式接口
void MotorApp_Acceleration_SetMode(AccelMode mode);
void MotorApp_Acceleration_ToggleMode(void);

// Circle Control 模式接口
void MotorApp_CircleControl_SetTarget(uint8_t circles);
void MotorApp_CircleControl_IncreaseTarget(void);
void MotorApp_CircleControl_DecreaseTarget(void);

// 状态查询接口
MotorState* MotorApp_GetState(void);
float MotorApp_GetCurrentRPM(void);
uint8_t MotorApp_IsRunning(void);

#endif
