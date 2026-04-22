/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : tft_driver.c
  * @brief          : TFT驱动层实现（优化版 - 非阻塞）
  ******************************************************************************
  * @attention
  *
  * 优化要点：
  * 1. 使用状态机替代HAL_Delay，实现非阻塞初始化
  * 2. 完全依赖BSP层抽象接口，无STM32特定代码
  * 3. 初始化总耗时155ms，分散到多次调用中
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "tft_driver.h"
#include "bsp_lcd.h"
#include <stddef.h>

/* Private defines -----------------------------------------------------------*/
// ST7789V命令定义
#define TFT_CMD_NOP       0x00U
#define TFT_CMD_SWRESET   0x01U
#define TFT_CMD_SLPIN     0x10U
#define TFT_CMD_SLPOUT    0x11U
#define TFT_CMD_INVOFF    0x20U
#define TFT_CMD_INVON     0x21U
#define TFT_CMD_DISPOFF   0x28U
#define TFT_CMD_DISPON    0x29U
#define TFT_CMD_CASET     0x2AU
#define TFT_CMD_RASET     0x2BU
#define TFT_CMD_RAMWR     0x2CU
#define TFT_CMD_MADCTL    0x36U
#define TFT_CMD_COLMOD    0x3AU

/* Private variables ---------------------------------------------------------*/
static TFT_InitState_t s_init_state = TFT_INIT_IDLE;
static uint32_t s_state_timestamp = 0;

static const TFT_PanelConfig_t s_panel_config = {
    .madctl = 0x00U,
    .x_offset = TFT_X_OFFSET,
    .y_offset = TFT_Y_OFFSET,
    .panel_width = TFT_WIDTH,
    .panel_height = TFT_HEIGHT
};

/* Private function prototypes -----------------------------------------------*/
static void TFT_ConfigureRegisters(void);
static void TFT_WriteColorRepeat(uint16_t color, uint32_t count);

/* Exported functions --------------------------------------------------------*/

void TFT_InitStart(void)
{
    s_init_state = TFT_INIT_RESET_LOW;
    s_state_timestamp = 0;
}

uint8_t TFT_InitProcess(void)
{
    const struct lcd_ops *ops = BSP_LCD_GetOps();
    const lcd_interface_t *iface = BSP_LCD_GetInterface();
    uint32_t current_tick = ops->get_tick_ms();
    uint32_t elapsed;

    switch (s_init_state) {
        case TFT_INIT_IDLE:
            return 0;

        case TFT_INIT_RESET_LOW:
            // 复位拉低
            ops->pin_write(&iface->res, 0);
            s_state_timestamp = current_tick;
            s_init_state = TFT_INIT_RESET_HIGH;
            return 0;

        case TFT_INIT_RESET_HIGH:
            elapsed = current_tick - s_state_timestamp;
            if (elapsed >= 10U) {  // 等待10ms
                // 复位拉高
                ops->pin_write(&iface->res, 1);
                s_state_timestamp = current_tick;
                s_init_state = TFT_INIT_SLEEP_OUT;
            }
            return 0;

        case TFT_INIT_SLEEP_OUT:
            elapsed = current_tick - s_state_timestamp;
            if (elapsed >= 5U) {  // 等待5ms
                // 退出睡眠模式
                TFT_WriteCommand(TFT_CMD_SLPOUT);
                s_state_timestamp = current_tick;
                s_init_state = TFT_INIT_CONFIG_REGS;
            }
            return 0;

        case TFT_INIT_CONFIG_REGS:
            elapsed = current_tick - s_state_timestamp;
            if (elapsed >= 120U) {  // 等待120ms
                // 配置寄存器
                TFT_ConfigureRegisters();
                s_state_timestamp = current_tick;
                s_init_state = TFT_INIT_DISPLAY_ON;
            }
            return 0;

        case TFT_INIT_DISPLAY_ON:
            elapsed = current_tick - s_state_timestamp;
            if (elapsed >= 20U) {  // 等待20ms
                // 开启显示
                TFT_WriteCommand(TFT_CMD_INVON);
                TFT_WriteCommand(TFT_CMD_DISPON);

                // 设置默认窗口
                TFT_SetAddressWindow(0U, 0U,
                    (uint16_t)(s_panel_config.panel_width - 1U),
                    (uint16_t)(s_panel_config.panel_height - 1U));

                s_init_state = TFT_INIT_COMPLETED;
                return 1;  // 初始化完成
            }
            return 0;

        case TFT_INIT_COMPLETED:
            return 1;

        default:
            s_init_state = TFT_INIT_IDLE;
            return 0;
    }
}

uint8_t TFT_IsReady(void)
{
    return (s_init_state == TFT_INIT_COMPLETED) ? 1U : 0U;
}

void TFT_SetBacklight(uint8_t percent)
{
    BSP_LCD_SetBacklight(percent);
}

const TFT_PanelConfig_t* TFT_GetPanelConfig(void)
{
    return &s_panel_config;
}

