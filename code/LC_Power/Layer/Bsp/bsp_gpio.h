/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bsp_gpio.h
  * @brief          : BSP GPIO header - 借鉴开源项目的优雅设计
  *                   参考：RT-Thread PIN框架 + Zephyr GPIO API
  ******************************************************************************
  * @attention
  *
  * Author: LCYX
  *
  * 设计理念（借鉴开源项目）：
  * 1. 使用gpio_pin_t结构体封装引脚（Zephyr风格）
  * 2. 使用操作集（ops）实现平台抽象（RT-Thread风格）
  * 3. 使用GET_PIN宏简化引脚定义（RT-Thread风格）
  * 4. 完全平台无关，换芯片只需修改底层驱动
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __BSP_GPIO_H
#define __BSP_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief GPIO电平定义
 */
typedef enum
{
  PIN_LOW = 0u,
  PIN_HIGH = 1u
} pin_level_t;

/**
 * @brief GPIO引脚描述符（平台无关）
 * @note  借鉴Zephyr的gpio_dt_spec设计
 */
typedef struct {
    void *port;      // GPIO端口（平台相关，可以是寄存器基地址或设备指针）
    uint8_t pin;     // 引脚号（0-15）
} gpio_pin_t;

/**
 * @brief GPIO操作集（平台相关，由底层驱动实现）
 * @note  借鉴RT-Thread的rt_pin_ops设计
 */
struct gpio_ops {
    void (*pin_write)(const gpio_pin_t *pin, uint8_t value);
    uint8_t (*pin_read)(const gpio_pin_t *pin);
    void (*pin_toggle)(const gpio_pin_t *pin);
};

/* Exported macros -----------------------------------------------------------*/

/**
 * @brief 定义GPIO引脚（便捷宏）
 * @note  借鉴RT-Thread的GET_PIN宏
 * @param port: GPIO端口基地址
 * @param pin: 引脚号
 */
#define GPIO_PIN_DEFINE(port, pin)  { .port = (void*)(port), .pin = (pin) }

/* Exported constants --------------------------------------------------------*/

// ========== GPIO引脚定义（使用便捷宏） ==========
// 注意：这些定义是平台相关的，但只需定义一次

#ifdef STM32_PLATFORM
#include "main.h"  // 包含STM32的GPIO定义

// LED引脚定义
extern const gpio_pin_t GPIO_LED0;
extern const gpio_pin_t GPIO_LED1;
extern const gpio_pin_t GPIO_LED2;
extern const gpio_pin_t GPIO_LED3;

// 按键引脚定义
extern const gpio_pin_t GPIO_BTN_KEY1_SET;
extern const gpio_pin_t GPIO_BTN_KEY1_EXIT;
extern const gpio_pin_t GPIO_BTN_KEY1_UP;
extern const gpio_pin_t GPIO_BTN_KEY1_DOWN;
extern const gpio_pin_t GPIO_BTN_KEY2_SET;
extern const gpio_pin_t GPIO_BTN_KEY2_UP;
extern const gpio_pin_t GPIO_BTN_KEY2_DOWN;

#elif defined(ARDUINO_PLATFORM)
// Arduino平台的引脚定义
extern const gpio_pin_t GPIO_LED0;
// ...

#elif defined(TI_C2000_PLATFORM)
// TI平台的引脚定义
extern const gpio_pin_t GPIO_LED0;
// ...

#endif

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief  初始化GPIO子系统
 * @note   注册GPIO操作集，初始化底层驱动
 * @retval None
 */
void BSP_GPIO_Init(void);

/**
 * @brief  写入GPIO引脚电平（平台无关）
 * @param  pin: GPIO引脚描述符
 * @param  value: 引脚电平（PIN_LOW或PIN_HIGH）
 * @retval None
 */
void gpio_pin_write(const gpio_pin_t *pin, uint8_t value);

/**
 * @brief  读取GPIO引脚电平（平台无关）
 * @param  pin: GPIO引脚描述符
 * @retval 引脚电平（PIN_LOW或PIN_HIGH）
 */
uint8_t gpio_pin_read(const gpio_pin_t *pin);

/**
 * @brief  翻转GPIO引脚电平（平台无关）
 * @param  pin: GPIO引脚描述符
 * @retval None
 */
void gpio_pin_toggle(const gpio_pin_t *pin);

// ========== 应用层接口（兼容旧代码） ==========
uint8_t Drv_Btn_Read_SET1(void);
uint8_t Drv_Btn_Read_SET2(void);
uint8_t Drv_Btn_Read_UP1(void);
uint8_t Drv_Btn_Read_DOWN1(void);
uint8_t Drv_Btn_Read_UP2(void);
uint8_t Drv_Btn_Read_DOWN2(void);
uint8_t Drv_Btn_Read_EXIT(void);

void Drv_LED0_ON(void);
void Drv_LED0_OFF(void);
void Drv_LED0_Toggle(void);

void Drv_LED1_ON(void);
void Drv_LED1_OFF(void);
void Drv_LED1_Toggle(void);

void Drv_LED2_ON(void);
void Drv_LED2_OFF(void);
void Drv_LED2_Toggle(void);

void Drv_LED3_ON(void);
void Drv_LED3_OFF(void);
void Drv_LED3_Toggle(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_GPIO_H */
