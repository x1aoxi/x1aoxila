#include "motor_app.h"
#include "pid_app.h"
#include "encoder_app.h"

// ============================= 外部变量引用 =============================
extern Encoder left_encoder;
extern Encoder right_encoder;
extern PID_T pid_speed_left;
extern PID_T pid_speed_right;
extern unsigned char pid_running;

// 选择用于圈数控制的编码器 (只有右轮电机,使用右轮编码器)
#define CIRCLE_CONTROL_ENCODER  right_encoder

// ============================= 电机硬件实例 =============================
MOTOR left_motor;
MOTOR right_motor;

// ============================= 电机状态 =============================
static MotorState motor_state = {
    .mode = MOTOR_MODE_IDLE,
    .is_running = 0,
    .direction = MOTOR_DIR_FORWARD,
    .basic_speed = 40.0f,
    .current_gear = SPEED_GEAR_LOW,
    .target_rpm = 30.0f,
    .accel_mode = ACCEL_MODE_LOW,
    .accel_target_rpm = 30.0f,
    .trapezoid_phase = TRAPEZOID_IDLE,
    .trapezoid_timer = 0,
    .trapezoid_current_rpm = 30.0f,
    .circle_state = CIRCLE_IDLE,
    .target_circles = 5,
    .start_total_count = 0,
    .current_circles = 0.0f,
    .remain_circles = 0.0f,
    .current_rpm = 0.0f,
    .left_rpm = 0.0f,
    .right_rpm = 0.0f
};

// 三档转速定义
static const float gear_speeds[] = {30.0f, 50.0f, 80.0f};

// 加速度定义(rpm/s)
static const float accel_rates[] = {5.0f, 20.0f};  // Low: 5rpm/s, High: 20rpm/s

// 梯形曲线参数定义
#define TRAPEZOID_START_RPM     30.0f   // 起始转速
#define TRAPEZOID_ACCEL_RATE    7.0f   // 加速度 (rpm/s)
#define TRAPEZOID_DECEL_RATE    7.0f   // 减速度 (rpm/s)

// 固定时间参数
#define TRAPEZOID_ACCEL_TIME_S  5.0f    // 加速时间(秒) - 固定
#define TRAPEZOID_CONST_TIME_S  3.0f    // 恒速时间(秒)
#define TRAPEZOID_DECEL_TIME_S  5.0f    // 减速时间(秒) - 固定

// 根据加速度和时间计算最大转速
#define TRAPEZOID_MAX_RPM       (TRAPEZOID_START_RPM + TRAPEZOID_ACCEL_RATE * TRAPEZOID_ACCEL_TIME_S)  // 30 + 10*3 = 60 rpm

// 圈数控制参数定义
#define CIRCLE_CONTROL_SPEED    30.0f   // 圈数控制运行速度(rpm) - 使用中速档
#define ENCODER_PPR_VALUE       1551    // 每圈脉冲数(实际测量值)

// ============================= 初始化函数 =============================

/**
 * @brief 初始化电机硬件
 */
void Motor_Init(void)
{
    Motor_Config_Init(&left_motor, &htim1, TIM_CHANNEL_2, &htim1, TIM_CHANNEL_1, 1,  550);
    Motor_Config_Init(&right_motor, &htim1, TIM_CHANNEL_4, &htim1, TIM_CHANNEL_3, 0, 550);
}

// ============================= 内部辅助函数 =============================

/**
 * @brief 更新实时反馈数据
 * @note 从编码器读取当前转速
 */
static void Motor_UpdateFeedback(void)
{
    motor_state.left_rpm = left_encoder.rpm;
    motor_state.right_rpm = right_encoder.rpm;
    // 使用滤波后的转速,减少OLED显示跳动
    motor_state.current_rpm = (left_encoder.rpm_filtered + right_encoder.rpm_filtered) / 2.0f;
}

/**
 * @brief 更新PID目标值
 * @param target_rpm 目标转速(考虑方向)
 */
