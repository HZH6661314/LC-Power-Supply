/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bsp_lcd.h
  * @brief          : BSP LCD硬件抽象层
  *                   提供平台无关的LCD底层接口
  ******************************************************************************
  * @attention
  *
  * 设计理念：
  * 1. 完全平台无关，不包含任何STM32特定头文件
  * 2. 使用操作集（ops）模式实现硬件抽象
  * 3. 支持DMA传输，提高性能
  * 4. 提供非阻塞延时接口
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __BSP_LCD_H
#define __BSP_LCD_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief LCD引脚描述符（平台无关）
 */
typedef struct {
    void *port;      // GPIO端口（平台无关）
    uint16_t pin;    // 引脚掩码
} lcd_pin_t;

/**
 * @brief LCD硬件接口（平台无关）
 */
typedef struct {
    lcd_pin_t sck;   // 时钟引脚
    lcd_pin_t sda;   // 数据引脚
    lcd_pin_t cs;    // 片选引脚
    lcd_pin_t dc;    // 数据/命令引脚
    lcd_pin_t res;   // 复位引脚
} lcd_interface_t;

/**
 * @brief LCD操作集（由BSP层实现）
 */
struct lcd_ops {
    void (*pin_write)(const lcd_pin_t *pin, uint8_t value);
    void (*spi_write_byte)(uint8_t data);
    void (*spi_write_buffer)(const uint8_t *data, uint32_t len);
    uint32_t (*get_tick_ms)(void);
};

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief  初始化LCD BSP层
 * @retval None
 */
void BSP_LCD_Init(void);

/**
 * @brief  获取LCD接口描述符
 * @retval LCD接口指针
 */
const lcd_interface_t* BSP_LCD_GetInterface(void);

/**
 * @brief  获取LCD操作集
 * @retval 操作集指针
 */
const struct lcd_ops* BSP_LCD_GetOps(void);

/**
 * @brief  设置LCD背光亮度
 * @param  percent: 亮度百分比 (0-100)
 * @retval None
 */
void BSP_LCD_SetBacklight(uint8_t percent);

/**
 * @brief  获取当前时间戳（毫秒）
 * @retval 时间戳
 */
uint32_t BSP_LCD_GetTickMS(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_LCD_H */
