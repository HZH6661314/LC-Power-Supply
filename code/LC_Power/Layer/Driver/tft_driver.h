/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : tft_driver.h
  * @brief          : Non-blocking TFT driver interface.
  ******************************************************************************
  * @attention
  *
  * Author: LCYX
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __TFT_DRIVER_H
#define __TFT_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define TFT_WIDTH         240U
#define TFT_HEIGHT        240U
#define TFT_GRAM_HEIGHT   320U
#define TFT_X_OFFSET      0U
#define TFT_Y_OFFSET      40U

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

typedef enum {
    TFT_INIT_IDLE = 0,
    TFT_INIT_RESET_LOW,
    TFT_INIT_RESET_HIGH,
    TFT_INIT_SLEEP_OUT,
    TFT_INIT_CONFIG_REGS,
    TFT_INIT_DISPLAY_ON,
    TFT_INIT_COMPLETED
} TFT_InitState_t;

typedef struct {
    uint8_t madctl;
    uint16_t x_offset;
    uint16_t y_offset;
    uint16_t panel_width;
    uint16_t panel_height;
} TFT_PanelConfig_t;

void TFT_InitStart(void);
uint8_t TFT_InitProcess(void);
uint8_t TFT_IsReady(void);
void TFT_SetBacklight(uint8_t percent);
const TFT_PanelConfig_t* TFT_GetPanelConfig(void);

void TFT_WriteCommand(uint8_t command);
void TFT_WriteData8(uint8_t data);
void TFT_WriteData16(uint16_t data);

void TFT_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void TFT_PushPixelsRGB565(const uint16_t *pixels, uint32_t count);
void TFT_FlushRectRGB565(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint16_t *pixels);

void TFT_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void TFT_FillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
void TFT_FillScreen(uint16_t color);

#ifdef __cplusplus
}
#endif

#endif /* __TFT_DRIVER_H */
