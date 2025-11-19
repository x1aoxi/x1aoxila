#include "uart_driver.h"

// UART发送队列相关定义
#define UART_TX_BUFFER_SIZE 1024  // 发送缓冲区大小为1024字节
static uint8_t uart_tx_buffer[UART_TX_BUFFER_SIZE];  // 发送数据缓冲区
static struct rt_ringbuffer uart_tx_ringbuffer;      // 发送环形缓冲区结构体
static volatile uint8_t uart_tx_busy = 0;            // UART发送忙标志，0-空闲，1-忙
static UART_HandleTypeDef *current_huart;            // 当前UART句柄

/**
 * @brief 初始化UART发送缓冲区
 * @param 无
 * @retval 无
 * @note 在系统初始化时调用此函数初始化发送缓冲区
 */
void Uart_Tx_Init(void)
{
    // 使用RT-Thread环形缓冲区库初始化发送缓冲区
    rt_ringbuffer_init(&uart_tx_ringbuffer, uart_tx_buffer, UART_TX_BUFFER_SIZE);
}

/**
 * @brief UART发送完成回调函数
 * @param huart UART句柄
 * @retval 无
 * @note 此函数在UART中断发送完成时被调用，用于处理队列中的下一包数据
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    uint8_t *data_ptr;      // 数据指针
    rt_size_t data_len;     // 数据长度
    
    // 设置发送空闲状态
    uart_tx_busy = 0;
    
    // 检查环形缓冲区中是否还有数据需要发送
    data_len = rt_ringbuffer_peek(&uart_tx_ringbuffer, &data_ptr);
    
    if (data_len > 0) {
        // 设置发送忙标志
        uart_tx_busy = 1;
        // 使用中断方式发送数据
        HAL_UART_Transmit_IT(current_huart, data_ptr, data_len);
        
        // 更新读指针，从缓冲区中移除已发送的数据
        rt_ringbuffer_get(&uart_tx_ringbuffer, NULL, data_len);
    }
}

/**
 * @brief 非阻塞式UART格式化打印函数
 * @param huart UART句柄
 * @param format 格式化字符串
 * @param ... 可变参数
 * @retval 实际放入发送队列的字节数
 * @note 此函数是非阻塞的，数据会被放入发送队列后立即返回
 */
int Uart_Printf(UART_HandleTypeDef *huart, const char *format, ...)
{
    char temp_buffer[512];  // 临时缓冲区，用于存储格式化后的数据
    va_list arg;            // 可变参数列表
    int len;                // 格式化后的数据长度
    rt_size_t put_len;      // 实际放入环形缓冲区的数据长度

    // 处理可变参数并格式化字符串
    va_start(arg, format);
    len = vsnprintf(temp_buffer, sizeof(temp_buffer), format, arg);
    va_end(arg);

    // 将数据放入发送队列（环形缓冲区）
    put_len = rt_ringbuffer_put(&uart_tx_ringbuffer, (uint8_t*)temp_buffer, len);
    
    // 如果当前没有发送任务，则启动发送
    if (!uart_tx_busy) {
        uint8_t *data_ptr;   // 数据指针
        rt_size_t data_len;  // 数据长度
        
        // 获取要发送的数据
        data_len = rt_ringbuffer_peek(&uart_tx_ringbuffer, &data_ptr);
      
        if (data_len > 0) {
            // 设置发送忙标志
            uart_tx_busy = 1;
            current_huart = huart;
            // 使用中断方式发送数据
            HAL_UART_Transmit_IT(huart, data_ptr, data_len);
            
            // 更新读指针，从缓冲区中移除已发送的数据
            rt_ringbuffer_get(&uart_tx_ringbuffer, NULL, data_len);
        }
    }

    // 返回实际放入发送队列的字节数
    return put_len;
}

/* 串口 1 */
uint8_t uart1_rx_dma_buffer[BUFFER_SIZE]; // DMA 读取缓冲区

uint8_t uart1_ring_buffer_input[BUFFER_SIZE]; // 环形缓冲区对应的线性数组
struct rt_ringbuffer uart1_ring_buffer; // 环形缓冲区

uint8_t uart1_data_buffer[BUFFER_SIZE]; // 数据处理缓冲区

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    /* 串口 1 */
    if (huart->Instance == USART1)
    {
        HAL_UART_DMAStop(huart);

        rt_ringbuffer_put(&uart1_ring_buffer, uart1_rx_dma_buffer, Size);

        memset(uart1_rx_dma_buffer, 0, sizeof(uart1_rx_dma_buffer));

        HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uart1_rx_dma_buffer, sizeof(uart1_rx_dma_buffer));
        
         __HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
    }
}
