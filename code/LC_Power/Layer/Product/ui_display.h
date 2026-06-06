/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : ui_display.h
  * @brief          : Pure UI display drawing layer.
  ******************************************************************************
  * @attention
  *
  * Author: LCYX
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __UI_DISPLAY_H
#define __UI_DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void UI_Display_Init(void);
void UI_Display_Process(void);
void UI_DrawChinese(int16_t x, int16_t y, const char *str, uint16_t color);

#ifdef __cplusplus
}
#endif

#endif /* __UI_DISPLAY_H */

/* Author: LCYX */
