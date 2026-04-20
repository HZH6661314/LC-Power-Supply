#include "1.54TFT.h"

#include "tim.h"

#define TFT_CMD_SLPOUT 0x11U
#define TFT_CMD_INVON  0x21U
#define TFT_CMD_DISPON 0x29U
#define TFT_CMD_CASET  0x2AU
#define TFT_CMD_RASET  0x2BU
#define TFT_CMD_RAMWR  0x2CU
#define TFT_CMD_MADCTL 0x36U
#define TFT_CMD_COLMOD 0x3AU

#define TFT_PIN_SET(port, pin)   ((port)->BSRR = (pin))
#define TFT_PIN_RESET(port, pin) ((port)->BSRR = ((uint32_t)(pin) << 16U))

static const TFT_PanelConfig s_tft_panel_config = {
  0x70U,
  0U,
  0U,
  TFT_WIDTH,
  TFT_HEIGHT
};

static uint8_t s_backlight_ready;
static uint8_t s_backlight_fault;

static void TFT_SCL_Low(void);
static void TFT_SCL_High(void);
static void TFT_SDA_Low(void);
static void TFT_SDA_High(void);
static void TFT_CS_Low(void);
static void TFT_CS_High(void);
static void TFT_DC_Low(void);
static void TFT_DC_High(void);
static void TFT_RES_Low(void);
static void TFT_RES_High(void);
static void TFT_WriteBusByte(uint8_t data);
static void TFT_WriteColorRepeat(uint16_t color, uint32_t count);
static void TFT_BacklightEnsureReady(void);
static void TFT_BacklightFatalError(void);
static void TFT_HardwareReset(void);
static void TFT_ConfigureRegisters(void);

const TFT_PanelConfig *TFT_GetPanelConfig(void)
{
  return &s_tft_panel_config;
}

static void TFT_SCL_Low(void)
{
  TFT_PIN_RESET(LCD_SCL_GPIO_Port, LCD_SCL_Pin);
}

static void TFT_SCL_High(void)
{
  TFT_PIN_SET(LCD_SCL_GPIO_Port, LCD_SCL_Pin);
}

static void TFT_SDA_Low(void)
{
  TFT_PIN_RESET(LCD_SDA_GPIO_Port, LCD_SDA_Pin);
}

static void TFT_SDA_High(void)
{
  TFT_PIN_SET(LCD_SDA_GPIO_Port, LCD_SDA_Pin);
}

static void TFT_CS_Low(void)
{
  TFT_PIN_RESET(LCD_CS_GPIO_Port, LCD_CS_Pin);
}

static void TFT_CS_High(void)
{
  TFT_PIN_SET(LCD_CS_GPIO_Port, LCD_CS_Pin);
}

static void TFT_DC_Low(void)
{
  TFT_PIN_RESET(LCD_DC_GPIO_Port, LCD_DC_Pin);
}

static void TFT_DC_High(void)
{
  TFT_PIN_SET(LCD_DC_GPIO_Port, LCD_DC_Pin);
}

static void TFT_RES_Low(void)
{
  TFT_PIN_RESET(LCD_RES_GPIO_Port, LCD_RES_Pin);
}

static void TFT_RES_High(void)
{
  TFT_PIN_SET(LCD_RES_GPIO_Port, LCD_RES_Pin);
}

static void TFT_WriteBusByte(uint8_t data)
{
  uint8_t mask;

  for (mask = 0x80U; mask != 0U; mask >>= 1U)
  {
    TFT_SCL_Low();
    if ((data & mask) != 0U)
    {
      TFT_SDA_High();
    }
    else
    {
      TFT_SDA_Low();
    }
    TFT_SCL_High();
  }
  TFT_SCL_Low();
}

void TFT_WriteCommand(uint8_t command)
{
  TFT_DC_Low();
  TFT_CS_Low();
  TFT_WriteBusByte(command);
  TFT_CS_High();
}

void TFT_WriteData8(uint8_t data)
{
  TFT_DC_High();
  TFT_CS_Low();
  TFT_WriteBusByte(data);
  TFT_CS_High();
}

void TFT_WriteData16(uint16_t data)
{
  TFT_DC_High();
  TFT_CS_Low();
  TFT_WriteBusByte((uint8_t)(data >> 8));
  TFT_WriteBusByte((uint8_t)data);
  TFT_CS_High();
}

static void TFT_WriteColorRepeat(uint16_t color, uint32_t count)
{
  uint8_t high = (uint8_t)(color >> 8);
  uint8_t low = (uint8_t)color;

  TFT_DC_High();
  TFT_CS_Low();
  while (count-- > 0U)
  {
    TFT_WriteBusByte(high);
    TFT_WriteBusByte(low);
  }
  TFT_CS_High();
}

static void TFT_BacklightFatalError(void)
{
  s_backlight_fault = 1U;
  HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
  Error_Handler();
}

