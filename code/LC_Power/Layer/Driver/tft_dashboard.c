#include "tft_dashboard.h"

#define DASHBOARD_CENTER_X          120
#define DASHBOARD_CENTER_Y          120
#define DASHBOARD_FACE_RADIUS       80
#define DASHBOARD_POINTER_RADIUS    66
#define DASHBOARD_POINTER_TAIL      12
#define DASHBOARD_MINOR_TICK_INNER  90
#define DASHBOARD_MINOR_TICK_OUTER  98
#define DASHBOARD_MAJOR_TICK_INNER  86
#define DASHBOARD_MAJOR_TICK_OUTER  98
#define DASHBOARD_CENTER_CAP_RADIUS 8
#define DASHBOARD_VALUE_BOX_X       48
#define DASHBOARD_VALUE_BOX_Y       184
#define DASHBOARD_VALUE_BOX_W       144
#define DASHBOARD_VALUE_BOX_H       30
#define DASHBOARD_VALUE_INNER_X     52
#define DASHBOARD_VALUE_INNER_Y     188
#define DASHBOARD_VALUE_INNER_W     136
#define DASHBOARD_VALUE_INNER_H     22
#define DASHBOARD_UPDATE_PERIOD_MS  25U

#define DASHBOARD_BG_COLOR          0x0841U
#define DASHBOARD_FACE_COLOR        0x10C3U
#define DASHBOARD_FACE_BORDER       0x4208U
#define DASHBOARD_MAJOR_TICK_COLOR  TFT_COLOR_WHITE
#define DASHBOARD_MINOR_TICK_COLOR  0x94B2U
#define DASHBOARD_VALUE_BG          0x0000U
#define DASHBOARD_VALUE_BORDER      0xC618U
#define DASHBOARD_TITLE_COLOR       TFT_COLOR_WHITE

static uint8_t s_dashboard_initialized;
static uint8_t s_dashboard_value;
static int8_t s_dashboard_direction = 1;
static int16_t s_previous_angle = 32767;
static uint32_t s_last_update_ms;

static void TFT_Dashboard_DrawStatic(void);
static void TFT_Dashboard_DrawTicks(void);
static void TFT_Dashboard_DrawLabels(void);
static void TFT_Dashboard_DrawValueBoxFrame(void);
static void TFT_Dashboard_DrawNeedle(int16_t angle, uint16_t color);
static void TFT_Dashboard_DrawCenterCap(void);
static void TFT_Dashboard_UpdateDynamic(void);
static int16_t TFT_Dashboard_ValueToAngle(uint8_t value);
static uint16_t TFT_Dashboard_ValueColor(uint8_t value);
static void TFT_Dashboard_FormatValue(char *buffer, uint8_t value);

static void TFT_Dashboard_DrawTicks(void)
{
  uint8_t value;

  for (value = 0U; value <= 100U; value = (uint8_t)(value + 5U))
  {
    int16_t angle = (int16_t)(-120 + (((int16_t)value * 240) / 100));
    int16_t x0;
    int16_t y0;
    int16_t x1;
    int16_t y1;
    uint16_t color;

    if ((value % 10U) == 0U)
    {
      TFTGFX_PolarToPoint(DASHBOARD_CENTER_X,
                          DASHBOARD_CENTER_Y,
                          DASHBOARD_MAJOR_TICK_INNER,
                          angle,
                          &x0,
                          &y0);
      color = DASHBOARD_MAJOR_TICK_COLOR;
    }
    else
    {
      TFTGFX_PolarToPoint(DASHBOARD_CENTER_X,
                          DASHBOARD_CENTER_Y,
                          DASHBOARD_MINOR_TICK_INNER,
                          angle,
                          &x0,
                          &y0);
      color = DASHBOARD_MINOR_TICK_COLOR;
    }

    TFTGFX_PolarToPoint(DASHBOARD_CENTER_X,
                        DASHBOARD_CENTER_Y,
                        DASHBOARD_MAJOR_TICK_OUTER,
                        angle,
                        &x1,
                        &y1);
    TFTGFX_DrawLine(x0, y0, x1, y1, color);
  }
}

