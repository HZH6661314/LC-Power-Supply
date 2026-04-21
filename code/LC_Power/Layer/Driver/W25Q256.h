/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : W25Q256.h
  * @brief          : W25Q256 Flash驱动头文件
  *                   这是Driver层，只调用BSP层的抽象接口
  ******************************************************************************
  * @attention
  *
  * Author: LCYX
  *
  * 架构说明：
  * - Driver层不依赖任何具体芯片（STM32/TI/Arduino等）
  * - 只调用BSP层提供的抽象接口（SoftSPI_t）
  * - 换芯片时，Driver层无需修改
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef W25Q256_H
#define W25Q256_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported types ------------------------------------------------------------*/

/* W25Q256命令定义（根据数据手册） */
#define W25Q256_CMD_WRITE_ENABLE        0x06  // 写使能
#define W25Q256_CMD_WRITE_DISABLE       0x04  // 写禁止
#define W25Q256_CMD_READ_STATUS_REG1    0x05  // 读状态寄存器1
#define W25Q256_CMD_READ_STATUS_REG2    0x35  // 读状态寄存器2
#define W25Q256_CMD_READ_STATUS_REG3    0x15  // 读状态寄存器3
#define W25Q256_CMD_WRITE_STATUS_REG1   0x01  // 写状态寄存器1
#define W25Q256_CMD_WRITE_STATUS_REG2   0x31  // 写状态寄存器2
#define W25Q256_CMD_WRITE_STATUS_REG3   0x11  // 写状态寄存器3

// 读取命令
#define W25Q256_CMD_READ_DATA           0x03  // 读数据（3字节地址）
#define W25Q256_CMD_READ_DATA_4B        0x13  // 读数据（4字节地址）
#define W25Q256_CMD_FAST_READ           0x0B  // 快速读取
#define W25Q256_CMD_FAST_READ_4B        0x0C  // 快速读取（4字节地址）

// 写入命令
#define W25Q256_CMD_PAGE_PROGRAM        0x02  // 页编程（3字节地址）
#define W25Q256_CMD_PAGE_PROGRAM_4B     0x12  // 页编程（4字节地址）

// 擦除命令
#define W25Q256_CMD_SECTOR_ERASE        0x20  // 扇区擦除4KB（3字节地址）
#define W25Q256_CMD_SECTOR_ERASE_4B     0x21  // 扇区擦除4KB（4字节地址）
#define W25Q256_CMD_BLOCK_ERASE_32K     0x52  // 块擦除32KB
#define W25Q256_CMD_BLOCK_ERASE_64K     0xD8  // 块擦除64KB
#define W25Q256_CMD_BLOCK_ERASE_64K_4B  0xDC  // 块擦除64KB（4字节地址）
#define W25Q256_CMD_CHIP_ERASE          0xC7  // 芯片擦除（整片擦除）

// ID和信息命令
#define W25Q256_CMD_READ_JEDEC_ID       0x9F  // 读取JEDEC ID
#define W25Q256_CMD_READ_UNIQUE_ID      0x4B  // 读取唯一ID

// 地址模式命令
#define W25Q256_CMD_ENTER_4B_MODE       0xB7  // 进入4字节地址模式
#define W25Q256_CMD_EXIT_4B_MODE        0xE9  // 退出4字节地址模式

// 电源管理命令
#define W25Q256_CMD_POWER_DOWN          0xB9  // 掉电模式
#define W25Q256_CMD_RELEASE_POWER_DOWN  0xAB  // 释放掉电模式

// 状态寄存器位定义
#define W25Q256_STATUS_BUSY             0x01  // 忙标志位
#define W25Q256_STATUS_WEL              0x02  // 写使能标志位

// Flash参数
#define W25Q256_PAGE_SIZE               256   // 页大小：256字节
#define W25Q256_SECTOR_SIZE             4096  // 扇区大小：4KB
#define W25Q256_BLOCK_SIZE_32K          32768 // 块大小：32KB
#define W25Q256_BLOCK_SIZE_64K          65536 // 块大小：64KB
#define W25Q256_CHIP_SIZE               33554432  // 芯片大小：32MB

// JEDEC ID
#define W25Q256_MANUFACTURER_ID         0xEF  // Winbond制造商ID
#define W25Q256_DEVICE_ID               0x4019 // W25Q256设备ID

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief  初始化W25Q256驱动
 * @note   这个函数会从BSP层获取SPI接口，并进入4字节地址模式
 * @retval None
 */
void W25Q256_Init(void);

/**
 * @brief  读取W25Q256的JEDEC ID
 * @param  manufacturer_id: 制造商ID（输出参数，应为0xEF）
 * @param  device_id: 设备ID（输出参数，应为0x4019）
 * @retval None
 */
void W25Q256_ReadID(uint8_t *manufacturer_id, uint16_t *device_id);

/**
 * @brief  读取唯一ID（64位）
 * @param  unique_id: 唯一ID缓冲区（8字节）
 * @retval None
 */
void W25Q256_ReadUniqueID(uint8_t *unique_id);

/**
 * @brief  读取数据（支持32MB全地址空间）
 * @param  addr: 读取地址（32位地址，0x00000000 ~ 0x01FFFFFF）
 * @param  buffer: 数据缓冲区
 * @param  len: 读取长度
 * @retval None
 */
void W25Q256_ReadData(uint32_t addr, uint8_t *buffer, uint32_t len);

/**
 * @brief  快速读取数据
 * @param  addr: 读取地址
 * @param  buffer: 数据缓冲区
 * @param  len: 读取长度
 * @retval None
 */
void W25Q256_FastRead(uint32_t addr, uint8_t *buffer, uint32_t len);

/**
 * @brief  写入数据（页编程，自动处理跨页）
 * @param  addr: 写入地址
 * @param  buffer: 数据缓冲区
 * @param  len: 写入长度
 * @retval None
 */
void W25Q256_WriteData(uint32_t addr, const uint8_t *buffer, uint32_t len);

/**
 * @brief  写入一页数据（256字节以内）
 * @param  addr: 写入地址
 * @param  buffer: 数据缓冲区
 * @param  len: 写入长度（最大256字节）
 * @retval None
 */
void W25Q256_WritePage(uint32_t addr, const uint8_t *buffer, uint32_t len);

/**
 * @brief  擦除扇区（4KB）
 * @param  addr: 扇区地址（任意地址，自动对齐到4KB边界）
 * @retval None
 */
void W25Q256_EraseSector(uint32_t addr);

/**
 * @brief  擦除块（64KB）
 * @param  addr: 块地址（任意地址，自动对齐到64KB边界）
 * @retval None
 */
void W25Q256_EraseBlock64K(uint32_t addr);

/**
 * @brief  擦除整个芯片
 * @note   这个操作需要很长时间（约20-100秒）
 * @retval None
 */
void W25Q256_EraseChip(void);

/**
 * @brief  等待Flash空闲
 * @retval None
 */
void W25Q256_WaitBusy(void);

/**
 * @brief  进入掉电模式（降低功耗）
 * @retval None
 */
void W25Q256_PowerDown(void);

/**
 * @brief  唤醒（退出掉电模式）
 * @retval None
 */
void W25Q256_WakeUp(void);

/**
 * @brief  检查Flash是否正常工作
 * @retval 1: 正常, 0: 异常
 */
uint8_t W25Q256_SelfTest(void);

#ifdef __cplusplus
}
#endif

#endif /* W25Q256_H */
