/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bsp_spi.h
  * @brief          : BSP SPI header - 借鉴开源项目的优雅设计
  *                   参考：RT-Thread SPI框架 + Zephyr SPI API
  ******************************************************************************
  * @attention
  *
  * Author: LCYX
  *
  * 设计理念（借鉴开源项目）：
  * 1. 使用spi_device_t结构体封装SPI设备（Zephyr风格）
  * 2. 使用操作集（ops）实现平台抽象（RT-Thread风格）
  * 3. 完全平台无关，换芯片只需修改底层驱动
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __BSP_SPI_H
#define __BSP_SPI_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief SPI引脚描述符（平台无关）
 * @note  借鉴GPIO的设计，使用void*实现平台无关
 */
typedef struct {
    void *port;      // GPIO端口（平台无关）
    uint8_t pin;     // 引脚号（0-15）
} spi_pin_t;

/**
 * @brief SPI设备描述符（平台无关）
 * @note  借鉴Zephyr的spi_dt_spec设计
 */
typedef struct {
    spi_pin_t sck;    // 时钟引脚
    spi_pin_t mosi;   // 主出从入
    spi_pin_t miso;   // 主入从出
    spi_pin_t cs;     // 片选引脚
    uint32_t delay_cycles;  // 延时周期数
} spi_device_t;

/**
 * @brief SPI操作集（平台相关，由底层驱动实现）
 * @note  借鉴RT-Thread的spi_ops设计
 */
struct spi_ops {
    void (*pin_set)(const spi_pin_t *pin, uint8_t value);
    uint8_t (*pin_get)(const spi_pin_t *pin);
    void (*delay)(uint32_t cycles);
};

/* Exported constants --------------------------------------------------------*/

// SPI设备定义
extern spi_device_t SPI_W25Q256;

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief  初始化SPI子系统
 * @note   注册SPI操作集，初始化底层驱动
 * @retval None
 */
void BSP_SPI_Init(void);

/**
 * @brief  SPI传输一个字节（平台无关）
 * @param  dev: SPI设备描述符
 * @param  data: 要发送的字节
 * @retval 接收到的字节
 */
uint8_t SPI_TransferByte(const spi_device_t *dev, uint8_t data);

/**
 * @brief  SPI发送一个字节（平台无关）
 * @param  dev: SPI设备描述符
 * @param  data: 要发送的字节
 * @retval None
 */
void SPI_WriteByte(const spi_device_t *dev, uint8_t data);

/**
 * @brief  SPI接收一个字节（平台无关）
 * @param  dev: SPI设备描述符
 * @retval 接收到的字节
 */
uint8_t SPI_ReadByte(const spi_device_t *dev);

/**
 * @brief  SPI片选控制（平台无关）
 * @param  dev: SPI设备描述符
 * @param  select: 1=选中（拉低），0=释放（拉高）
 * @retval None
 */
void SPI_CS_Control(const spi_device_t *dev, uint8_t select);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_SPI_H */
