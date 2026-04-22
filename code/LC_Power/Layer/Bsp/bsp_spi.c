/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bsp_spi.c
  * @brief          : BSP SPI implementation - 借鉴开源项目的优雅设计
  *                   参考：RT-Thread SPI框架 + Zephyr SPI API
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
#include "bsp_spi.h"
#include "main.h"

/* Private macros ------------------------------------------------------------*/

// 将GPIO_PIN_x位掩码转换为引脚号的宏（编译时计算）
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

// SPI操作集（由底层驱动填充）
static const struct spi_ops *g_spi_ops = NULL;

/* SPI设备定义（STM32平台） */
spi_device_t SPI_W25Q256 = {
    .sck = { .port = (void*)W25Q256_SCK_GPIO_Port, .pin = GPIO_PIN_TO_NUM(W25Q256_SCK_Pin) },
    .mosi = { .port = (void*)W25Q256_MOSI_GPIO_Port, .pin = GPIO_PIN_TO_NUM(W25Q256_MOSI_Pin) },
    .miso = { .port = (void*)W25Q256_MISO_GPIO_Port, .pin = GPIO_PIN_TO_NUM(W25Q256_MISO_Pin) },
    .cs = { .port = (void*)W25Q256_CS_GPIO_Port, .pin = GPIO_PIN_TO_NUM(W25Q256_CS_Pin) },
    .delay_cycles = 10  // 约0.14us延时（72MHz）
};

/* Private function prototypes -----------------------------------------------*/

// ========== STM32平台的SPI操作集实现 ==========
// 换芯片时只需修改这3个函数！

static void stm32_spi_pin_set(const spi_pin_t *pin, uint8_t value)
{
    GPIO_TypeDef *port = (GPIO_TypeDef *)pin->port;
    uint16_t gpio_pin = (1 << pin->pin);
    HAL_GPIO_WritePin(port, gpio_pin, value ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static uint8_t stm32_spi_pin_get(const spi_pin_t *pin)
{
    GPIO_TypeDef *port = (GPIO_TypeDef *)pin->port;
    uint16_t gpio_pin = (1 << pin->pin);
    return (HAL_GPIO_ReadPin(port, gpio_pin) == GPIO_PIN_SET) ? 1 : 0;
}

static void stm32_spi_delay(uint32_t cycles)
{
    for (volatile uint32_t i = 0; i < cycles; i++) {
        __NOP();  // 空操作，防止编译器优化
    }
}

// STM32平台的操作集
static const struct spi_ops stm32_spi_ops = {
    .pin_set = stm32_spi_pin_set,
    .pin_get = stm32_spi_pin_get,
    .delay = stm32_spi_delay,
};

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  初始化SPI子系统
 * @note   注册平台相关的操作集
 */
void BSP_SPI_Init(void)
{
    // 注册STM32的SPI操作集
    g_spi_ops = &stm32_spi_ops;

    // 初始化引脚状态
    g_spi_ops->pin_set(&SPI_W25Q256.sck, 0);   // SCK默认低电平（SPI Mode 0）
    g_spi_ops->pin_set(&SPI_W25Q256.cs, 1);    // CS默认高电平（未选中）
}

// ========== 平台无关的SPI操作函数 ==========

/**
 * @brief  SPI传输一个字节（同时发送和接收）
 * @note   SPI Mode 0: CPOL=0, CPHA=0（时钟空闲为低，第一个边沿采样）
 */
uint8_t SPI_TransferByte(const spi_device_t *dev, uint8_t data)
{
    uint8_t recv = 0;

    for (uint8_t i = 0; i < 8; i++) {
        // 发送数据（MSB先发送）
        if (data & 0x80) {
            g_spi_ops->pin_set(&dev->mosi, 1);
        } else {
            g_spi_ops->pin_set(&dev->mosi, 0);
        }
        data <<= 1;

        g_spi_ops->delay(dev->delay_cycles);

        // 时钟上升沿
        g_spi_ops->pin_set(&dev->sck, 1);

        g_spi_ops->delay(dev->delay_cycles);

        // 读取数据（在时钟高电平期间采样）
        recv <<= 1;
        if (g_spi_ops->pin_get(&dev->miso)) {
            recv |= 0x01;
        }

        // 时钟下降沿
        g_spi_ops->pin_set(&dev->sck, 0);
    }

    return recv;
}

/**
 * @brief  SPI发送一个字节
 */
void SPI_WriteByte(const spi_device_t *dev, uint8_t data)
{
    (void)SPI_TransferByte(dev, data);
}

/**
 * @brief  SPI接收一个字节
 */
uint8_t SPI_ReadByte(const spi_device_t *dev)
{
    return SPI_TransferByte(dev, 0xFF);  // 发送0xFF作为dummy数据
}

/**
 * @brief  SPI片选控制
 */
void SPI_CS_Control(const spi_device_t *dev, uint8_t select)
{
    if (select) {
        g_spi_ops->pin_set(&dev->cs, 0);  // 选中（拉低）
    } else {
        g_spi_ops->pin_set(&dev->cs, 1);  // 释放（拉高）
    }
}