static void TFT_Dashboard_DrawLabels(void)
{
  const char *title = "POWER METER";
  uint16_t title_width = TFTGFX_MeasureStringWidth(title, 2U);

  TFTGFX_DrawString((int16_t)((TFT_WIDTH - title_width) / 2U), 26, title, DASHBOARD_TITLE_COLOR, 2U);

  TFTGFX_DrawString(22, 150, "0", TFT_COLOR_WHITE, 1U);
  TFTGFX_DrawString(104, 72, "50", TFT_COLOR_WHITE, 1U);
  TFTGFX_DrawString(198, 150, "100", TFT_COLOR_WHITE, 1U);
}

static void TFT_Dashboard_DrawValueBoxFrame(void)
{
  TFTGFX_DrawRect(DASHBOARD_VALUE_BOX_X,
                  DASHBOARD_VALUE_BOX_Y,
                  DASHBOARD_VALUE_BOX_W,
                  DASHBOARD_VALUE_BOX_H,
                  DASHBOARD_VALUE_BORDER);
  TFTGFX_DrawRect((int16_t)(DASHBOARD_VALUE_BOX_X + 1),
                  (int16_t)(DASHBOARD_VALUE_BOX_Y + 1),
                  (int16_t)(DASHBOARD_VALUE_BOX_W - 2),
                  (int16_t)(DASHBOARD_VALUE_BOX_H - 2),
                  DASHBOARD_FACE_BORDER);
}

static void TFT_Dashboard_DrawStatic(void)
{
  TFT_FillScreen(DASHBOARD_BG_COLOR);
  TFTGFX_DrawRing(DASHBOARD_CENTER_X, DASHBOARD_CENTER_Y, 104, 101, DASHBOARD_FACE_BORDER);
  TFTGFX_FillCircle(DASHBOARD_CENTER_X, DASHBOARD_CENTER_Y, DASHBOARD_FACE_RADIUS, DASHBOARD_FACE_COLOR);
  TFTGFX_DrawCircle(DASHBOARD_CENTER_X, DASHBOARD_CENTER_Y, DASHBOARD_FACE_RADIUS, TFT_COLOR_WHITE);

  TFT_Dashboard_DrawTicks();
  TFT_Dashboard_DrawLabels();
  TFT_Dashboard_DrawValueBoxFrame();
}

static int16_t TFT_Dashboard_ValueToAngle(uint8_t value)
{
  return (int16_t)(-120 + (((int16_t)value * 240) / 100));
}

static uint16_t TFT_Dashboard_ValueColor(uint8_t value)
{
  if (value < 40U)
  {
    return TFT_COLOR_GREEN;
  }
  if (value < 75U)
  {
    return TFT_COLOR_YELLOW;
  }
  return TFT_COLOR_RED;
}

static void TFT_Dashboard_DrawNeedle(int16_t angle, uint16_t color)
{
  int16_t tip_x;
  int16_t tip_y;
  int16_t tail_x;
  int16_t tail_y;

  TFTGFX_PolarToPoint(DASHBOARD_CENTER_X,
                      DASHBOARD_CENTER_Y,
                      DASHBOARD_POINTER_RADIUS,
                      angle,
                      &tip_x,
                      &tip_y);
  TFTGFX_PolarToPoint(DASHBOARD_CENTER_X,
                      DASHBOARD_CENTER_Y,
                      DASHBOARD_POINTER_TAIL,
                      (int16_t)(angle + 180),
                      &tail_x,
                      &tail_y);
  TFTGFX_DrawLine(tail_x, tail_y, tip_x, tip_y, color);
}