static void Motor_UpdatePIDTarget(float target_rpm)
{
    pid_set_target(&pid_speed_left, target_rpm);
    pid_set_target(&pid_speed_right, target_rpm);
}

/**
 * @brief RPM转换为PWM值
 * @param rpm 目标转速
 * @return PWM值
 * @note 使用线性插值: 已测试 PWM=550→30rpm, PWM=750→50rpm
 *       斜率 k = (50-30)/(750-550) = 0.1
 *       rpm = 0.1 * pwm - 25
 *       pwm = (rpm + 25) * 10
 */
static int Motor_RPM_to_PWM(float rpm)
{
    int pwm = (int)((rpm + 25.0f) * 10.0f);

    // PWM限幅 (最小100, 最大999)
    if (pwm < 100) pwm = 100;
    if (pwm > 999) pwm = 999;

    return pwm;
}

// ============================= 任务函数 =============================

/**
 * @brief 电机控制任务(每10ms调用一次)
 * @note 更新实时反馈数据
 */
void Motor_Task(void)
{
    // 更新反馈数据
    Motor_UpdateFeedback();

    // 根据当前模式执行特定逻辑
    switch (motor_state.mode) {
        case MOTOR_MODE_IDLE:
            // 空闲模式,无需操作
            break;

        case MOTOR_MODE_BASIC_RUN:
            // 基本运行模式,PID自动控制
            break;

        case MOTOR_MODE_SPEED_GEAR:
            // 三档转速模式,PID自动控制
            break;

        case MOTOR_MODE_ACCELERATION:
            // 加速度测试模式: 持续加速
            if (motor_state.is_running) {
                // 每10ms增加转速
                float accel_rate = accel_rates[motor_state.accel_mode];  // rpm/s
                float delta_rpm = accel_rate * 0.01f;  // 10ms增量 (Low:0.05rpm, High:0.2rpm)

                motor_state.accel_target_rpm += delta_rpm;

                // 限制最大转速为210rpm
                if (motor_state.accel_target_rpm > 210.0f) {
                    motor_state.accel_target_rpm = 210.0f;
                }

                // 将目标转速转换为PWM并设置
                int pwm_value = Motor_RPM_to_PWM(motor_state.accel_target_rpm);
                Motor_Set_Speed(&left_motor, pwm_value);
                Motor_Set_Speed(&right_motor, pwm_value);
            }
            break;

        case MOTOR_MODE_TRAPEZOID:
            // 梯形曲线模式: 加速3s → 恒速3s → 减速3s
            if (motor_state.is_running) {
                motor_state.trapezoid_timer++;  // 每10ms递增

                switch (motor_state.trapezoid_phase) {
                    case TRAPEZOID_ACCEL:
                        // 加速阶段 (5秒 = 500次)
                        {
                            float delta_rpm = TRAPEZOID_ACCEL_RATE * 0.01f;  // 每10ms增量: 10*0.01=0.1rpm
                            motor_state.trapezoid_current_rpm += delta_rpm;

                            // 检查是否达到最大转速或超时
                            if (motor_state.trapezoid_current_rpm >= TRAPEZOID_MAX_RPM ||
                                motor_state.trapezoid_timer >= 500) {  // 5秒
                                motor_state.trapezoid_current_rpm = TRAPEZOID_MAX_RPM;
                                motor_state.trapezoid_phase = TRAPEZOID_CONST;
                                motor_state.trapezoid_timer = 0;
                            }

                            int pwm_value = Motor_RPM_to_PWM(motor_state.trapezoid_current_rpm);
                            Motor_Set_Speed(&left_motor, pwm_value);
                            Motor_Set_Speed(&right_motor, pwm_value);
                        }
                        break;

                    case TRAPEZOID_CONST:
                        // 恒速阶段 (3秒 = 300次)
                        if (motor_state.trapezoid_timer >= 300) {  // 3秒
                            motor_state.trapezoid_phase = TRAPEZOID_DECEL;
                            motor_state.trapezoid_timer = 0;
                        }
                        // 保持最大转速
                        break;

                    case TRAPEZOID_DECEL:
                        // 减速阶段 (5秒 = 500次)
                        {
                            float delta_rpm = TRAPEZOID_DECEL_RATE * 0.01f;  // 每10ms减量: 10*0.01=0.1rpm
                            motor_state.trapezoid_current_rpm -= delta_rpm;

                            // 检查是否降到起始转速或超时
                            if (motor_state.trapezoid_current_rpm <= TRAPEZOID_START_RPM ||
                                motor_state.trapezoid_timer >= 500) {  // 5秒
                                motor_state.trapezoid_current_rpm = TRAPEZOID_START_RPM;
                                motor_state.trapezoid_phase = TRAPEZOID_FINISHED;
                                motor_state.trapezoid_timer = 0;
                                // 自动停止
                                MotorApp_Stop();
                            }

                            if (motor_state.is_running) {  // 检查是否被Stop中断
                                int pwm_value = Motor_RPM_to_PWM(motor_state.trapezoid_current_rpm);
                                Motor_Set_Speed(&left_motor, pwm_value);
                                Motor_Set_Speed(&right_motor, pwm_value);
                            }
                        }
                        break;

                    default:
                        break;
                }
            }
            break;

        case MOTOR_MODE_CIRCLE_CONTROL:
            // 圈数控制模式: 监控右轮编码器脉冲数
            if (motor_state.is_running && motor_state.circle_state == CIRCLE_RUNNING) {
                // 计算当前已转脉冲数 (使用右轮编码器)
                int32_t delta_count = CIRCLE_CONTROL_ENCODER.total_count - motor_state.start_total_count;

                // 处理正反转(取绝对值)
                if (delta_count < 0) delta_count = -delta_count;

                // 计算当前圈数和剩余圈数(用于显示)
                motor_state.current_circles = (float)delta_count / ENCODER_PPR_VALUE;
                motor_state.remain_circles = motor_state.target_circles - motor_state.current_circles;
                if (motor_state.remain_circles < 0.0f) {
                    motor_state.remain_circles = 0.0f;
                }

                // 关键逻辑: 当脉冲数达到目标时停止
                // 目标脉冲数 = 目标圈数 × 每圈脉冲数
                int32_t target_pulses = motor_state.target_circles * ENCODER_PPR_VALUE;

                if (delta_count >= target_pulses) {
                    motor_state.circle_state = CIRCLE_FINISHED;
                    MotorApp_Stop();
                }
            }
            break;

        default:
            break;
    }
}

