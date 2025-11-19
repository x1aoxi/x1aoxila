# 07_Encoder - 编码器测量示例

[![HAL](https://img.shields.io/badge/HAL-STM32F4-brightgreen.svg)](/Drivers) [![Keil](https://img.shields.io/badge/IDE-Keil_MDK-blue.svg)](https://www.keil.com/) 

## 项目简介

07_Encoder是STM32F407VET6开发板的编码器测量示例工程，演示了如何使用STM32的编码器接口测量旋转编码器的位置和速度。该项目提供了完整的编码器处理方案，包括正交编码、计数处理、速度计算、方向判断等功能，适用于各种需要精确位置和速度测量的应用。

## 主要特性

- **正交解码**: 硬件自动解码A/B相正交信号
- **位置测量**: 精确的绝对和相对位置测量
- **速度计算**: 实时转速和线速度计算
- **方向检测**: 自动检测旋转方向(正转/反转)
- **多种单位**: 支持脉冲、角度、圈数等多种单位
- **滤波处理**: 软件滤波减少噪声影响
- **零位校准**: 支持零位设定和校准功能
- **数据记录**: 位置和速度数据记录与分析

## 硬件连接

### 增量式编码器连接

| 编码器引脚 | STM32引脚 | 功能描述 |
|-----------|-----------|----------|
| A相       | PA0 (TIM2_CH1) | 编码器A相信号 |
| B相       | PA1 (TIM2_CH2) | 编码器B相信号 |
| Z相(可选) | PA2       | 编码器Z相(零位信号) |
| VCC       | 5V/3.3V   | 编码器电源(根据规格) |
| GND       | GND       | 编码器地 |

### 编码器电路连接示例

```
     编码器                    STM32F407VET6
    ┌─────────┐               ┌──────────────┐
    │   VCC   │──────────────→│ 5V/3.3V      │
    │   GND   │──────────────→│ GND          │
    │    A    │──────────────→│ PA0(TIM2_CH1)│
    │    B    │──────────────→│ PA1(TIM2_CH2)│
    │   Z(Z相)│──────────────→│ PA2          │
    └─────────┘               └──────────────┘
```

### 常见编码器规格

| 参数 | 典型值 | 说明 |
|------|--------|------|
| 分辨率 | 360/600/1024 PPR | 每转脉冲数 |
| 工作电压 | 5V/12V/24V | 供电电压 |
| 输出类型 | 推挽/开漏 | 输出电路类型 |
| 频响 | 100kHz | 最大响应频率 |

## 目录结构

```
07_Encoder/
├── Core/
│   ├── Inc/                     # 核心头文件
│   │   ├── main.h
│   │   ├── gpio.h
│   │   ├── usart.h
│   │   ├── tim.h
│   │   └── dma.h
│   └── Src/                     # 核心源文件
│       ├── main.c
│       ├── gpio.c
│       ├── usart.c
│       ├── tim.c
│       └── dma.c
├── Drivers/
│   ├── CMSIS/                   # CMSIS库
│   └── STM32F4xx_HAL_Driver/    # HAL驱动库
├── MDK-ARM/
│   ├── 07_Encoder.uvprojx       # Keil工程文件
│   └── startup_stm32f407xx.s    # 启动文件
├── User/
│   ├── MyDefine.h               # 用户定义
│   ├── Scheduler.c              # 任务调度器
│   ├── Scheduler.h
│   ├── Scheduler_Task.c         # 任务实现
│   ├── Scheduler_Task.h
│   ├── App/
│   │   ├── encoder_app.c        # 编码器应用
│   │   ├── encoder_app.h
│   │   ├── position_control.c   # 位置控制
│   │   ├── position_control.h
│   │   ├── data_logger.c        # 数据记录
│   │   └── data_logger.h
│   ├── Driver/
│   │   ├── encoder_driver.c     # 编码器驱动
│   │   ├── encoder_driver.h
│   │   ├── speed_calc.c         # 速度计算
│   │   └── speed_calc.h
│   └── Module/
│       ├── filter.c             # 滤波算法
│       ├── filter.h
│       ├── calibration.c        # 校准功能
│       └── calibration.h
├── 07_Encoder.ioc              # CubeMX配置文件
└── keilkilll.bat               # Keil进程清理脚本
```

## 使用方法

### 快速开始

1. **硬件连接**
   - 按照连接表连接编码器
   - 确保电源电压匹配
   - 检查信号线连接正确

2. **编译下载**
   ```bash
   # 打开Keil工程
   双击 MDK-ARM/07_Encoder.uvprojx
   # 编译工程 (F7)
   # 下载程序 (F8)
   ```

3. **运行测试**
   - 打开串口终端(115200,8,N,1)
   - 旋转编码器观察数据变化
   - 发送命令进行功能测试

### 代码示例

#### 编码器驱动实现

```c
#include "encoder_driver.h"

/* 编码器数据结构 */
typedef struct {
    int32_t raw_count;          // 原始计数值
    int32_t position;           // 绝对位置(脉冲)
    float position_deg;         // 位置(度)
    float position_rad;         // 位置(弧度)
    int32_t turns;              // 圈数
    float speed_rpm;            // 转速(RPM)
    float speed_rps;            // 转速(RPS)
    int8_t direction;           // 方向: 1=正转, -1=反转, 0=停止
    uint32_t update_time;       // 更新时间戳
    uint8_t z_signal;           // Z相信号状态
} encoder_data_t;

encoder_data_t encoder_data;

/* 编码器配置参数 */
typedef struct {
    uint16_t ppr;               // 每转脉冲数
    float wheel_diameter;       // 轮径(mm), 用于线速度计算
    uint8_t reverse;            // 方向反向
    int32_t zero_offset;        // 零点偏移
} encoder_config_t;

encoder_config_t encoder_config = {
    .ppr = 360,                 // 默认360PPR
    .wheel_diameter = 100.0f,   // 默认轮径100mm
    .reverse = 0,               // 不反向
    .zero_offset = 0            // 零点偏移为0
};

#define ENCODER_TIMER_FREQ      84000000    // 定时器频率84MHz
#define SPEED_CALC_PERIOD       100         // 速度计算周期(ms)

/* 编码器初始化 */
void Encoder_Init(void)
{
    /* 配置编码器模式 */
    HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
    
    /* 设置定时器计数器初始值(中点值，便于双向计数) */
    __HAL_TIM_SET_COUNTER(&htim2, 32768);
    
    /* 初始化数据结构 */
    memset(&encoder_data, 0, sizeof(encoder_data));
    encoder_data.update_time = HAL_GetTick();
    
    /* 配置Z相中断(如果使用) */
    HAL_GPIO_EXTI_RegisterCallback(&GPIO_EXTI_Callback);
    
    printf("Encoder Initialized:\\r\\n");
    printf("  PPR: %d\\r\\n", encoder_config.ppr);
    printf("  Wheel Diameter: %.1f mm\\r\\n", encoder_config.wheel_diameter);
    printf("  Timer Counter Range: 0-65535\\r\\n");
    printf("  Center Value: 32768\\r\\n");
}

/* 读取编码器原始计数 */
int32_t Encoder_Get_Raw_Count(void)
{
    uint16_t tim_count = __HAL_TIM_GET_COUNTER(&htim2);
    encoder_data.raw_count = (int32_t)tim_count - 32768;  // 转换为有符号数
    
    return encoder_data.raw_count;
}

/* 更新编码器位置 */
void Encoder_Update_Position(void)
{
    static int32_t last_raw_count = 0;
    
    int32_t current_raw = Encoder_Get_Raw_Count();
    int32_t delta = current_raw - last_raw_count;
    
    /* 处理计数器溢出 */
    if(delta > 32767)
    {
        delta -= 65536;
    }
    else if(delta < -32767)
    {
        delta += 65536;
    }
    
    /* 应用方向反向 */
    if(encoder_config.reverse)
    {
        delta = -delta;
    }
    
    /* 更新绝对位置 */
    encoder_data.position += delta;
    
    /* 计算角度位置 */
    encoder_data.position_deg = (float)encoder_data.position * 360.0f / encoder_config.ppr;
    encoder_data.position_rad = encoder_data.position_deg * M_PI / 180.0f;
    
    /* 计算圈数 */
    encoder_data.turns = encoder_data.position / encoder_config.ppr;
    
    /* 更新上次计数值 */
    last_raw_count = current_raw;
}

/* 计算编码器速度 */
void Encoder_Calculate_Speed(void)
{
    static int32_t last_position = 0;
    static uint32_t last_time = 0;
    
    uint32_t current_time = HAL_GetTick();
    uint32_t delta_time = current_time - last_time;
    
    if(delta_time >= SPEED_CALC_PERIOD)
    {
        int32_t delta_position = encoder_data.position - last_position;
        
        if(delta_time > 0)
        {
            /* 计算转速 (RPM) */
            encoder_data.speed_rpm = (float)delta_position * 60000.0f / 
                                   (encoder_config.ppr * delta_time);
            
            /* 计算转速 (RPS) */
            encoder_data.speed_rps = encoder_data.speed_rpm / 60.0f;
            
            /* 计算方向 */
            if(delta_position > 1)
                encoder_data.direction = 1;   // 正转
            else if(delta_position < -1)
                encoder_data.direction = -1;  // 反转
            else
                encoder_data.direction = 0;   // 停止
        }
        
        /* 更新记录值 */
        last_position = encoder_data.position;
        last_time = current_time;
        encoder_data.update_time = current_time;
    }
}

/* 获取线速度 */
float Encoder_Get_Linear_Speed(void)
{
    /* 线速度 = 角速度 * 半径 */
    float circumference = M_PI * encoder_config.wheel_diameter;  // 周长(mm)
    float linear_speed_mm_s = encoder_data.speed_rps * circumference;  // mm/s
    
    return linear_speed_mm_s;
}

/* 获取线距离 */
float Encoder_Get_Linear_Distance(void)
{
    float circumference = M_PI * encoder_config.wheel_diameter;  // 周长(mm)
    float distance = (float)encoder_data.position * circumference / encoder_config.ppr;
    
    return distance;  // mm
}

/* Z相中断回调 */
void GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == GPIO_PIN_2)  // Z相信号
    {
        encoder_data.z_signal = 1;
        
        /* Z相校准(可选) */
        // Encoder_Zero_Calibration();
        
        printf("Z-Phase Signal Detected\\r\\n");
    }
}

/* 零点校准 */
void Encoder_Zero_Calibration(void)
{
    /* 记录当前位置作为零点偏移 */
    encoder_config.zero_offset = encoder_data.position;
    
    /* 重置位置计数 */
    encoder_data.position = 0;
    encoder_data.position_deg = 0;
    encoder_data.position_rad = 0;
    encoder_data.turns = 0;
    
    printf("Encoder Zero Calibration Complete\\r\\n");
}

/* 设置编码器参数 */
void Encoder_Set_Config(uint16_t ppr, float wheel_diameter, uint8_t reverse)
{
    encoder_config.ppr = ppr;
    encoder_config.wheel_diameter = wheel_diameter;
    encoder_config.reverse = reverse;
    
    printf("Encoder Config Updated:\\r\\n");
    printf("  PPR: %d\\r\\n", ppr);
    printf("  Wheel Diameter: %.1f mm\\r\\n", wheel_diameter);
    printf("  Reverse: %s\\r\\n", reverse ? "Yes" : "No");
}

/* 复位编码器 */
void Encoder_Reset(void)
{
    /* 重置定时器计数器 */
    __HAL_TIM_SET_COUNTER(&htim2, 32768);
    
    /* 重置数据结构 */
    memset(&encoder_data, 0, sizeof(encoder_data));
    encoder_data.update_time = HAL_GetTick();
    
    /* 重置配置 */
    encoder_config.zero_offset = 0;
    
    printf("Encoder Reset Complete\\r\\n");
}

/* 获取编码器数据 */
encoder_data_t* Encoder_Get_Data(void)
{
    Encoder_Update_Position();
    Encoder_Calculate_Speed();
    
    return &encoder_data;
}
```

#### 速度滤波算法

```c
#include "filter.h"

/* 滤波器类型 */
typedef enum {
    FILTER_NONE = 0,
    FILTER_AVERAGE,
    FILTER_LOWPASS,
    FILTER_KALMAN
} filter_type_t;

/* 移动平均滤波器 */
typedef struct {
    float buffer[10];           // 滤波缓存
    uint8_t index;              // 当前索引
    uint8_t count;              // 有效数据个数
    float sum;                  // 累加和
} moving_average_t;

/* 低通滤波器 */
typedef struct {
    float alpha;                // 滤波系数
    float last_output;          // 上次输出值
} lowpass_filter_t;

/* 卡尔曼滤波器 */
typedef struct {
    float x;                    // 状态估计
    float P;                    // 协方差
    float Q;                    // 过程噪声
    float R;                    // 测量噪声
} kalman_filter_t;

static moving_average_t speed_avg_filter;
static lowpass_filter_t speed_lp_filter = {0.2f, 0.0f};  // α=0.2
static kalman_filter_t speed_kalman = {0.0f, 1.0f, 0.01f, 0.1f};

/* 移动平均滤波 */
float Moving_Average_Filter(float input, moving_average_t* filter)
{
    /* 更新缓存 */
    filter->sum -= filter->buffer[filter->index];
    filter->buffer[filter->index] = input;
    filter->sum += input;
    
    /* 更新索引 */
    filter->index = (filter->index + 1) % 10;
    
    /* 更新计数 */
    if(filter->count < 10)
        filter->count++;
    
    /* 返回平均值 */
    return filter->sum / filter->count;
}

/* 低通滤波 */
float Lowpass_Filter(float input, lowpass_filter_t* filter)
{
    filter->last_output = filter->alpha * input + (1.0f - filter->alpha) * filter->last_output;
    return filter->last_output;
}

/* 卡尔曼滤波 */
float Kalman_Filter(float input, kalman_filter_t* filter)
{
    /* 预测步骤 */
    float x_pred = filter->x;
    float P_pred = filter->P + filter->Q;
    
    /* 更新步骤 */
    float K = P_pred / (P_pred + filter->R);  // 卡尔曼增益
    filter->x = x_pred + K * (input - x_pred);
    filter->P = (1.0f - K) * P_pred;
    
    return filter->x;
}

/* 速度滤波处理 */
float Speed_Filter_Process(float raw_speed, filter_type_t type)
{
    switch(type)
    {
        case FILTER_AVERAGE:
            return Moving_Average_Filter(raw_speed, &speed_avg_filter);
            
        case FILTER_LOWPASS:
            return Lowpass_Filter(raw_speed, &speed_lp_filter);
            
        case FILTER_KALMAN:
            return Kalman_Filter(raw_speed, &speed_kalman);
            
        default:
            return raw_speed;  // 无滤波
    }
}

/* 滤波器初始化 */
void Filter_Init(void)
{
    /* 初始化移动平均滤波器 */
    memset(&speed_avg_filter, 0, sizeof(speed_avg_filter));
    
    /* 初始化低通滤波器 */
    speed_lp_filter.last_output = 0.0f;
    
    /* 初始化卡尔曼滤波器 */
    speed_kalman.x = 0.0f;
    speed_kalman.P = 1.0f;
    
    printf("Speed Filters Initialized\\r\\n");
}
```

#### 位置控制实现

```c
#include "position_control.h"

/* 位置控制结构体 */
typedef struct {
    float target_position;      // 目标位置(度)
    float current_position;     // 当前位置(度)
    float position_error;       // 位置误差
    float speed_limit;          // 速度限制(RPM)
    uint8_t control_mode;       // 控制模式
    uint8_t target_reached;     // 目标到达标志
} position_control_t;

position_control_t pos_ctrl;

/* 位置控制初始化 */
void Position_Control_Init(void)
{
    pos_ctrl.target_position = 0.0f;
    pos_ctrl.current_position = 0.0f;
    pos_ctrl.position_error = 0.0f;
    pos_ctrl.speed_limit = 100.0f;  // 默认限速100RPM
    pos_ctrl.control_mode = 0;      // 默认关闭
    pos_ctrl.target_reached = 1;
    
    printf("Position Control Initialized\\r\\n");
}

/* 设置目标位置 */
void Position_Control_Set_Target(float target_deg)
{
    pos_ctrl.target_position = target_deg;
    pos_ctrl.target_reached = 0;
    
    printf("Position Target Set: %.2f degrees\\r\\n", target_deg);
}

/* 位置控制更新 */
void Position_Control_Update(void)
{
    encoder_data_t* encoder = Encoder_Get_Data();
    
    /* 更新当前位置 */
    pos_ctrl.current_position = encoder->position_deg;
    
    /* 计算位置误差 */
    pos_ctrl.position_error = pos_ctrl.target_position - pos_ctrl.current_position;
    
    /* 判断是否到达目标 */
    if(fabs(pos_ctrl.position_error) < 1.0f)  // 1度精度
    {
        if(!pos_ctrl.target_reached)
        {
            pos_ctrl.target_reached = 1;
            printf("Position Target Reached: %.2f degrees\\r\\n", pos_ctrl.current_position);
        }
    }
}

/* 获取位置控制状态 */
position_control_t* Position_Control_Get_Status(void)
{
    Position_Control_Update();
    return &pos_ctrl;
}
```

#### 数据记录功能

```c
#include "data_logger.h"

/* 数据记录结构 */
typedef struct {
    uint32_t timestamp;         // 时间戳
    int32_t position;           // 位置(脉冲)
    float position_deg;         // 位置(度)
    float speed_rpm;            // 转速(RPM)
    int8_t direction;           // 方向
} data_record_t;

#define MAX_RECORDS     1000
data_record_t data_log[MAX_RECORDS];
uint16_t log_index = 0;
uint8_t logging_enabled = 0;

/* 开始数据记录 */
void Data_Logger_Start(void)
{
    log_index = 0;
    logging_enabled = 1;
    
    printf("Data logging started\\r\\n");
}

/* 停止数据记录 */
void Data_Logger_Stop(void)
{
    logging_enabled = 0;
    printf("Data logging stopped. Records: %d\\r\\n", log_index);
}

/* 记录数据 */
void Data_Logger_Record(void)
{
    if(!logging_enabled || log_index >= MAX_RECORDS)
        return;
    
    encoder_data_t* encoder = Encoder_Get_Data();
    
    data_log[log_index].timestamp = HAL_GetTick();
    data_log[log_index].position = encoder->position;
    data_log[log_index].position_deg = encoder->position_deg;
    data_log[log_index].speed_rpm = encoder->speed_rpm;
    data_log[log_index].direction = encoder->direction;
    
    log_index++;
}

/* 导出数据 */
void Data_Logger_Export(void)
{
    printf("\\r\\n=== Data Export ===\\r\\n");
    printf("Time(ms),Position(pulse),Position(deg),Speed(RPM),Direction\\r\\n");
    
    for(uint16_t i = 0; i < log_index; i++)
    {
        printf("%ld,%ld,%.2f,%.2f,%d\\r\\n",
               data_log[i].timestamp,
               data_log[i].position,
               data_log[i].position_deg,
               data_log[i].speed_rpm,
               data_log[i].direction);
    }
    
    printf("=== Export Complete ===\\r\\n");
}

/* 清除记录 */
void Data_Logger_Clear(void)
{
    log_index = 0;
    logging_enabled = 0;
    
    printf("Data log cleared\\r\\n");
}
```

#### 串口命令处理

```c
/* 串口命令处理 */
void Encoder_Process_Command(char* cmd)
{
    encoder_data_t* data = Encoder_Get_Data();
    
    if(strncmp(cmd, "STATUS", 6) == 0)
    {
        printf("\\r\\n=== Encoder Status ===\\r\\n");
        printf("Raw Count: %ld\\r\\n", data->raw_count);
        printf("Position: %ld pulses\\r\\n", data->position);
        printf("Position: %.2f degrees\\r\\n", data->position_deg);
        printf("Position: %.4f radians\\r\\n", data->position_rad);
        printf("Turns: %ld\\r\\n", data->turns);
        printf("Speed: %.2f RPM\\r\\n", data->speed_rpm);
        printf("Speed: %.2f RPS\\r\\n", data->speed_rps);
        printf("Linear Speed: %.2f mm/s\\r\\n", Encoder_Get_Linear_Speed());
        printf("Linear Distance: %.2f mm\\r\\n", Encoder_Get_Linear_Distance());
        printf("Direction: %s\\r\\n", 
               data->direction == 1 ? "Forward" : 
               data->direction == -1 ? "Backward" : "Stop");
        printf("===================\\r\\n");
    }
    else if(strncmp(cmd, "RESET", 5) == 0)
    {
        Encoder_Reset();
    }
    else if(strncmp(cmd, "ZERO", 4) == 0)
    {
        Encoder_Zero_Calibration();
    }
    else if(strncmp(cmd, "CONFIG", 6) == 0)
    {
        int ppr;
        float diameter;
        int reverse;
        if(sscanf(cmd, "CONFIG %d %f %d", &ppr, &diameter, &reverse) == 3)
        {
            Encoder_Set_Config(ppr, diameter, reverse);
        }
        else
        {
            printf("Usage: CONFIG <ppr> <diameter> <reverse>\\r\\n");
        }
    }
    else if(strncmp(cmd, "LOG_START", 9) == 0)
    {
        Data_Logger_Start();
    }
    else if(strncmp(cmd, "LOG_STOP", 8) == 0)
    {
        Data_Logger_Stop();
    }
    else if(strncmp(cmd, "LOG_EXPORT", 10) == 0)
    {
        Data_Logger_Export();
    }
    else if(strncmp(cmd, "LOG_CLEAR", 9) == 0)
    {
        Data_Logger_Clear();
    }
    else
    {
        printf("Commands:\\r\\n");
        printf("  STATUS - Show encoder status\\r\\n");
        printf("  RESET - Reset encoder\\r\\n");
        printf("  ZERO - Zero calibration\\r\\n");
        printf("  CONFIG <ppr> <diameter> <reverse> - Set parameters\\r\\n");
        printf("  LOG_START/STOP/EXPORT/CLEAR - Data logging\\r\\n");
    }
}
```

## API参考

### 基础函数

```c
/* 编码器控制 */
void Encoder_Init(void);                               // 编码器初始化
int32_t Encoder_Get_Raw_Count(void);                  // 获取原始计数
void Encoder_Update_Position(void);                   // 更新位置
void Encoder_Calculate_Speed(void);                   // 计算速度
encoder_data_t* Encoder_Get_Data(void);               // 获取数据
void Encoder_Reset(void);                             // 复位编码器
```

### 配置函数

```c
/* 配置管理 */
void Encoder_Set_Config(uint16_t ppr, float wheel_diameter, uint8_t reverse);
void Encoder_Zero_Calibration(void);                  // 零点校准
```

### 计算函数

```c
/* 数据计算 */
float Encoder_Get_Linear_Speed(void);                 // 获取线速度
float Encoder_Get_Linear_Distance(void);              // 获取线距离
```

### 滤波函数

```c
/* 滤波处理 */
void Filter_Init(void);                               // 滤波器初始化
float Speed_Filter_Process(float raw_speed, filter_type_t type);
float Moving_Average_Filter(float input, moving_average_t* filter);
float Lowpass_Filter(float input, lowpass_filter_t* filter);
float Kalman_Filter(float input, kalman_filter_t* filter);
```

### 数据记录函数

```c
/* 数据记录 */
void Data_Logger_Start(void);                         // 开始记录
void Data_Logger_Stop(void);                          // 停止记录
void Data_Logger_Record(void);                        // 记录数据
void Data_Logger_Export(void);                        // 导出数据
void Data_Logger_Clear(void);                         // 清除记录
```

## 配置选项

### 编码器参数配置

```c
/* 在encoder_driver.h中配置 */
#define DEFAULT_PPR             360         // 默认每转脉冲数
#define DEFAULT_WHEEL_DIAMETER  100.0f      // 默认轮径(mm)
#define SPEED_CALC_PERIOD       100         // 速度计算周期(ms)
#define POSITION_THRESHOLD      1.0f        // 位置到达阈值(度)
```

### 滤波参数配置

```c
/* 滤波器配置 */
#define AVERAGE_FILTER_SIZE     10          // 移动平均滤波器大小
#define LOWPASS_ALPHA          0.2f         // 低通滤波系数
#define KALMAN_Q               0.01f        // 卡尔曼过程噪声
#define KALMAN_R               0.1f         // 卡尔曼测量噪声
```

### 数据记录配置

```c
/* 数据记录配置 */
#define MAX_RECORDS            1000         // 最大记录数
#define LOG_INTERVAL_MS        50           // 记录间隔(ms)
```

## 使用场景

1. **机器人导航**: 轮式机器人的里程计和定位系统
2. **工业自动化**: 传送带位置监控和控制
3. **精密测量**: 角度和位置的精确测量
4. **运动控制**: 伺服系统的位置和速度反馈
5. **实验教学**: 控制系统实验和学习平台

## 故障排除

### 常见问题

1. **计数不准确**
   - 检查A/B相连线是否正确
   - 确认编码器电源电压
   - 验证信号波形质量

2. **速度计算异常**
   - 调整速度计算周期
   - 检查PPR参数设置
   - 使用滤波算法

3. **方向判断错误**
   - 检查A/B相接线顺序
   - 使用reverse参数纠正
   - 验证编码器规格

## 学习要点

1. **编码器原理**: 学习增量式编码器工作原理
2. **正交解码**: 理解A/B相正交信号处理
3. **速度计算**: 掌握基于脉冲的速度计算方法
4. **滤波算法**: 学习各种数字滤波算法
5. **数据处理**: 了解传感器数据处理和分析

## 版权信息

版权 (c) 2025 MCU Electronics Technology

MIT License - 详见LICENSE文件
