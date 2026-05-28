/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bsp_gpio.c
  * @brief          : BSP GPIO implementation - 借鉴开源项目的优雅设计
  *                   参考：RT-Thread PIN框架 + Zephyr GPIO API
  ******************************************************************************
  * @attention
  *
  * Author: LCYX
  *
  * 设计亮点：
  * 1. 使用操作集（ops）实现平台抽象
  * 2. 应用层代码完全平台无关
  * 3. 换芯片只需修改操作集的实现
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#define STM32_PLATFORM  // 定义当前平台
#include "bsp_gpio.h"

/* Private macros ------------------------------------------------------------*/

// 将GPIO_PIN_x位掩码转换为引脚号的宏（编译时计算）
// 使用条件表达式展开，支持常量初始化
#define GPIO_PIN_TO_NUM(pin) ( \
    (pin) == 0x0001 ? 0 : \
    (pin) == 0x0002 ? 1 : \
    (pin) == 0x0004 ? 2 : \
    (pin) == 0x0008 ? 3 : \
    (pin) == 0x0010 ? 4 : \
    (pin) == 0x0020 ? 5 : \
    (pin) == 0x0040 ? 6 : \
    (pin) == 0x0080 ? 7 : \
    (pin) == 0x0100 ? 8 : \
    (pin) == 0x0200 ? 9 : \
    (pin) == 0x0400 ? 10 : \
    (pin) == 0x0800 ? 11 : \
    (pin) == 0x1000 ? 12 : \
    (pin) == 0x2000 ? 13 : \
    (pin) == 0x4000 ? 14 : \
    (pin) == 0x8000 ? 15 : 0 \
)

/* Private variables ---------------------------------------------------------*/

// GPIO操作集（由底层驱动填充）
static const struct gpio_ops *g_gpio_ops = NULL;

/* GPIO引脚定义（STM32平台） */
gpio_pin_t GPIO_LED0 = { .port = (void*)LED0_GPIO_Port, .pin = GPIO_PIN_TO_NUM(LED0_Pin) };
gpio_pin_t GPIO_LED1 = { .port = (void*)LED1_GPIO_Port, .pin = GPIO_PIN_TO_NUM(LED1_Pin) };
gpio_pin_t GPIO_LED2 = { .port = (void*)LED2_GPIO_Port, .pin = GPIO_PIN_TO_NUM(LED2_Pin) };
gpio_pin_t GPIO_LED3 = { .port = (void*)LED3_GPIO_Port, .pin = GPIO_PIN_TO_NUM(LED3_Pin) };

gpio_pin_t GPIO_BTN_KEY1_SET = { .port = (void*)KEY1_SET_GPIO_Port, .pin = GPIO_PIN_TO_NUM(KEY1_SET_Pin) };
gpio_pin_t GPIO_BTN_KEY1_EXIT = { .port = (void*)KEY1_EXIT_GPIO_Port, .pin = GPIO_PIN_TO_NUM(KEY1_EXIT_Pin) };
gpio_pin_t GPIO_BTN_KEY1_UP = { .port = (void*)KEY1_UP_GPIO_Port, .pin = GPIO_PIN_TO_NUM(KEY1_UP_Pin) };
gpio_pin_t GPIO_BTN_KEY1_DOWN = { .port = (void*)KEY1_DOWN_GPIO_Port, .pin = GPIO_PIN_TO_NUM(KEY1_DOWN_Pin) };
gpio_pin_t GPIO_BTN_KEY2_SET = { .port = (void*)KEY2_SET_GPIO_Port, .pin = GPIO_PIN_TO_NUM(KEY2_SET_Pin) };
gpio_pin_t GPIO_BTN_KEY2_UP = { .port = (void*)KEY2_UP_GPIO_Port, .pin = GPIO_PIN_TO_NUM(KEY2_UP_Pin) };
gpio_pin_t GPIO_BTN_KEY2_DOWN = { .port = (void*)KEY2_DOWN_GPIO_Port, .pin = GPIO_PIN_TO_NUM(KEY2_DOWN_Pin) };

gpio_pin_t GPIO_OUT_CTRL_Pin = { .port = (void*)DC_OUT_CTRL_GPIO_Port, .pin = GPIO_PIN_TO_NUM(DC_OUT_CTRL_Pin) };  // 复用SET引脚
gpio_pin_t GPIO_FAN_PWM_Pin = { .port = (void*)FAN_PWM_GPIO_Port, .pin = GPIO_PIN_TO_NUM(FAN_PWM_Pin) };  // 风扇PWM引脚    
/* Private function prototypes -----------------------------------------------*/

// ========== STM32平台的GPIO操作集实现 ==========
// 换芯片时只需修改这3个函数！