// ============================= 模式控制接口 =============================

/**
 * @brief 设置运动模式
 * @param mode 目标模式
 */
void MotorApp_SetMode(MotorMode mode)
{
    // 切换模式前先停止电机
    if (motor_state.is_running) {
        MotorApp_Stop();
    }

    motor_state.mode = mode;
}

/**
 * @brief 启动电机
 */
void MotorApp_Start(void)
{
    if (motor_state.is_running) return;  // 已经在运行

    motor_state.is_running = 1;
    // 暂时不启动PID,使用开环控制
    // pid_running = 1;

    // 根据当前模式设置电机PWM
    int pwm_value = 0;

    switch (motor_state.mode) {
        case MOTOR_MODE_BASIC_RUN:
            // 基本运行:直接设置固定PWM (30rpm约需要200-300 PWM)
            pwm_value = 600;  // 可以根据实际测试调整
            if (motor_state.direction == MOTOR_DIR_REVERSE) {
                pwm_value = -pwm_value;
            }
            Motor_Set_Speed(&left_motor, pwm_value);
            Motor_Set_Speed(&right_motor, pwm_value);
            break;

        case MOTOR_MODE_SPEED_GEAR:
            // 三档转速:根据档位设置不同PWM
            // Low(30rpm)→600, Mid(50rpm)→640, High(80rpm)→705
            switch (motor_state.current_gear) {
                case SPEED_GEAR_LOW:
                    pwm_value = 600;
                    break;
                case SPEED_GEAR_MID:
                    pwm_value = 640;
                    break;
                case SPEED_GEAR_HIGH:
                    pwm_value = 710;
                    break;
            }
            Motor_Set_Speed(&left_motor, pwm_value);
            Motor_Set_Speed(&right_motor, pwm_value);
            break;

        case MOTOR_MODE_ACCELERATION:
            // 加速度模式: 从30rpm开始
            motor_state.accel_target_rpm = 30.0f;
            pwm_value = Motor_RPM_to_PWM(30.0f);  // 30rpm对应的PWM
            Motor_Set_Speed(&left_motor, pwm_value);
            Motor_Set_Speed(&right_motor, pwm_value);
            break;

        case MOTOR_MODE_TRAPEZOID:
            // 梯形曲线模式: 初始化状态机
            motor_state.trapezoid_phase = TRAPEZOID_ACCEL;
            motor_state.trapezoid_timer = 0;
            motor_state.trapezoid_current_rpm = TRAPEZOID_START_RPM;
            pwm_value = Motor_RPM_to_PWM(TRAPEZOID_START_RPM);  // 从30rpm开始
            Motor_Set_Speed(&left_motor, pwm_value);
            Motor_Set_Speed(&right_motor, pwm_value);
            break;

        case MOTOR_MODE_CIRCLE_CONTROL:
            // 圈数控制模式: 记录右轮编码器起始脉冲数并启动
            motor_state.circle_state = CIRCLE_RUNNING;
            motor_state.start_total_count = CIRCLE_CONTROL_ENCODER.total_count;
            motor_state.current_circles = 0.0f;
            motor_state.remain_circles = motor_state.target_circles;

            pwm_value = Motor_RPM_to_PWM(CIRCLE_CONTROL_SPEED);
            Motor_Set_Speed(&left_motor, pwm_value);
            Motor_Set_Speed(&right_motor, pwm_value);
            break;

        default:
            break;
    }
}

