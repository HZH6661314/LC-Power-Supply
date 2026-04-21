/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : W25Q256.c
  * @brief          : W25Q256 Flash驱动实现（完整版）
  *                   这是Driver层，只调用BSP层的抽象接口
  ******************************************************************************
  * @attention
  *
  * Author: LCYX
  * 根据W25Q256数据手册完整实现
  *
  * 架构亮点：
  * 1. Driver层不包含任何STM32相关的头文件（如stm32f3xx_hal.h）
  * 2. 只调用BSP层提供的抽象接口（SoftSPI_t）
  * 3. 完全可移植，换芯片时无需修改这个文件
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "W25Q256.h"
#include "bsp_spi.h"
#include <stddef.h>  // 包含NULL定义

/* Private variables ---------------------------------------------------------*/
static SoftSPI_t *s_spi = NULL;

/* Private function prototypes -----------------------------------------------*/
static uint8_t W25Q256_ReadStatusReg1(void);
static void W25Q256_WriteEnable(void);

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  初始化W25Q256驱动
 * @note   从BSP层获取SPI接口，并进入4字节地址模式
 */
void W25Q256_Init(void)
{
    s_spi = BSP_SPI_GetInterface();

    // 唤醒Flash
    W25Q256_WakeUp();

    // 进入4字节地址模式（W25Q256需要32位地址访问全部32MB）
    s_spi->CS.SetLow();
    SoftSPI_WriteByte(s_spi, W25Q256_CMD_ENTER_4B_MODE);
    s_spi->CS.SetHigh();
}

/**
 * @brief  读取JEDEC ID
 */
void W25Q256_ReadID(uint8_t *manufacturer_id, uint16_t *device_id)
{
    s_spi->CS.SetLow();
    SoftSPI_WriteByte(s_spi, W25Q256_CMD_READ_JEDEC_ID);
    *manufacturer_id = SoftSPI_ReadByte(s_spi);
    *device_id = SoftSPI_ReadByte(s_spi) << 8;
    *device_id |= SoftSPI_ReadByte(s_spi);
    s_spi->CS.SetHigh();
}

/**
 * @brief  读取唯一ID（64位）
 */
void W25Q256_ReadUniqueID(uint8_t *unique_id)
{
    s_spi->CS.SetLow();
    SoftSPI_WriteByte(s_spi, W25Q256_CMD_READ_UNIQUE_ID);

    // 发送4个dummy字节
    for (uint8_t i = 0; i < 4; i++) {
        SoftSPI_WriteByte(s_spi, 0xFF);
    }

    // 读取8字节唯一ID
    for (uint8_t i = 0; i < 8; i++) {
        unique_id[i] = SoftSPI_ReadByte(s_spi);
    }

    s_spi->CS.SetHigh();
}

/**
 * @brief  读取状态寄存器1
 */
static uint8_t W25Q256_ReadStatusReg1(void)
{
    uint8_t status;
    s_spi->CS.SetLow();
    SoftSPI_WriteByte(s_spi, W25Q256_CMD_READ_STATUS_REG1);
    status = SoftSPI_ReadByte(s_spi);
    s_spi->CS.SetHigh();
    return status;
}

/**
 * @brief  等待Flash空闲
 */
void W25Q256_WaitBusy(void)
{
    while (W25Q256_ReadStatusReg1() & W25Q256_STATUS_BUSY) {
        // 等待BUSY位清零
    }
}

/**
 * @brief  写使能
 */
static void W25Q256_WriteEnable(void)
{
    s_spi->CS.SetLow();
    SoftSPI_WriteByte(s_spi, W25Q256_CMD_WRITE_ENABLE);
    s_spi->CS.SetHigh();
}

/**
 * @brief  读取数据（4字节地址模式）
 */
void W25Q256_ReadData(uint32_t addr, uint8_t *buffer, uint32_t len)
{
    s_spi->CS.SetLow();

    // 发送读数据命令（4字节地址）
    SoftSPI_WriteByte(s_spi, W25Q256_CMD_READ_DATA_4B);

    // 发送32位地址
    SoftSPI_WriteByte(s_spi, (addr >> 24) & 0xFF);
    SoftSPI_WriteByte(s_spi, (addr >> 16) & 0xFF);
    SoftSPI_WriteByte(s_spi, (addr >> 8) & 0xFF);
    SoftSPI_WriteByte(s_spi, addr & 0xFF);

    // 读取数据
    for (uint32_t i = 0; i < len; i++) {
        buffer[i] = SoftSPI_ReadByte(s_spi);
    }

    s_spi->CS.SetHigh();
}

/**
 * @brief  快速读取数据
 */
void W25Q256_FastRead(uint32_t addr, uint8_t *buffer, uint32_t len)
{
    s_spi->CS.SetLow();

    SoftSPI_WriteByte(s_spi, W25Q256_CMD_FAST_READ_4B);

    // 发送32位地址
    SoftSPI_WriteByte(s_spi, (addr >> 24) & 0xFF);
    SoftSPI_WriteByte(s_spi, (addr >> 16) & 0xFF);
    SoftSPI_WriteByte(s_spi, (addr >> 8) & 0xFF);
    SoftSPI_WriteByte(s_spi, addr & 0xFF);

    // 发送1个dummy字节
    SoftSPI_WriteByte(s_spi, 0xFF);

    // 读取数据
    for (uint32_t i = 0; i < len; i++) {
        buffer[i] = SoftSPI_ReadByte(s_spi);
    }

    s_spi->CS.SetHigh();
}