static void TFT_BacklightEnsureReady(void)
{
  if (s_backlight_ready != 0U)
  {
    return;
  }

  HAL_TIM_PWM_Stop(&htim16, TIM_CHANNEL_1);

  htim16.Init.Prescaler = 0U;
  htim16.Init.Period = 3599U;
  htim16.Instance->PSC = 0U;
  htim16.Instance->ARR = 3599U;
  __HAL_TIM_SET_COMPARE(&htim16, TIM_CHANNEL_1, 0U);
  __HAL_TIM_SET_COUNTER(&htim16, 0U);
  htim16.Instance->EGR = TIM_EGR_UG;

  if (HAL_TIM_PWM_Start(&htim16, TIM_CHANNEL_1) != HAL_OK)
  {
    TFT_BacklightFatalError();
    return;
  }

  s_backlight_ready = 1U;
}

void TFT_SetBacklight(uint8_t percent)
{
  uint32_t compare;

  if (percent > 100U)
  {
    percent = 100U;
  }

  TFT_BacklightEnsureReady();
  if (s_backlight_fault != 0U)
  {
    return;
  }

  compare = ((uint32_t)(htim16.Instance->ARR + 1U) * percent) / 100U;
  if (compare > htim16.Instance->ARR)
  {
    compare = htim16.Instance->ARR;
  }

  __HAL_TIM_SET_COMPARE(&htim16, TIM_CHANNEL_1, compare);
}

static void TFT_HardwareReset(void)
{
  TFT_CS_High();
  TFT_DC_High();
  TFT_SDA_High();
  TFT_SCL_Low();

  TFT_RES_Low();
  HAL_Delay(10U);
  TFT_RES_High();
  HAL_Delay(5U);
}

static void TFT_ConfigureRegisters(void)
{
  const TFT_PanelConfig *panel = TFT_GetPanelConfig();

  TFT_WriteCommand(TFT_CMD_SLPOUT);
  HAL_Delay(120U);

  TFT_WriteCommand(0xB2U);
  TFT_WriteData8(0x0CU);
  TFT_WriteData8(0x0CU);
  TFT_WriteData8(0x00U);
  TFT_WriteData8(0x33U);
  TFT_WriteData8(0x33U);

  TFT_WriteCommand(0x20U);

  TFT_WriteCommand(0xB7U);
  TFT_WriteData8(0x56U);

  TFT_WriteCommand(0xBBU);
  TFT_WriteData8(0x18U);

  TFT_WriteCommand(0xC0U);
  TFT_WriteData8(0x2CU);

  TFT_WriteCommand(0xC2U);
  TFT_WriteData8(0x01U);

  TFT_WriteCommand(0xC3U);
  TFT_WriteData8(0x1FU);

  TFT_WriteCommand(0xC4U);
  TFT_WriteData8(0x20U);

  TFT_WriteCommand(0xC6U);
  TFT_WriteData8(0x0FU);

  TFT_WriteCommand(0xD0U);
  TFT_WriteData8(0xA6U);
  TFT_WriteData8(0xA1U);

  TFT_WriteCommand(0xE0U);
  TFT_WriteData8(0xD0U);
  TFT_WriteData8(0x0DU);
  TFT_WriteData8(0x14U);
  TFT_WriteData8(0x0BU);
  TFT_WriteData8(0x0BU);
  TFT_WriteData8(0x07U);
  TFT_WriteData8(0x3AU);
  TFT_WriteData8(0x44U);
  TFT_WriteData8(0x50U);
  TFT_WriteData8(0x08U);
  TFT_WriteData8(0x13U);
  TFT_WriteData8(0x13U);
  TFT_WriteData8(0x2DU);
  TFT_WriteData8(0x32U);

  TFT_WriteCommand(0xE1U);
  TFT_WriteData8(0xD0U);
  TFT_WriteData8(0x0DU);
  TFT_WriteData8(0x14U);
  TFT_WriteData8(0x0BU);
  TFT_WriteData8(0x0BU);
  TFT_WriteData8(0x07U);
  TFT_WriteData8(0x3AU);
  TFT_WriteData8(0x44U);
  TFT_WriteData8(0x50U);
  TFT_WriteData8(0x08U);
  TFT_WriteData8(0x13U);
  TFT_WriteData8(0x13U);
  TFT_WriteData8(0x2DU);
  TFT_WriteData8(0x32U);

  TFT_WriteCommand(TFT_CMD_MADCTL);
  TFT_WriteData8(panel->madctl);

  TFT_WriteCommand(TFT_CMD_COLMOD);
  TFT_WriteData8(0x55U);

  TFT_WriteCommand(0xE7U);
  TFT_WriteData8(0x00U);

  TFT_WriteCommand(TFT_CMD_INVON);
  TFT_WriteCommand(TFT_CMD_DISPON);
  HAL_Delay(20U);

  TFT_SetAddressWindow(0U, 0U, (uint16_t)(panel->panel_width - 1U), (uint16_t)(panel->panel_height - 1U));
}