/**
 * @brief 停止电机
 */
void MotorApp_Stop(void)
{
    if (!motor_state.is_running) return;  // 已经停止

    motor_state.is_running = 0;
    // pid_running = 0;  // PID已禁用

    // 直接停止电机
    Motor_Set_Speed(&left_motor, 0);
    Motor_Set_Speed(&right_motor, 0);

    // 清零反馈数据
    motor_state.current_rpm = 0.0f;

    // 如果是梯形曲线模式,重置状态机
    if (motor_state.mode == MOTOR_MODE_TRAPEZOID) {
        motor_state.trapezoid_phase = TRAPEZOID_IDLE;
        motor_state.trapezoid_timer = 0;
    }

    // 如果是圈数控制模式,重置状态
    if (motor_state.mode == MOTOR_MODE_CIRCLE_CONTROL) {
        motor_state.circle_state = CIRCLE_IDLE;
        // 保留圈数显示数据,不清零(便于查看最终结果)
    }
}

// ============================= Basic Run 模式接口 =============================

/**
 * @brief 设置运动方向
 * @param dir 目标方向
 */
void MotorApp_BasicRun_SetDirection(MotorDirection dir)
{
    motor_state.direction = dir;

    // 如果正在运行,立即更新电机PWM(开环控制)
    if (motor_state.is_running && motor_state.mode == MOTOR_MODE_BASIC_RUN) {
        int pwm_value = 640;  // 与MotorApp_Start()中保持一致
        if (dir == MOTOR_DIR_REVERSE) {
            pwm_value = -pwm_value;
        }
        Motor_Set_Speed(&left_motor, pwm_value);
        Motor_Set_Speed(&right_motor, pwm_value);
    }
}

/**
 * @brief 设置基础速度
 * @param speed_rpm 目标速度(rpm)
 */