/**
 * @brief  写入一页数据（256字节以内）
 */
void W25Q256_WritePage(uint32_t addr, const uint8_t *buffer, uint32_t len)
{
    if (len > W25Q256_PAGE_SIZE) {
        len = W25Q256_PAGE_SIZE;
    }

    W25Q256_WriteEnable();

    s_spi->CS.SetLow();

    // 发送页编程命令（4字节地址）
    SoftSPI_WriteByte(s_spi, W25Q256_CMD_PAGE_PROGRAM_4B);

    // 发送32位地址
    SoftSPI_WriteByte(s_spi, (addr >> 24) & 0xFF);
    SoftSPI_WriteByte(s_spi, (addr >> 16) & 0xFF);
    SoftSPI_WriteByte(s_spi, (addr >> 8) & 0xFF);
    SoftSPI_WriteByte(s_spi, addr & 0xFF);

    // 写入数据
    for (uint32_t i = 0; i < len; i++) {
        SoftSPI_WriteByte(s_spi, buffer[i]);
    }

    s_spi->CS.SetHigh();

    W25Q256_WaitBusy();
}

/**
 * @brief  写入数据（自动处理跨页）
 */
void W25Q256_WriteData(uint32_t addr, const uint8_t *buffer, uint32_t len)
{
    uint32_t page_remain = W25Q256_PAGE_SIZE - (addr % W25Q256_PAGE_SIZE);

    if (len <= page_remain) {
        // 数据在同一页内
        W25Q256_WritePage(addr, buffer, len);
    } else {
        // 数据跨页
        // 先写第一页剩余部分
        W25Q256_WritePage(addr, buffer, page_remain);

        addr += page_remain;
        buffer += page_remain;
        len -= page_remain;

        // 写完整页
        while (len >= W25Q256_PAGE_SIZE) {
            W25Q256_WritePage(addr, buffer, W25Q256_PAGE_SIZE);
            addr += W25Q256_PAGE_SIZE;
            buffer += W25Q256_PAGE_SIZE;
            len -= W25Q256_PAGE_SIZE;
        }

        // 写最后不足一页的数据
        if (len > 0) {
            W25Q256_WritePage(addr, buffer, len);
        }
    }
}

/**
 * @brief  擦除扇区（4KB）
 */
void W25Q256_EraseSector(uint32_t addr)
{
    W25Q256_WriteEnable();

    s_spi->CS.SetLow();

    // 发送扇区擦除命令（4字节地址）
    SoftSPI_WriteByte(s_spi, W25Q256_CMD_SECTOR_ERASE_4B);

    // 发送32位地址
    SoftSPI_WriteByte(s_spi, (addr >> 24) & 0xFF);
    SoftSPI_WriteByte(s_spi, (addr >> 16) & 0xFF);
    SoftSPI_WriteByte(s_spi, (addr >> 8) & 0xFF);
    SoftSPI_WriteByte(s_spi, addr & 0xFF);

    s_spi->CS.SetHigh();

    W25Q256_WaitBusy();
}

/**
 * @brief  擦除块（64KB）
 */
void W25Q256_EraseBlock64K(uint32_t addr)
{
    W25Q256_WriteEnable();

    s_spi->CS.SetLow();

    SoftSPI_WriteByte(s_spi, W25Q256_CMD_BLOCK_ERASE_64K_4B);

    SoftSPI_WriteByte(s_spi, (addr >> 24) & 0xFF);
    SoftSPI_WriteByte(s_spi, (addr >> 16) & 0xFF);
    SoftSPI_WriteByte(s_spi, (addr >> 8) & 0xFF);
    SoftSPI_WriteByte(s_spi, addr & 0xFF);

    s_spi->CS.SetHigh();

    W25Q256_WaitBusy();
}

/**
 * @brief  擦除整个芯片
 */
void W25Q256_EraseChip(void)
{
    W25Q256_WriteEnable();

    s_spi->CS.SetLow();
    SoftSPI_WriteByte(s_spi, W25Q256_CMD_CHIP_ERASE);
    s_spi->CS.SetHigh();

    W25Q256_WaitBusy();  // 这个操作需要很长时间（20-100秒）
}

/**
 * @brief  进入掉电模式
 */
void W25Q256_PowerDown(void)
{
    s_spi->CS.SetLow();
    SoftSPI_WriteByte(s_spi, W25Q256_CMD_POWER_DOWN);
    s_spi->CS.SetHigh();
}

/**
 * @brief  唤醒（退出掉电模式）
 */
void W25Q256_WakeUp(void)
{
    s_spi->CS.SetLow();
    SoftSPI_WriteByte(s_spi, W25Q256_CMD_RELEASE_POWER_DOWN);
    s_spi->CS.SetHigh();
}

/**
 * @brief  自检（检查Flash是否正常工作）
 */
uint8_t W25Q256_SelfTest(void)
{
    uint8_t manufacturer_id;
    uint16_t device_id;

    W25Q256_ReadID(&manufacturer_id, &device_id);

    // 检查ID是否正确
    if (manufacturer_id == W25Q256_MANUFACTURER_ID &&
        device_id == W25Q256_DEVICE_ID) {
        return 1;  // 正常
    }

    return 0;  // 异常
}
