/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bsp_spi.c
  * @brief          : BSP SPI implementation file.
  *                   This file contains SPI transfer service stubs.
  ******************************************************************************
  * @attention
  *
  * Author: LCYX
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "bsp_spi.h"
#include "main.h"  // 包含GPIO引脚定义

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

// 全局软件SPI接口（单例模式）
static SoftSPI_t s_soft_spi;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

// ========== STM32平台相关的GPIO操作函数 ==========
// 注意：这些函数是平台相关的，换芯片时需要修改

static void W25Q256_SCK_SetHigh(void) {
    HAL_GPIO_WritePin(W25Q256_SCK_GPIO_Port, W25Q256_SCK_Pin, GPIO_PIN_SET);
}

static void W25Q256_SCK_SetLow(void) {
    HAL_GPIO_WritePin(W25Q256_SCK_GPIO_Port, W25Q256_SCK_Pin, GPIO_PIN_RESET);
}

static void W25Q256_MOSI_SetHigh(void) {
    HAL_GPIO_WritePin(W25Q256_MOSI_GPIO_Port, W25Q256_MOSI_Pin, GPIO_PIN_SET);
}

static void W25Q256_MOSI_SetLow(void) {
    HAL_GPIO_WritePin(W25Q256_MOSI_GPIO_Port, W25Q256_MOSI_Pin, GPIO_PIN_RESET);
}

static uint8_t W25Q256_MISO_Read(void) {
    return (HAL_GPIO_ReadPin(W25Q256_MISO_GPIO_Port, W25Q256_MISO_Pin) == GPIO_PIN_SET) ? 1 : 0;
}

static void W25Q256_CS_SetHigh(void) {
    HAL_GPIO_WritePin(W25Q256_CS_GPIO_Port, W25Q256_CS_Pin, GPIO_PIN_SET);
}

static void W25Q256_CS_SetLow(void) {
    HAL_GPIO_WritePin(W25Q256_CS_GPIO_Port, W25Q256_CS_Pin, GPIO_PIN_RESET);
}

// 简单的延时函数（空循环，不用HAL_Delay）
static inline void SoftSPI_Delay(uint32_t cycles) {
    for (volatile uint32_t i = 0; i < cycles; i++) {
        __NOP();  // 空操作，防止编译器优化
    }
}

/* USER CODE END PFP */

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  初始化软件SPI（绑定虚函数指针）
 * @note   这是BSP层的核心：将平台相关的GPIO函数绑定到虚函数指针
 *         换芯片时只需修改这个函数的实现
 */
void BSP_SPI_Init(void)
{
  /* USER CODE BEGIN BSP_SPI_Init */

  // 绑定SCK引脚的虚函数
  s_soft_spi.SCK.SetHigh = W25Q256_SCK_SetHigh;
  s_soft_spi.SCK.SetLow  = W25Q256_SCK_SetLow;
  s_soft_spi.SCK.Read    = NULL;  // SCK不需要读取

  // 绑定MOSI引脚的虚函数
  s_soft_spi.MOSI.SetHigh = W25Q256_MOSI_SetHigh;
  s_soft_spi.MOSI.SetLow  = W25Q256_MOSI_SetLow;
  s_soft_spi.MOSI.Read    = NULL;  // MOSI不需要读取

  // 绑定MISO引脚的虚函数
  s_soft_spi.MISO.SetHigh = NULL;  // MISO不需要写入
  s_soft_spi.MISO.SetLow  = NULL;
  s_soft_spi.MISO.Read    = W25Q256_MISO_Read;

  // 绑定CS引脚的虚函数
  s_soft_spi.CS.SetHigh = W25Q256_CS_SetHigh;
  s_soft_spi.CS.SetLow  = W25Q256_CS_SetLow;
  s_soft_spi.CS.Read    = NULL;  // CS不需要读取

  // 设置延时周期（根据MCU频率调整，这里假设72MHz）
  s_soft_spi.delay_cycles = 10;  // 约0.14us延时

  // 初始化引脚状态
  s_soft_spi.SCK.SetLow();   // SCK默认低电平（SPI Mode 0）
  s_soft_spi.CS.SetHigh();   // CS默认高电平（未选中）

  /* USER CODE END BSP_SPI_Init */
}

/**
 * @brief  获取软件SPI抽象接口
 * @note   Driver层通过这个函数获取SPI接口
 * @retval 软件SPI抽象接口指针
 */
SoftSPI_t* BSP_SPI_GetInterface(void)
{
    return &s_soft_spi;
}

/* USER CODE BEGIN 1 */

// ========== 通用的软件SPI实现（完全可移植，不依赖任何硬件）==========

/**
 * @brief  软件SPI传输一个字节（同时发送和接收）
 * @note   这个函数只调用虚函数指针，完全可移植
 *         SPI Mode 0: CPOL=0, CPHA=0（时钟空闲为低，第一个边沿采样）
 * @param  spi: 软件SPI接口指针
 * @param  data: 要发送的字节
 * @retval 接收到的字节
 */
uint8_t SoftSPI_TransferByte(SoftSPI_t *spi, uint8_t data)
{
    uint8_t recv = 0;

    for (uint8_t i = 0; i < 8; i++) {
        // 发送数据（MSB先发送）
        if (data & 0x80) {
            spi->MOSI.SetHigh();
        } else {
            spi->MOSI.SetLow();
        }
        data <<= 1;

        SoftSPI_Delay(spi->delay_cycles);

        // 时钟上升沿
        spi->SCK.SetHigh();

        SoftSPI_Delay(spi->delay_cycles);

        // 读取数据（在时钟高电平期间采样）
        recv <<= 1;
        if (spi->MISO.Read()) {
            recv |= 0x01;
        }

        // 时钟下降沿
        spi->SCK.SetLow();
    }

    return recv;
}

/**
 * @brief  软件SPI发送一个字节
 * @param  spi: 软件SPI接口指针
 * @param  data: 要发送的字节
 * @retval None
 */
void SoftSPI_WriteByte(SoftSPI_t *spi, uint8_t data)
{
    (void)SoftSPI_TransferByte(spi, data);
}

/**
 * @brief  软件SPI接收一个字节
 * @param  spi: 软件SPI接口指针
 * @retval 接收到的字节
 */
uint8_t SoftSPI_ReadByte(SoftSPI_t *spi)
{
    return SoftSPI_TransferByte(spi, 0xFF);  // 发送0xFF作为dummy数据
}

/* USER CODE END 1 */
