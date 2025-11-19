/*
 * @����������������״� STM32F407VET6 ��ذ� �� HAL ����б�д
 * @���ܣ��� 0.91 �� OLED ����ʾ�����ת��(rmp �� cm/s)��ͬʱ�ô��ڴ�ӡ����
*/

#include "Scheduler_Task.h"

void System_Init(void)
{
	Uart_Tx_Init();
  Led_Init();
  Key_Init();
  Uart_Init();
  Oled_Init();
  Gray_Init();
  Motor_Init();
  Encoder_Init();
  PID_Init();  // Initialize PID controllers
  Uart_Printf(DEBUG_UART, "==== System Init ====\r\n");
  HAL_TIM_Base_Start_IT(&htim2);
}

unsigned char encoder_timer10ms;
unsigned char pid_timer10ms;
unsigned char motor_timer10ms;

// TIM2 中断服务函数(1ms 中断)
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if(htim->Instance != htim2.Instance) return;

  if(++encoder_timer10ms >= 10)
  {
    encoder_timer10ms = 0;
    Encoder_Task();  // 更新编码器数据
  }

  if(++motor_timer10ms >= 10)
  {
    motor_timer10ms = 0;
    Motor_Task();    // 更新电机状态反馈
  }

  if(++pid_timer10ms >= 10)
  {
    pid_timer10ms = 0;
    PID_Task();      // 执行PID控制
  }

}