static void TFT_Dashboard_DrawCenterCap(void)
{
  TFTGFX_FillCircle(DASHBOARD_CENTER_X, DASHBOARD_CENTER_Y, DASHBOARD_CENTER_CAP_RADIUS, TFT_COLOR_WHITE);
  TFTGFX_FillCircle(DASHBOARD_CENTER_X, DASHBOARD_CENTER_Y, (int16_t)(DASHBOARD_CENTER_CAP_RADIUS - 3), DASHBOARD_FACE_BORDER);
}

static void TFT_Dashboard_FormatValue(char *buffer, uint8_t value)
{
  buffer[0] = 'L';
  buffer[1] = 'O';
  buffer[2] = 'A';
  buffer[3] = 'D';
  buffer[4] = ' ';
  buffer[5] = (char)('0' + (value / 100U));
  buffer[6] = (char)('0' + ((value / 10U) % 10U));
  buffer[7] = (char)('0' + (value % 10U));
  buffer[8] = '%';
  buffer[9] = '\0';
}

static void TFT_Dashboard_UpdateDynamic(void)
{
  char value_text[10];
  uint16_t value_width;
  int16_t current_angle = TFT_Dashboard_ValueToAngle(s_dashboard_value);
  uint16_t pointer_color = TFT_Dashboard_ValueColor(s_dashboard_value);

  if (s_previous_angle != 32767)
  {
    TFT_Dashboard_DrawNeedle(s_previous_angle, DASHBOARD_FACE_COLOR);
  }

  TFT_Dashboard_DrawNeedle(current_angle, pointer_color);
  TFT_Dashboard_DrawCenterCap();

  TFTGFX_FillRect(DASHBOARD_VALUE_INNER_X,
                  DASHBOARD_VALUE_INNER_Y,
                  DASHBOARD_VALUE_INNER_W,
                  DASHBOARD_VALUE_INNER_H,
                  DASHBOARD_VALUE_BG);

  TFT_Dashboard_FormatValue(value_text, s_dashboard_value);
  value_width = TFTGFX_MeasureStringWidth(value_text, 2U);
  TFTGFX_DrawString((int16_t)((TFT_WIDTH - value_width) / 2U),
                    (int16_t)(DASHBOARD_VALUE_INNER_Y + 3),
                    value_text,
                    pointer_color,
                    2U);

  s_previous_angle = current_angle;
}

void TFT_Dashboard_InitStatic(void)
{
  s_dashboard_value = 0U;
  s_dashboard_direction = 1;
  s_previous_angle = 32767;
  s_last_update_ms = HAL_GetTick();

  TFT_Dashboard_DrawStatic();
  s_dashboard_initialized = 1U;
}

void TFT_Dashboard_Init(void)
{
  TFT_Dashboard_InitStatic();
  TFT_Dashboard_UpdateDynamic();
}

void TFT_Dashboard_Task(uint32_t now_ms)
{
  if (s_dashboard_initialized == 0U)
  {
    TFT_Dashboard_Init();
    return;
  }

  if (s_previous_angle == 32767)
  {
    TFT_Dashboard_UpdateDynamic();
    return;
  }

  while ((uint32_t)(now_ms - s_last_update_ms) >= DASHBOARD_UPDATE_PERIOD_MS)
  {
    s_last_update_ms += DASHBOARD_UPDATE_PERIOD_MS;

    if (s_dashboard_direction > 0)
    {
      if (s_dashboard_value >= 100U)
      {
        s_dashboard_direction = -1;
        if (s_dashboard_value > 0U)
        {
          --s_dashboard_value;
        }
      }
      else
      {
        ++s_dashboard_value;
      }
    }
    else
    {
      if (s_dashboard_value == 0U)
      {
        s_dashboard_direction = 1;
        ++s_dashboard_value;
      }
      else
      {
        --s_dashboard_value;
      }
    }

    TFT_Dashboard_UpdateDynamic();
  }
}