void TFT_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
  const TFT_PanelConfig *panel = TFT_GetPanelConfig();
  uint16_t gram_x0;
  uint16_t gram_x1;
  uint16_t gram_y0;
  uint16_t gram_y1;

  if ((x0 >= panel->panel_width) || (y0 >= panel->panel_height))
  {
    return;
  }

  if (x1 >= panel->panel_width)
  {
    x1 = (uint16_t)(panel->panel_width - 1U);
  }
  if (y1 >= panel->panel_height)
  {
    y1 = (uint16_t)(panel->panel_height - 1U);
  }
  if ((x1 < x0) || (y1 < y0))
  {
    return;
  }

  gram_x0 = (uint16_t)(x0 + panel->x_offset);
  gram_x1 = (uint16_t)(x1 + panel->x_offset);
  gram_y0 = (uint16_t)(y0 + panel->y_offset);
  gram_y1 = (uint16_t)(y1 + panel->y_offset);

  TFT_WriteCommand(TFT_CMD_CASET);
  TFT_WriteData8((uint8_t)(gram_x0 >> 8));
  TFT_WriteData8((uint8_t)gram_x0);
  TFT_WriteData8((uint8_t)(gram_x1 >> 8));
  TFT_WriteData8((uint8_t)gram_x1);

  TFT_WriteCommand(TFT_CMD_RASET);
  TFT_WriteData8((uint8_t)(gram_y0 >> 8));
  TFT_WriteData8((uint8_t)gram_y0);
  TFT_WriteData8((uint8_t)(gram_y1 >> 8));
  TFT_WriteData8((uint8_t)gram_y1);

  TFT_WriteCommand(TFT_CMD_RAMWR);
}

void TFT_PushPixelsRGB565(const uint16_t *pixels, uint32_t count)
{
  if ((pixels == 0) || (count == 0U))
  {
    return;
  }

  TFT_DC_High();
  TFT_CS_Low();
  while (count-- > 0U)
  {
    uint16_t pixel = *pixels++;
    TFT_WriteBusByte((uint8_t)(pixel >> 8));
    TFT_WriteBusByte((uint8_t)pixel);
  }
  TFT_CS_High();
}

void TFT_FlushRectRGB565(uint16_t x,
                         uint16_t y,
                         uint16_t width,
                         uint16_t height,
                         const uint16_t *pixels)
{
  uint16_t clipped_width;
  uint16_t clipped_height;
  uint16_t row;

  if ((pixels == 0) || (width == 0U) || (height == 0U))
  {
    return;
  }

  if ((x >= TFT_WIDTH) || (y >= TFT_HEIGHT))
  {
    return;
  }

  clipped_width = width;
  clipped_height = height;

  if ((uint32_t)x + clipped_width > TFT_WIDTH)
  {
    clipped_width = (uint16_t)(TFT_WIDTH - x);
  }
  if ((uint32_t)y + clipped_height > TFT_HEIGHT)
  {
    clipped_height = (uint16_t)(TFT_HEIGHT - y);
  }

  TFT_SetAddressWindow(x,
                       y,
                       (uint16_t)(x + clipped_width - 1U),
                       (uint16_t)(y + clipped_height - 1U));

  TFT_DC_High();
  TFT_CS_Low();
  for (row = 0U; row < clipped_height; ++row)
  {
    const uint16_t *row_pixels = pixels + ((uint32_t)row * width);
    uint16_t column;
    for (column = 0U; column < clipped_width; ++column)
    {
      uint16_t pixel = row_pixels[column];
      TFT_WriteBusByte((uint8_t)(pixel >> 8));
      TFT_WriteBusByte((uint8_t)pixel);
    }
  }
  TFT_CS_High();
}

void TFT_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
  if ((x >= TFT_WIDTH) || (y >= TFT_HEIGHT))
  {
    return;
  }

  TFT_SetAddressWindow(x, y, x, y);
  TFT_WriteColorRepeat(color, 1U);
}

void TFT_FillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color)
{
  uint16_t x1;
  uint16_t y1;
  uint16_t clipped_width;
  uint16_t clipped_height;
  uint32_t pixel_count;

  if ((width == 0U) || (height == 0U))
  {
    return;
  }

  if ((x >= TFT_WIDTH) || (y >= TFT_HEIGHT))
  {
    return;
  }

  x1 = (uint16_t)(x + width - 1U);
  y1 = (uint16_t)(y + height - 1U);

  if (x1 >= TFT_WIDTH)
  {
    x1 = (uint16_t)(TFT_WIDTH - 1U);
  }
  if (y1 >= TFT_HEIGHT)
  {
    y1 = (uint16_t)(TFT_HEIGHT - 1U);
  }

  clipped_width = (uint16_t)(x1 - x + 1U);
  clipped_height = (uint16_t)(y1 - y + 1U);
  pixel_count = (uint32_t)clipped_width * clipped_height;

  TFT_SetAddressWindow(x, y, x1, y1);
  TFT_WriteColorRepeat(color, pixel_count);
}

void TFT_FillScreen(uint16_t color)
{
  TFT_FillRect(0U, 0U, TFT_WIDTH, TFT_HEIGHT, color);
}

void TFT_Init(void)
{
  TFT_BacklightEnsureReady();
  TFT_SetBacklight(0U);
  TFT_HardwareReset();
  TFT_ConfigureRegisters();
}