static void stm32_gpio_write(const gpio_pin_t *pin, uint8_t value)
{
    GPIO_TypeDef *port = (GPIO_TypeDef *)pin->port;
    uint16_t gpio_pin = (1 << pin->pin);
    HAL_GPIO_WritePin(port, gpio_pin, value ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static uint8_t stm32_gpio_read(const gpio_pin_t *pin)
{
    GPIO_TypeDef *port = (GPIO_TypeDef *)pin->port;
    uint16_t gpio_pin = (1 << pin->pin);
    return (HAL_GPIO_ReadPin(port, gpio_pin) == GPIO_PIN_SET) ? PIN_HIGH : PIN_LOW;
}

static void stm32_gpio_toggle(const gpio_pin_t *pin)
{
    GPIO_TypeDef *port = (GPIO_TypeDef *)pin->port;
    uint16_t gpio_pin = (1 << pin->pin);
    HAL_GPIO_TogglePin(port, gpio_pin);
}

// STM32平台的操作集
static const struct gpio_ops stm32_gpio_ops = {
    .pin_write = stm32_gpio_write,
    .pin_read = stm32_gpio_read,
    .pin_toggle = stm32_gpio_toggle,
};

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  初始化GPIO子系统
 * @note   注册平台相关的操作集
 */
void BSP_GPIO_Init(void)
{
    // 注册STM32的GPIO操作集
    g_gpio_ops = &stm32_gpio_ops;

    // 初始化LED状态（全部熄灭）
    Drv_LED0_OFF();
    Drv_LED1_OFF();
    Drv_LED2_OFF();
    Drv_LED3_OFF();
}

// ========== 平台无关的GPIO操作函数 ==========

void gpio_pin_write(const gpio_pin_t *pin, uint8_t value)
{
    if (g_gpio_ops && g_gpio_ops->pin_write) {
        g_gpio_ops->pin_write(pin, value);
    }
}

uint8_t gpio_pin_read(const gpio_pin_t *pin)
{
    if (g_gpio_ops && g_gpio_ops->pin_read) {
        return g_gpio_ops->pin_read(pin);
    }
    return PIN_LOW;
}

void gpio_pin_toggle(const gpio_pin_t *pin)
{
    if (g_gpio_ops && g_gpio_ops->pin_toggle) {
        g_gpio_ops->pin_toggle(pin);
    }
}

// ========== 应用层接口（完全平台无关） ==========

uint8_t Drv_Btn_Read_SET1(void)
{
    // 按键按下时为低电平
    return (gpio_pin_read(&GPIO_BTN_KEY1_SET) == PIN_LOW) ? PIN_LOW : PIN_HIGH;
}

uint8_t Drv_Btn_Read_SET2(void)
{
    return (gpio_pin_read(&GPIO_BTN_KEY2_SET) == PIN_LOW) ? PIN_LOW : PIN_HIGH;
}

uint8_t Drv_Btn_Read_UP1(void)
{
    return (gpio_pin_read(&GPIO_BTN_KEY1_UP) == PIN_LOW) ? PIN_LOW : PIN_HIGH;
}

uint8_t Drv_Btn_Read_DOWN1(void)
{
    return (gpio_pin_read(&GPIO_BTN_KEY1_DOWN) == PIN_LOW) ? PIN_LOW : PIN_HIGH;
}

uint8_t Drv_Btn_Read_UP2(void)
{
    return (gpio_pin_read(&GPIO_BTN_KEY2_UP) == PIN_LOW) ? PIN_LOW : PIN_HIGH;
}

uint8_t Drv_Btn_Read_DOWN2(void)
{
    return (gpio_pin_read(&GPIO_BTN_KEY2_DOWN) == PIN_LOW) ? PIN_LOW : PIN_HIGH;
}

uint8_t Drv_Btn_Read_EXIT(void)
{
    return (gpio_pin_read(&GPIO_BTN_KEY1_EXIT) == PIN_LOW) ? PIN_LOW : PIN_HIGH;
}

void Drv_LED0_ON(void)
{
    gpio_pin_write(&GPIO_LED0, PIN_LOW);  // 低电平点亮
    gpio_pin_write(&GPIO_OUT_CTRL_Pin, PIN_HIGH);  // 同时控制输出使能引脚，确保LED完全点亮
    gpio_pin_write(&GPIO_FAN_PWM_Pin, PIN_HIGH);  // 风扇引脚保持高电平，确保风扇开启
}

void Drv_LED0_OFF(void)
{
    gpio_pin_write(&GPIO_LED0, PIN_HIGH);  // 高电平熄灭
    gpio_pin_write(&GPIO_OUT_CTRL_Pin, PIN_LOW);  // 同时控制输出使能引脚，确保LED完全关闭
    gpio_pin_write(&GPIO_FAN_PWM_Pin, PIN_LOW);  // 风扇引脚保持低电平，确保风扇关闭
}

void Drv_LED0_Toggle(void)
{
    gpio_pin_toggle(&GPIO_LED0);
}

void Drv_LED1_ON(void)
{
    gpio_pin_write(&GPIO_LED1, PIN_LOW);
}

void Drv_LED1_OFF(void)
{
    gpio_pin_write(&GPIO_LED1, PIN_HIGH);
}

void Drv_LED1_Toggle(void)
{
    gpio_pin_toggle(&GPIO_LED1);
}

void Drv_LED2_ON(void)
{
    gpio_pin_write(&GPIO_LED2, PIN_LOW);
}

void Drv_LED2_OFF(void)
{
    gpio_pin_write(&GPIO_LED2, PIN_HIGH);
}

void Drv_LED2_Toggle(void)
{
    gpio_pin_toggle(&GPIO_LED2);
}

void Drv_LED3_ON(void)
{
    gpio_pin_write(&GPIO_LED3, PIN_LOW);
}

void Drv_LED3_OFF(void)
{
    gpio_pin_write(&GPIO_LED3, PIN_HIGH);
}

void Drv_LED3_Toggle(void)
{
    gpio_pin_toggle(&GPIO_LED3);
}
