#include "uart_app.h"

/* 串口 1 */
extern uint8_t uart1_rx_dma_buffer[BUFFER_SIZE]; // DMA 读取缓冲区

extern uint8_t uart1_ring_buffer_input[BUFFER_SIZE]; // 环形缓冲区对应的线性数组
extern struct rt_ringbuffer uart1_ring_buffer; // 环形缓冲区

extern uint8_t uart1_data_buffer[BUFFER_SIZE]; // 数据处理缓冲区

void Uart_Init(void)
{
  /* 串口 1 */
  rt_ringbuffer_init(&uart1_ring_buffer, uart1_ring_buffer_input, BUFFER_SIZE);
  
  HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uart1_rx_dma_buffer, BUFFER_SIZE); // 启动读取中断
  __HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT); // 关闭 DMA 的"半满中断"功能
  
}

/* 串口 1 */
void Uart1_Task(void)
{
  uint16_t uart_data_len = rt_ringbuffer_data_len(&uart1_ring_buffer);
  if(uart_data_len > 0)
  {
    rt_ringbuffer_get(&uart1_ring_buffer, uart1_data_buffer, uart_data_len);
    uart1_data_buffer[uart_data_len] = '\0';
    /* 数据解析 */
    Uart_Printf(DEBUG_UART, "UART1 Ringbuffer:%s\r\n", uart1_data_buffer);
    
    memset(uart1_data_buffer, 0, uart_data_len);
  }
}

