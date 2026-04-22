/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : tft_driver.h
  * @brief          : TFT驱动层（优化版 - 非阻塞 + 分层清晰）
  ******************************************************************************
  * @attention
  *
  * 设计理念：
  * 1. 完全平台无关，只依赖BSP层抽象接口
  * 2. 使用状态机实现非阻塞初始化
  * 3. 消除所有HAL_Delay阻塞
  * 4. 支持DMA传输（未来扩展）
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __TFT_DRIVER_H
#define __TFT_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported constants --------------------------------------------------------*/
#define TFT_WIDTH         240U
#define TFT_HEIGHT        240U
#define TFT_GRAM_HEIGHT   320U
#define TFT_X_OFFSET      0U
#define TFT_Y_OFFSET      40U

/* 颜色定义 */
#define TFT_COLOR_BLACK   0x0000U
#define TFT_COLOR_WHITE   0xFFFFU
#define TFT_COLOR_RED     0xF800U
#define TFT_COLOR_GREEN   0x07E0U
#define TFT_COLOR_BLUE    0x001FU
#define TFT_COLOR_YELLOW  0xFFE0U
#define TFT_COLOR_CYAN    0x07FFU
#define TFT_COLOR_MAGENTA 0xF81FU
#define TFT_COLOR_ORANGE  0xFD20U
#define TFT_COLOR_GRAY    0x8410U

/* Exported types ------------------------------------------------------------*/

/**
 * @brief TFT初始化状态
 */
typedef enum {
    TFT_INIT_IDLE = 0,           // 未初始化
    TFT_INIT_RESET_LOW,          // 复位拉低
    TFT_INIT_RESET_HIGH,         // 复位拉高
    TFT_INIT_SLEEP_OUT,          // 退出睡眠模式
    TFT_INIT_CONFIG_REGS,        // 配置寄存器
    TFT_INIT_DISPLAY_ON,         // 开启显示
    TFT_INIT_COMPLETED           // 初始化完成
} TFT_InitState_t;

/**
 * @brief TFT面板配置
 */
typedef struct {
    uint8_t madctl;
    uint16_t x_offset;
    uint16_t y_offset;
    uint16_t panel_width;
    uint16_t panel_height;
} TFT_PanelConfig_t;

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief  启动TFT初始化（非阻塞）
 * @note   调用后需要周期性调用TFT_InitProcess()
 * @retval None
 */
void TFT_InitStart(void);

/**
 * @brief  TFT初始化状态机处理（非阻塞）
 * @note   在主循环或定时任务中周期性调用
 * @retval 1=初始化完成, 0=初始化中
 */
uint8_t TFT_InitProcess(void);

/**
 * @brief  检查TFT是否已初始化
 * @retval 1=已初始化, 0=未初始化
 */
uint8_t TFT_IsReady(void);

/**
 * @brief  设置背光亮度
 * @param  percent: 亮度百分比 (0-100)
 * @retval None
 */
void TFT_SetBacklight(uint8_t percent);

/**
 * @brief  获取面板配置
 * @retval 面板配置指针
 */
const TFT_PanelConfig_t* TFT_GetPanelConfig(void);

/**
 * @brief  写命令
 * @param  command: 命令字节
 * @retval None
 */
void TFT_WriteCommand(uint8_t command);

/**
 * @brief  写8位数据
 * @param  data: 数据字节
 * @retval None
 */
void TFT_WriteData8(uint8_t data);

/**
 * @brief  写16位数据
 * @param  data: 数据字
 * @retval None
 */
void TFT_WriteData16(uint16_t data);

/**
 * @brief  设置绘图窗口
 * @param  x0: 起始X坐标
 * @param  y0: 起始Y坐标
 * @param  x1: 结束X坐标
 * @param  y1: 结束Y坐标
 * @retval None
 */
void TFT_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

/**
 * @brief  推送像素数据（RGB565）
 * @param  pixels: 像素数据指针
 * @param  count: 像素数量
 * @retval None
 */
void TFT_PushPixelsRGB565(const uint16_t *pixels, uint32_t count);

/**
 * @brief  刷新矩形区域（RGB565）
 * @param  x: 起始X坐标
 * @param  y: 起始Y坐标
 * @param  width: 宽度
 * @param  height: 高度
 * @param  pixels: 像素数据指针
 * @retval None
 */
void TFT_FlushRectRGB565(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint16_t *pixels);

/**
 * @brief  绘制单个像素
 * @param  x: X坐标
 * @param  y: Y坐标
 * @param  color: 颜色（RGB565）
 * @retval None
 */
void TFT_DrawPixel(uint16_t x, uint16_t y, uint16_t color);

/**
 * @brief  填充矩形
 * @param  x: 起始X坐标
 * @param  y: 起始Y坐标
 * @param  width: 宽度
 * @param  height: 高度
 * @param  color: 颜色（RGB565）
 * @retval None
 */
void TFT_FillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);

/**
 * @brief  填充整个屏幕
 * @param  color: 颜色（RGB565）
 * @retval None
 */
void TFT_FillScreen(uint16_t color);

#ifdef __cplusplus
}
#endif

#endif /* __TFT_DRIVER_H */
