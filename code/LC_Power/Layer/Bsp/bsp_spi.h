/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bsp_spi.h
  * @brief          : Header for bsp_spi.c file.
  *                   This file contains the module definitions and interfaces.
  ******************************************************************************
  * @attention
  *
  * Author: LCYX
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_SPI_H
#define __BSP_SPI_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/**
 * @brief GPIO抽象接口 - 虚函数指针
 * @note  这是硬件抽象的核心，Driver层只调用这些函数指针
 *        换芯片时只需修改BSP层的函数实现，Driver层无需修改
 */
typedef struct {
    void (*SetHigh)(void);   // 设置引脚为高电平
    void (*SetLow)(void);    // 设置引脚为低电平
    uint8_t (*Read)(void);   // 读取引脚电平（返回0或1）
} GPIO_Pin_t;

/**
 * @brief 软件SPI抽象接口
 * @note  包含4个GPIO引脚的虚函数指针
 */
typedef struct {
    GPIO_Pin_t SCK;    // 时钟引脚
    GPIO_Pin_t MOSI;   // 主出从入
    GPIO_Pin_t MISO;   // 主入从出
    GPIO_Pin_t CS;     // 片选引脚
    uint32_t   delay_cycles;  // 延时周期数（空循环，不用延时函数）
} SoftSPI_t;

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief  初始化软件SPI（绑定虚函数指针）
 * @note   这个函数在BSP层实现，将STM32的GPIO函数绑定到虚函数指针
 * @retval None
 */
void BSP_SPI_Init(void);

/**
 * @brief  获取软件SPI抽象接口
 * @note   Driver层通过这个函数获取SPI接口，然后调用虚函数
 * @retval 软件SPI抽象接口指针
 */
SoftSPI_t* BSP_SPI_GetInterface(void);

/**
 * @brief  软件SPI发送一个字节（通用实现，完全可移植）
 * @note   这个函数只调用虚函数指针，不依赖任何硬件
 * @param  spi: 软件SPI接口指针
 * @param  data: 要发送的字节
 * @retval None
 */
void SoftSPI_WriteByte(SoftSPI_t *spi, uint8_t data);

/**
 * @brief  软件SPI接收一个字节（通用实现，完全可移植）
 * @param  spi: 软件SPI接口指针
 * @retval 接收到的字节
 */
uint8_t SoftSPI_ReadByte(SoftSPI_t *spi);

/**
 * @brief  软件SPI传输一个字节（同时发送和接收）
 * @param  spi: 软件SPI接口指针
 * @param  data: 要发送的字节
 * @retval 接收到的字节
 */
uint8_t SoftSPI_TransferByte(SoftSPI_t *spi, uint8_t data);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __BSP_SPI_H */

/* Author: LCYX */