void MotorApp_BasicRun_SetSpeed(float speed_rpm)
{
    motor_state.basic_speed = speed_rpm;

    // 如果正在运行,立即更新PID目标
    if (motor_state.is_running && motor_state.mode == MOTOR_MODE_BASIC_RUN) {
        float target = speed_rpm;
        if (motor_state.direction == MOTOR_DIR_REVERSE) {
            target = -target;
        }
        Motor_UpdatePIDTarget(target);
    }
}

// ============================= Speed Gear 模式接口 =============================

/**
 * @brief 设置速度档位
 * @param gear 目标档位
 */
void MotorApp_SpeedGear_SetGear(SpeedGear gear)
{
    if (gear > SPEED_GEAR_HIGH) return;  // 超出范围

    motor_state.current_gear = gear;
    motor_state.target_rpm = gear_speeds[gear];

    // 如果正在运行,立即更新电机PWM(开环控制)
    if (motor_state.is_running && motor_state.mode == MOTOR_MODE_SPEED_GEAR) {
        int pwm_value = 0;
        // 根据档位设置PWM值(与MotorApp_Start()中保持一致)
        switch (gear) {
            case SPEED_GEAR_LOW:
                pwm_value = 600;
                break;
            case SPEED_GEAR_MID:
                pwm_value = 640;
                break;
            case SPEED_GEAR_HIGH:
                pwm_value = 710;
                break;
        }
        Motor_Set_Speed(&left_motor, pwm_value);
        Motor_Set_Speed(&right_motor, pwm_value);
    }
}

/**
 * @brief 增加档位(Low→Mid→High)
 */
void MotorApp_SpeedGear_IncreaseGear(void)
{
    if (motor_state.current_gear < SPEED_GEAR_HIGH) {
        MotorApp_SpeedGear_SetGear(motor_state.current_gear + 1);
    }
}

/**
 * @brief 减少档位(High→Mid→Low)
 */
void MotorApp_SpeedGear_DecreaseGear(void)
{
    if (motor_state.current_gear > SPEED_GEAR_LOW) {
        MotorApp_SpeedGear_SetGear(motor_state.current_gear - 1);
    }
}

// ============================= Acceleration 模式接口 =============================

/**
 * @brief 设置加速度模式
 * @param mode 加速度模式(Low/High)
 */
void MotorApp_Acceleration_SetMode(AccelMode mode)
{
    motor_state.accel_mode = mode;
}

/**
 * @brief 切换加速度模式(Low ↔ High)
 */
void MotorApp_Acceleration_ToggleMode(void)
{
    motor_state.accel_mode = (motor_state.accel_mode == ACCEL_MODE_LOW) ?
                              ACCEL_MODE_HIGH : ACCEL_MODE_LOW;
}

// ============================= Circle Control 模式接口 =============================

/**
 * @brief 设置目标圈数
 * @param circles 目标圈数(1-20)
 */
void MotorApp_CircleControl_SetTarget(uint8_t circles)
{
    // 限制范围 1-20
    if (circles < 1) circles = 1;
    if (circles > 20) circles = 20;

    motor_state.target_circles = circles;
}

/**
 * @brief 增加目标圈数(+1圈,最大20)
 */
void MotorApp_CircleControl_IncreaseTarget(void)
{
    if (motor_state.target_circles < 20) {
        motor_state.target_circles++;
    }
}

/**
 * @brief 减少目标圈数(-1圈,最小1)
 */
void MotorApp_CircleControl_DecreaseTarget(void)
{
    if (motor_state.target_circles > 1) {
        motor_state.target_circles--;
    }
}

// ============================= 状态查询接口 =============================

/**
 * @brief 获取电机状态指针
 * @return 电机状态结构体指针
 */
MotorState* MotorApp_GetState(void)
{
    return &motor_state;
}

/**
 * @brief 获取当前平均转速
 * @return 当前转速(rpm)
 */
float MotorApp_GetCurrentRPM(void)
{
    return motor_state.current_rpm;
}

/**
 * @brief 查询电机是否在运行
 * @return 1=运行中, 0=停止
 */
uint8_t MotorApp_IsRunning(void)
{
    return motor_state.is_running;
}
