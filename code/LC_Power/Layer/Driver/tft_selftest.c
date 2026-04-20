#include "tft_selftest.h"

static void TFT_SelfTest_DrawCorners(void);
static void TFT_SelfTest_DrawCenterCross(void);
static void TFT_SelfTest_DrawBorder(void);
static void TFT_SelfTest_DrawLabels(void);

static void TFT_SelfTest_DrawBorder(void)
{
  TFT_FillScreen(TFT_COLOR_BLACK);

  TFT_FillRect(0U, 0U, TFT_WIDTH, 6U, TFT_COLOR_RED);
  TFT_FillRect(0U, (uint16_t)(TFT_HEIGHT - 6U), TFT_WIDTH, 6U, TFT_COLOR_BLUE);
  TFT_FillRect(0U, 0U, 6U, TFT_HEIGHT, TFT_COLOR_GREEN);
  TFT_FillRect((uint16_t)(TFT_WIDTH - 6U), 0U, 6U, TFT_HEIGHT, TFT_COLOR_YELLOW);

  TFTGFX_DrawRect(6, 6, (int16_t)(TFT_WIDTH - 12U), (int16_t)(TFT_HEIGHT - 12U), TFT_COLOR_WHITE);
}

static void TFT_SelfTest_DrawCorners(void)
{
  TFTGFX_FillRect(12, 12, 18, 18, TFT_COLOR_WHITE);
  TFTGFX_FillRect((int16_t)(TFT_WIDTH - 30U), 12, 18, 18, TFT_COLOR_WHITE);
  TFTGFX_FillRect(12, (int16_t)(TFT_HEIGHT - 30U), 18, 18, TFT_COLOR_WHITE);
  TFTGFX_FillRect((int16_t)(TFT_WIDTH - 30U), (int16_t)(TFT_HEIGHT - 30U), 18, 18, TFT_COLOR_WHITE);

  TFTGFX_DrawString(14, 35, "TL", TFT_COLOR_WHITE, 1U);
  TFTGFX_DrawString((int16_t)(TFT_WIDTH - 28U), 35, "TR", TFT_COLOR_WHITE, 1U);
  TFTGFX_DrawString(14, (int16_t)(TFT_HEIGHT - 44U), "BL", TFT_COLOR_WHITE, 1U);
  TFTGFX_DrawString((int16_t)(TFT_WIDTH - 28U), (int16_t)(TFT_HEIGHT - 44U), "BR", TFT_COLOR_WHITE, 1U);
}

static void TFT_SelfTest_DrawCenterCross(void)
{
  TFTGFX_DrawLine((int16_t)(TFT_WIDTH / 2U), 30, (int16_t)(TFT_WIDTH / 2U), (int16_t)(TFT_HEIGHT - 31U), TFT_COLOR_CYAN);
  TFTGFX_DrawLine(30, (int16_t)(TFT_HEIGHT / 2U), (int16_t)(TFT_WIDTH - 31U), (int16_t)(TFT_HEIGHT / 2U), TFT_COLOR_CYAN);
  TFTGFX_FillCircle((int16_t)(TFT_WIDTH / 2U), (int16_t)(TFT_HEIGHT / 2U), 4, TFT_COLOR_MAGENTA);
}

static void TFT_SelfTest_DrawLabels(void)
{
  TFTGFX_DrawString(74, 12, "TOP RED", TFT_COLOR_WHITE, 1U);
  TFTGFX_DrawString(62, (int16_t)(TFT_HEIGHT - 18U), "BOTTOM BLUE", TFT_COLOR_WHITE, 1U);
  TFTGFX_DrawString(72, 104, "CENTER", TFT_COLOR_WHITE, 1U);
}

void TFT_SelfTest_DrawMappingPage(void)
{
  TFT_SelfTest_DrawBorder();
  TFT_SelfTest_DrawCorners();
  TFT_SelfTest_DrawCenterCross();
  TFT_SelfTest_DrawLabels();
}