void TFT_WriteCommand(uint8_t command)
{
    const struct lcd_ops *ops = BSP_LCD_GetOps();
    const lcd_interface_t *iface = BSP_LCD_GetInterface();

    ops->pin_write(&iface->dc, 0);  // DC=0: 命令模式
    ops->pin_write(&iface->cs, 0);  // CS=0: 选中
    ops->spi_write_byte(command);
    ops->pin_write(&iface->cs, 1);  // CS=1: 释放
}

void TFT_WriteData8(uint8_t data)
{
    const struct lcd_ops *ops = BSP_LCD_GetOps();
    const lcd_interface_t *iface = BSP_LCD_GetInterface();

    ops->pin_write(&iface->dc, 1);  // DC=1: 数据模式
    ops->pin_write(&iface->cs, 0);  // CS=0: 选中
    ops->spi_write_byte(data);
    ops->pin_write(&iface->cs, 1);  // CS=1: 释放
}

void TFT_WriteData16(uint16_t data)
{
    TFT_WriteData8((uint8_t)(data >> 8));
    TFT_WriteData8((uint8_t)data);
}

void TFT_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    const TFT_PanelConfig_t *panel = &s_panel_config;
    uint16_t gram_x0, gram_x1, gram_y0, gram_y1;

    if ((x0 >= panel->panel_width) || (y0 >= panel->panel_height)) {
        return;
    }

    if (x1 >= panel->panel_width) {
        x1 = (uint16_t)(panel->panel_width - 1U);
    }
    if (y1 >= panel->panel_height) {
        y1 = (uint16_t)(panel->panel_height - 1U);
    }
    if ((x1 < x0) || (y1 < y0)) {
        return;
    }

    gram_x0 = (uint16_t)(x0 + panel->x_offset);
    gram_x1 = (uint16_t)(x1 + panel->x_offset);
    gram_y0 = (uint16_t)(y0 + panel->y_offset);
    gram_y1 = (uint16_t)(y1 + panel->y_offset);

    // 设置列地址
    TFT_WriteCommand(TFT_CMD_CASET);
    TFT_WriteData8((uint8_t)(gram_x0 >> 8));
    TFT_WriteData8((uint8_t)gram_x0);
    TFT_WriteData8((uint8_t)(gram_x1 >> 8));
    TFT_WriteData8((uint8_t)gram_x1);

    // 设置行地址
    TFT_WriteCommand(TFT_CMD_RASET);
    TFT_WriteData8((uint8_t)(gram_y0 >> 8));
    TFT_WriteData8((uint8_t)gram_y0);
    TFT_WriteData8((uint8_t)(gram_y1 >> 8));
    TFT_WriteData8((uint8_t)gram_y1);

    // 开始写入RAM
    TFT_WriteCommand(TFT_CMD_RAMWR);
}

void TFT_PushPixelsRGB565(const uint16_t *pixels, uint32_t count)
{
    const struct lcd_ops *ops = BSP_LCD_GetOps();
    const lcd_interface_t *iface = BSP_LCD_GetInterface();

    if ((pixels == NULL) || (count == 0U)) {
        return;
    }

    ops->pin_write(&iface->dc, 1);  // DC=1: 数据模式
    ops->pin_write(&iface->cs, 0);  // CS=0: 选中

    while (count-- > 0U) {
        uint16_t pixel = *pixels++;
        ops->spi_write_byte((uint8_t)(pixel >> 8));
        ops->spi_write_byte((uint8_t)pixel);
    }

    ops->pin_write(&iface->cs, 1);  // CS=1: 释放
}

void TFT_FlushRectRGB565(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint16_t *pixels)
{
    const struct lcd_ops *ops = BSP_LCD_GetOps();
    const lcd_interface_t *iface = BSP_LCD_GetInterface();
    uint16_t clipped_width, clipped_height, row;

    if ((pixels == NULL) || (width == 0U) || (height == 0U)) {
        return;
    }

    if ((x >= TFT_WIDTH) || (y >= TFT_HEIGHT)) {
        return;
    }

    clipped_width = width;
    clipped_height = height;

    if ((uint32_t)x + clipped_width > TFT_WIDTH) {
        clipped_width = (uint16_t)(TFT_WIDTH - x);
    }
    if ((uint32_t)y + clipped_height > TFT_HEIGHT) {
        clipped_height = (uint16_t)(TFT_HEIGHT - y);
    }

    TFT_SetAddressWindow(x, y,
        (uint16_t)(x + clipped_width - 1U),
        (uint16_t)(y + clipped_height - 1U));

    ops->pin_write(&iface->dc, 1);  // DC=1: 数据模式
    ops->pin_write(&iface->cs, 0);  // CS=0: 选中

    for (row = 0U; row < clipped_height; ++row) {
        const uint16_t *row_pixels = pixels + ((uint32_t)row * width);
        uint16_t column;
        for (column = 0U; column < clipped_width; ++column) {
            uint16_t pixel = row_pixels[column];
            ops->spi_write_byte((uint8_t)(pixel >> 8));
            ops->spi_write_byte((uint8_t)pixel);
        }
    }

    ops->pin_write(&iface->cs, 1);  // CS=1: 释放
}

void TFT_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    if ((x >= TFT_WIDTH) || (y >= TFT_HEIGHT)) {
        return;
    }

    TFT_SetAddressWindow(x, y, x, y);
    TFT_WriteColorRepeat(color, 1U);
}

void TFT_FillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color)
{
    uint16_t clipped_width, clipped_height;
    uint32_t pixel_count;

    if ((width == 0U) || (height == 0U)) {
        return;
    }

    if ((x >= TFT_WIDTH) || (y >= TFT_HEIGHT)) {
        return;
    }

    clipped_width = width;
    clipped_height = height;

    if ((uint32_t)x + clipped_width > TFT_WIDTH) {
        clipped_width = (uint16_t)(TFT_WIDTH - x);
    }
    if ((uint32_t)y + clipped_height > TFT_HEIGHT) {
        clipped_height = (uint16_t)(TFT_HEIGHT - y);
    }

    TFT_SetAddressWindow(x, y,
        (uint16_t)(x + clipped_width - 1U),
        (uint16_t)(y + clipped_height - 1U));

    pixel_count = (uint32_t)clipped_width * clipped_height;
    TFT_WriteColorRepeat(color, pixel_count);
}

void TFT_FillScreen(uint16_t color)
{
    TFT_FillRect(0U, 0U, TFT_WIDTH, TFT_HEIGHT, color);
}

/* Private functions ---------------------------------------------------------*/

static void TFT_ConfigureRegisters(void)
{
    const TFT_PanelConfig_t *panel = &s_panel_config;

    // 帧率控制
    TFT_WriteCommand(0xB2U);
    TFT_WriteData8(0x0CU);
    TFT_WriteData8(0x0CU);
    TFT_WriteData8(0x00U);
    TFT_WriteData8(0x33U);
    TFT_WriteData8(0x33U);

    // 电源控制
    TFT_WriteCommand(0xB7U);
    TFT_WriteData8(0x35U);

    TFT_WriteCommand(0xBBU);
    TFT_WriteData8(0x19U);

    TFT_WriteCommand(0xC0U);
    TFT_WriteData8(0x2CU);

    TFT_WriteCommand(0xC2U);
    TFT_WriteData8(0x01U);

    TFT_WriteCommand(0xC3U);
    TFT_WriteData8(0x12U);

    TFT_WriteCommand(0xC4U);
    TFT_WriteData8(0x20U);

    TFT_WriteCommand(0xC6U);
    TFT_WriteData8(0x0FU);

    TFT_WriteCommand(0xD0U);
    TFT_WriteData8(0xA4U);
    TFT_WriteData8(0xA1U);

    // Gamma设置
    TFT_WriteCommand(0xE0U);
    TFT_WriteData8(0xD0U);
    TFT_WriteData8(0x04U);
    TFT_WriteData8(0x0DU);
    TFT_WriteData8(0x11U);
    TFT_WriteData8(0x13U);
    TFT_WriteData8(0x2BU);
    TFT_WriteData8(0x3FU);
    TFT_WriteData8(0x54U);
    TFT_WriteData8(0x4CU);
    TFT_WriteData8(0x18U);
    TFT_WriteData8(0x0DU);
    TFT_WriteData8(0x0BU);
    TFT_WriteData8(0x1FU);
    TFT_WriteData8(0x23U);

    TFT_WriteCommand(0xE1U);
    TFT_WriteData8(0xD0U);
    TFT_WriteData8(0x04U);
    TFT_WriteData8(0x0CU);
    TFT_WriteData8(0x11U);
    TFT_WriteData8(0x13U);
    TFT_WriteData8(0x2CU);
    TFT_WriteData8(0x3FU);
    TFT_WriteData8(0x44U);
    TFT_WriteData8(0x51U);
    TFT_WriteData8(0x2FU);
    TFT_WriteData8(0x1FU);
    TFT_WriteData8(0x1FU);
    TFT_WriteData8(0x20U);
    TFT_WriteData8(0x23U);

    // 内存访问控制
    TFT_WriteCommand(TFT_CMD_MADCTL);
    TFT_WriteData8(panel->madctl);

    // 像素格式（RGB565）
    TFT_WriteCommand(TFT_CMD_COLMOD);
    TFT_WriteData8(0x55U);

    TFT_WriteCommand(0xE7U);
    TFT_WriteData8(0x00U);
}

static void TFT_WriteColorRepeat(uint16_t color, uint32_t count)
{
    const struct lcd_ops *ops = BSP_LCD_GetOps();
    const lcd_interface_t *iface = BSP_LCD_GetInterface();
    uint8_t color_hi = (uint8_t)(color >> 8);
    uint8_t color_lo = (uint8_t)color;

    ops->pin_write(&iface->dc, 1);  // DC=1: 数据模式
    ops->pin_write(&iface->cs, 0);  // CS=0: 选中

    while (count-- > 0U) {
        ops->spi_write_byte(color_hi);
        ops->spi_write_byte(color_lo);
    }

    ops->pin_write(&iface->cs, 1);  // CS=1: 释放
}
