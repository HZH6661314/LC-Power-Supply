#include "tft_gfx.h"

static const uint8_t tft_gfx_font_5x7[95][5] = {
  { 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x5FU, 0x00, 0x00 }, { 0x00, 0x07U, 0x00, 0x07U, 0x00 },
  { 0x14U, 0x7FU, 0x14U, 0x7FU, 0x14U }, { 0x24U, 0x2AU, 0x7FU, 0x2AU, 0x12U }, { 0x23U, 0x13U, 0x08U, 0x64U, 0x62U },
  { 0x36U, 0x49U, 0x55U, 0x22U, 0x50U }, { 0x00, 0x05U, 0x03U, 0x00, 0x00 }, { 0x00, 0x1CU, 0x22U, 0x41U, 0x00 },
  { 0x00, 0x41U, 0x22U, 0x1CU, 0x00 }, { 0x14U, 0x08U, 0x3EU, 0x08U, 0x14U }, { 0x08U, 0x08U, 0x3EU, 0x08U, 0x08U },
  { 0x00, 0x50U, 0x30U, 0x00, 0x00 }, { 0x08U, 0x08U, 0x08U, 0x08U, 0x08U }, { 0x00, 0x60U, 0x60U, 0x00, 0x00 },
  { 0x20U, 0x10U, 0x08U, 0x04U, 0x02U }, { 0x3EU, 0x51U, 0x49U, 0x45U, 0x3EU }, { 0x00, 0x42U, 0x7FU, 0x40U, 0x00 },
  { 0x42U, 0x61U, 0x51U, 0x49U, 0x46U }, { 0x21U, 0x41U, 0x45U, 0x4BU, 0x31U }, { 0x18U, 0x14U, 0x12U, 0x7FU, 0x10U },
  { 0x27U, 0x45U, 0x45U, 0x45U, 0x39U }, { 0x3CU, 0x4AU, 0x49U, 0x49U, 0x30U }, { 0x01U, 0x71U, 0x09U, 0x05U, 0x03U },
  { 0x36U, 0x49U, 0x49U, 0x49U, 0x36U }, { 0x06U, 0x49U, 0x49U, 0x29U, 0x1EU }, { 0x00, 0x36U, 0x36U, 0x00, 0x00 },
  { 0x00, 0x56U, 0x36U, 0x00, 0x00 }, { 0x08U, 0x14U, 0x22U, 0x41U, 0x00 }, { 0x14U, 0x14U, 0x14U, 0x14U, 0x14U },
  { 0x00, 0x41U, 0x22U, 0x14U, 0x08U }, { 0x02U, 0x01U, 0x51U, 0x09U, 0x06U }, { 0x32U, 0x49U, 0x79U, 0x41U, 0x3EU },
  { 0x7EU, 0x11U, 0x11U, 0x11U, 0x7EU }, { 0x7FU, 0x49U, 0x49U, 0x49U, 0x36U }, { 0x3EU, 0x41U, 0x41U, 0x41U, 0x22U },
  { 0x7FU, 0x41U, 0x41U, 0x22U, 0x1CU }, { 0x7FU, 0x49U, 0x49U, 0x49U, 0x41U }, { 0x7FU, 0x09U, 0x09U, 0x09U, 0x01U },
  { 0x3EU, 0x41U, 0x49U, 0x49U, 0x7AU }, { 0x7FU, 0x08U, 0x08U, 0x08U, 0x7FU }, { 0x00, 0x41U, 0x7FU, 0x41U, 0x00 },
  { 0x20U, 0x40U, 0x41U, 0x3FU, 0x01U }, { 0x7FU, 0x08U, 0x14U, 0x22U, 0x41U }, { 0x7FU, 0x40U, 0x40U, 0x40U, 0x40U },
  { 0x7FU, 0x02U, 0x0CU, 0x02U, 0x7FU }, { 0x7FU, 0x04U, 0x08U, 0x10U, 0x7FU }, { 0x3EU, 0x41U, 0x41U, 0x41U, 0x3EU },
  { 0x7FU, 0x09U, 0x09U, 0x09U, 0x06U }, { 0x3EU, 0x41U, 0x51U, 0x21U, 0x5EU }, { 0x7FU, 0x09U, 0x19U, 0x29U, 0x46U },
  { 0x46U, 0x49U, 0x49U, 0x49U, 0x31U }, { 0x01U, 0x01U, 0x7FU, 0x01U, 0x01U }, { 0x3FU, 0x40U, 0x40U, 0x40U, 0x3FU },
  { 0x1FU, 0x20U, 0x40U, 0x20U, 0x1FU }, { 0x3FU, 0x40U, 0x38U, 0x40U, 0x3FU }, { 0x63U, 0x14U, 0x08U, 0x14U, 0x63U },
  { 0x07U, 0x08U, 0x70U, 0x08U, 0x07U }, { 0x61U, 0x51U, 0x49U, 0x45U, 0x43U }, { 0x00, 0x7FU, 0x41U, 0x41U, 0x00 },
  { 0x02U, 0x04U, 0x08U, 0x10U, 0x20U }, { 0x00, 0x41U, 0x41U, 0x7FU, 0x00 }, { 0x04U, 0x02U, 0x01U, 0x02U, 0x04U },
  { 0x40U, 0x40U, 0x40U, 0x40U, 0x40U }, { 0x00, 0x01U, 0x02U, 0x04U, 0x00 }, { 0x20U, 0x54U, 0x54U, 0x54U, 0x78U },
  { 0x7FU, 0x48U, 0x44U, 0x44U, 0x38U }, { 0x38U, 0x44U, 0x44U, 0x44U, 0x20U }, { 0x38U, 0x44U, 0x44U, 0x48U, 0x7FU },
  { 0x38U, 0x54U, 0x54U, 0x54U, 0x18U }, { 0x08U, 0x7EU, 0x09U, 0x01U, 0x02U }, { 0x0CU, 0x52U, 0x52U, 0x52U, 0x3EU },
  { 0x7FU, 0x08U, 0x04U, 0x04U, 0x78U }, { 0x00, 0x44U, 0x7DU, 0x40U, 0x00 }, { 0x20U, 0x40U, 0x44U, 0x3DU, 0x00 },
  { 0x7FU, 0x10U, 0x28U, 0x44U, 0x00 }, { 0x00, 0x41U, 0x7FU, 0x40U, 0x00 }, { 0x7CU, 0x04U, 0x18U, 0x04U, 0x78U },
  { 0x7CU, 0x08U, 0x04U, 0x04U, 0x78U }, { 0x38U, 0x44U, 0x44U, 0x44U, 0x38U }, { 0x7CU, 0x14U, 0x14U, 0x14U, 0x08U },
  { 0x08U, 0x14U, 0x14U, 0x18U, 0x7CU }, { 0x7CU, 0x08U, 0x04U, 0x04U, 0x08U }, { 0x48U, 0x54U, 0x54U, 0x54U, 0x20U },
  { 0x04U, 0x3FU, 0x44U, 0x40U, 0x20U }, { 0x3CU, 0x40U, 0x40U, 0x20U, 0x7CU }, { 0x1CU, 0x20U, 0x40U, 0x20U, 0x1CU },
  { 0x3CU, 0x40U, 0x30U, 0x40U, 0x3CU }, { 0x44U, 0x28U, 0x10U, 0x28U, 0x44U }, { 0x0CU, 0x50U, 0x50U, 0x50U, 0x3CU },
  { 0x44U, 0x64U, 0x54U, 0x4CU, 0x44U }, { 0x00, 0x08U, 0x36U, 0x41U, 0x00 }, { 0x00, 0x00, 0x7FU, 0x00, 0x00 },
  { 0x00, 0x41U, 0x36U, 0x08U, 0x00 }, { 0x10U, 0x08U, 0x08U, 0x10U, 0x08U }
};

static const int16_t tft_gfx_sin_table[91] = {
  0, 17, 35, 52, 70, 87, 105, 122, 139, 156, 174, 191, 208,
  225, 242, 259, 276, 292, 309, 326, 342, 358, 375, 391, 407, 423,
  438, 454, 469, 485, 500, 515, 530, 545, 559, 574, 588, 602, 616,
  629, 643, 656, 669, 682, 695, 707, 719, 731, 743, 755, 766, 777,
  788, 799, 809, 819, 829, 839, 848, 857, 866, 875, 883, 891, 899,
  906, 914, 921, 927, 934, 940, 946, 951, 956, 961, 966, 970, 974,
  978, 982, 985, 988, 990, 993, 995, 996, 998, 999, 999, 1000, 1000
};

static int16_t TFTGFX_Abs16(int16_t value);
static int16_t TFTGFX_NormalizeDegrees(int16_t degrees);
static void TFTGFX_DrawHLineClipped(int16_t x0, int16_t x1, int16_t y, uint16_t color);

static int16_t TFTGFX_Abs16(int16_t value)
{
  return (value < 0) ? (int16_t)(-value) : value;
}

static int16_t TFTGFX_NormalizeDegrees(int16_t degrees)
{
  while (degrees < 0)
  {
    degrees = (int16_t)(degrees + 360);
  }
  while (degrees >= 360)
  {
    degrees = (int16_t)(degrees - 360);
  }
  return degrees;
}

static void TFTGFX_DrawHLineClipped(int16_t x0, int16_t x1, int16_t y, uint16_t color)
{
  int16_t width;

  if ((y < 0) || (y >= (int16_t)TFT_HEIGHT))
  {
    return;
  }

  if (x0 > x1)
  {
    int16_t temp = x0;
    x0 = x1;
    x1 = temp;
  }

  if ((x1 < 0) || (x0 >= (int16_t)TFT_WIDTH))
  {
    return;
  }

  if (x0 < 0)
  {
    x0 = 0;
  }
  if (x1 >= (int16_t)TFT_WIDTH)
  {
    x1 = (int16_t)(TFT_WIDTH - 1);
  }

  width = (int16_t)(x1 - x0 + 1);
  if (width > 0)
  {
    TFT_FillRect((uint16_t)x0, (uint16_t)y, (uint16_t)width, 1U, color);
  }
}

int16_t TFTGFX_SinDeg1000(int16_t degrees)
{
  degrees = TFTGFX_NormalizeDegrees(degrees);

  if (degrees <= 90)
  {
    return tft_gfx_sin_table[degrees];
  }
  if (degrees <= 180)
  {
    return tft_gfx_sin_table[180 - degrees];
  }
  if (degrees <= 270)
  {
    return (int16_t)(-tft_gfx_sin_table[degrees - 180]);
  }
  return (int16_t)(-tft_gfx_sin_table[360 - degrees]);
}

int16_t TFTGFX_CosDeg1000(int16_t degrees)
{
  return TFTGFX_SinDeg1000((int16_t)(90 - degrees));
}

void TFTGFX_PolarToPoint(int16_t cx,
                         int16_t cy,
                         int16_t radius,
                         int16_t degrees,
                         int16_t *x_out,
                         int16_t *y_out)
{
  int32_t sin_v = TFTGFX_SinDeg1000(degrees);
  int32_t cos_v = TFTGFX_CosDeg1000(degrees);

  if (x_out != 0)
  {
    *x_out = (int16_t)(cx + (int16_t)((sin_v * radius) / 1000));
  }
  if (y_out != 0)
  {
    *y_out = (int16_t)(cy - (int16_t)((cos_v * radius) / 1000));
  }
}

void TFTGFX_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
  int16_t dx = TFTGFX_Abs16((int16_t)(x1 - x0));
  int16_t sx = (x0 < x1) ? 1 : -1;
  int16_t dy = (int16_t)(-TFTGFX_Abs16((int16_t)(y1 - y0)));
  int16_t sy = (y0 < y1) ? 1 : -1;
  int16_t err = (int16_t)(dx + dy);

  if (y0 == y1)
  {
    TFTGFX_DrawHLineClipped(x0, x1, y0, color);
    return;
  }

  if (x0 == x1)
  {
    int16_t y_start = y0;
    int16_t y_end = y1;

    if (y_start > y_end)
    {
      int16_t temp = y_start;
      y_start = y_end;
      y_end = temp;
    }

    if ((x0 >= 0) && (x0 < (int16_t)TFT_WIDTH))
    {
      if (y_start < 0)
      {
        y_start = 0;
      }
      if (y_end >= (int16_t)TFT_HEIGHT)
      {
        y_end = (int16_t)(TFT_HEIGHT - 1);
      }
      if (y_start <= y_end)
      {
        TFT_FillRect((uint16_t)x0,
                     (uint16_t)y_start,
                     1U,
                     (uint16_t)(y_end - y_start + 1),
                     color);
      }
    }
    return;
  }

  while (1)
  {
    if ((x0 >= 0) && (y0 >= 0) && (x0 < (int16_t)TFT_WIDTH) && (y0 < (int16_t)TFT_HEIGHT))
    {
      TFT_DrawPixel((uint16_t)x0, (uint16_t)y0, color);
    }

    if ((x0 == x1) && (y0 == y1))
    {
      break;
    }

    if ((int16_t)(2 * err) >= dy)
    {
      err = (int16_t)(err + dy);
      x0 = (int16_t)(x0 + sx);
    }
    if ((int16_t)(2 * err) <= dx)
    {
      err = (int16_t)(err + dx);
      y0 = (int16_t)(y0 + sy);
    }
  }
}

void TFTGFX_DrawRect(int16_t x, int16_t y, int16_t width, int16_t height, uint16_t color)
{
  if ((width <= 0) || (height <= 0))
  {
    return;
  }

  TFTGFX_DrawLine(x, y, (int16_t)(x + width - 1), y, color);
  TFTGFX_DrawLine(x, (int16_t)(y + height - 1), (int16_t)(x + width - 1), (int16_t)(y + height - 1), color);
  TFTGFX_DrawLine(x, y, x, (int16_t)(y + height - 1), color);
  TFTGFX_DrawLine((int16_t)(x + width - 1), y, (int16_t)(x + width - 1), (int16_t)(y + height - 1), color);
}

void TFTGFX_FillRect(int16_t x, int16_t y, int16_t width, int16_t height, uint16_t color)
{
  if ((width <= 0) || (height <= 0))
  {
    return;
  }
  if ((x < 0) || (y < 0))
  {
    return;
  }

  TFT_FillRect((uint16_t)x, (uint16_t)y, (uint16_t)width, (uint16_t)height, color);
}

void TFTGFX_DrawCircle(int16_t cx, int16_t cy, int16_t radius, uint16_t color)
{
  int16_t x = radius;
  int16_t y = 0;
  int16_t err = 0;

  if (radius < 0)
  {
    return;
  }

  while (x >= y)
  {
    if (((cx + x) >= 0) && ((cy + y) >= 0) && ((cx + x) < (int16_t)TFT_WIDTH) && ((cy + y) < (int16_t)TFT_HEIGHT))
    {
      TFT_DrawPixel((uint16_t)(cx + x), (uint16_t)(cy + y), color);
    }
    if (((cx + y) >= 0) && ((cy + x) >= 0) && ((cx + y) < (int16_t)TFT_WIDTH) && ((cy + x) < (int16_t)TFT_HEIGHT))
    {
      TFT_DrawPixel((uint16_t)(cx + y), (uint16_t)(cy + x), color);
    }
    if (((cx - y) >= 0) && ((cy + x) >= 0) && ((cx - y) < (int16_t)TFT_WIDTH) && ((cy + x) < (int16_t)TFT_HEIGHT))
    {
      TFT_DrawPixel((uint16_t)(cx - y), (uint16_t)(cy + x), color);
    }
    if (((cx - x) >= 0) && ((cy + y) >= 0) && ((cx - x) < (int16_t)TFT_WIDTH) && ((cy + y) < (int16_t)TFT_HEIGHT))
    {
      TFT_DrawPixel((uint16_t)(cx - x), (uint16_t)(cy + y), color);
    }
    if (((cx - x) >= 0) && ((cy - y) >= 0) && ((cx - x) < (int16_t)TFT_WIDTH) && ((cy - y) < (int16_t)TFT_HEIGHT))
    {
      TFT_DrawPixel((uint16_t)(cx - x), (uint16_t)(cy - y), color);
    }
    if (((cx - y) >= 0) && ((cy - x) >= 0) && ((cx - y) < (int16_t)TFT_WIDTH) && ((cy - x) < (int16_t)TFT_HEIGHT))
    {
      TFT_DrawPixel((uint16_t)(cx - y), (uint16_t)(cy - x), color);
    }
    if (((cx + y) >= 0) && ((cy - x) >= 0) && ((cx + y) < (int16_t)TFT_WIDTH) && ((cy - x) < (int16_t)TFT_HEIGHT))
    {
      TFT_DrawPixel((uint16_t)(cx + y), (uint16_t)(cy - x), color);
    }
    if (((cx + x) >= 0) && ((cy - y) >= 0) && ((cx + x) < (int16_t)TFT_WIDTH) && ((cy - y) < (int16_t)TFT_HEIGHT))
    {
      TFT_DrawPixel((uint16_t)(cx + x), (uint16_t)(cy - y), color);
    }

    if (err <= 0)
    {
      y = (int16_t)(y + 1);
      err = (int16_t)(err + (2 * y) + 1);
    }
    if (err > 0)
    {
      x = (int16_t)(x - 1);
      err = (int16_t)(err - (2 * x) - 1);
    }
  }
}

void TFTGFX_FillCircle(int16_t cx, int16_t cy, int16_t radius, uint16_t color)
{
  int16_t x = radius;
  int16_t y = 0;
  int16_t err = 0;

  if (radius < 0)
  {
    return;
  }

  while (x >= y)
  {
    TFTGFX_DrawLine((int16_t)(cx - x), (int16_t)(cy + y), (int16_t)(cx + x), (int16_t)(cy + y), color);
    TFTGFX_DrawLine((int16_t)(cx - y), (int16_t)(cy + x), (int16_t)(cx + y), (int16_t)(cy + x), color);
    TFTGFX_DrawLine((int16_t)(cx - x), (int16_t)(cy - y), (int16_t)(cx + x), (int16_t)(cy - y), color);
    TFTGFX_DrawLine((int16_t)(cx - y), (int16_t)(cy - x), (int16_t)(cx + y), (int16_t)(cy - x), color);

    if (err <= 0)
    {
      y = (int16_t)(y + 1);
      err = (int16_t)(err + (2 * y) + 1);
    }
    if (err > 0)
    {
      x = (int16_t)(x - 1);
      err = (int16_t)(err - (2 * x) - 1);
    }
  }
}

void TFTGFX_DrawRing(int16_t cx,
                     int16_t cy,
                     int16_t outer_radius,
                     int16_t inner_radius,
                     uint16_t color)
{
  int16_t y;
  int32_t outer_sq;
  int32_t inner_sq;

  if ((outer_radius <= 0) || (inner_radius < 0) || (inner_radius >= outer_radius))
  {
    return;
  }

  outer_sq = (int32_t)outer_radius * outer_radius;
  inner_sq = (int32_t)inner_radius * inner_radius;

  for (y = (int16_t)(-outer_radius); y <= outer_radius; ++y)
  {
    int32_t y_sq = (int32_t)y * y;
    int16_t x_outer = outer_radius;
    int16_t x_inner = -1;

    while ((((int32_t)x_outer * x_outer) + y_sq) > outer_sq)
    {
      --x_outer;
    }

    if (inner_radius > 0)
    {
      x_inner = inner_radius;
      while ((x_inner >= 0) && ((((int32_t)x_inner * x_inner) + y_sq) >= inner_sq))
      {
        --x_inner;
      }
    }

    if (x_outer >= 0)
    {
      if (x_inner >= 0)
      {
        TFTGFX_DrawHLineClipped((int16_t)(cx - x_outer), (int16_t)(cx - x_inner - 1), (int16_t)(cy + y), color);
        TFTGFX_DrawHLineClipped((int16_t)(cx + x_inner + 1), (int16_t)(cx + x_outer), (int16_t)(cy + y), color);
      }
      else
      {
        TFTGFX_DrawHLineClipped((int16_t)(cx - x_outer), (int16_t)(cx + x_outer), (int16_t)(cy + y), color);
      }
    }
  }
}

void TFTGFX_DrawArc(int16_t cx,
                    int16_t cy,
                    int16_t radius,
                    int16_t start_deg,
                    int16_t end_deg,
                    uint16_t color)
{
  int16_t angle;
  int16_t prev_x;
  int16_t prev_y;
  int16_t curr_x;
  int16_t curr_y;

  if (radius < 0)
  {
    return;
  }

  if (end_deg < start_deg)
  {
    int16_t temp = start_deg;
    start_deg = end_deg;
    end_deg = temp;
  }

  TFTGFX_PolarToPoint(cx, cy, radius, start_deg, &prev_x, &prev_y);
  for (angle = (int16_t)(start_deg + 1); angle <= end_deg; ++angle)
  {
    TFTGFX_PolarToPoint(cx, cy, radius, angle, &curr_x, &curr_y);
    TFTGFX_DrawLine(prev_x, prev_y, curr_x, curr_y, color);
    prev_x = curr_x;
    prev_y = curr_y;
  }
}

void TFTGFX_DrawArcThick(int16_t cx,
                         int16_t cy,
                         int16_t radius,
                         int16_t thickness,
                         int16_t start_deg,
                         int16_t end_deg,
                         uint16_t color)
{
  int16_t inner_radius;
  int16_t outer_radius;
  int16_t current_radius;

  if ((radius <= 0) || (thickness <= 0))
  {
    return;
  }

  inner_radius = (int16_t)(radius - (thickness / 2));
  if (inner_radius < 0)
  {
    inner_radius = 0;
  }
  outer_radius = (int16_t)(inner_radius + thickness - 1);

  for (current_radius = inner_radius; current_radius <= outer_radius; ++current_radius)
  {
    TFTGFX_DrawArc(cx, cy, current_radius, start_deg, end_deg, color);
  }
}

void TFTGFX_DrawChar(int16_t x, int16_t y, char ch, uint16_t color, uint8_t scale)
{
  uint8_t column;

  if (scale == 0U)
  {
    scale = 1U;
  }

  if ((ch < 32) || (ch > 126))
  {
    ch = '?';
  }

  for (column = 0U; column < 5U; ++column)
  {
    uint8_t row_bits = tft_gfx_font_5x7[(uint8_t)(ch - 32)][column];
    uint8_t row;

    for (row = 0U; row < 7U; ++row)
    {
      if ((row_bits & (1U << row)) != 0U)
      {
        if (scale == 1U)
        {
          if (((int16_t)(x + column) >= 0) && ((int16_t)(y + row) >= 0))
          {
            TFT_DrawPixel((uint16_t)(x + column), (uint16_t)(y + row), color);
          }
        }
        else
        {
          TFTGFX_FillRect((int16_t)(x + ((int16_t)column * scale)),
                          (int16_t)(y + ((int16_t)row * scale)),
                          scale,
                          scale,
                          color);
        }
      }
    }
  }
}

void TFTGFX_DrawCharOpaque(int16_t x, int16_t y, char ch, uint16_t fg_color, uint16_t bg_color, uint8_t scale)
{
  uint8_t column;

  if (scale == 0U)
  {
    scale = 1U;
  }

  if ((ch < 32) || (ch > 126))
  {
    ch = '?';
  }

  for (column = 0U; column < 5U; ++column)
  {
    uint8_t row_bits = tft_gfx_font_5x7[(uint8_t)(ch - 32)][column];
    uint8_t row;

    for (row = 0U; row < 7U; ++row)
    {
      uint16_t pixel_color = ((row_bits & (1U << row)) != 0U) ? fg_color : bg_color;

      if (scale == 1U)
      {
        if (((int16_t)(x + column) >= 0) && ((int16_t)(y + row) >= 0))
        {
          TFT_DrawPixel((uint16_t)(x + column), (uint16_t)(y + row), pixel_color);
        }
      }
      else
      {
        TFTGFX_FillRect((int16_t)(x + ((int16_t)column * scale)),
                        (int16_t)(y + ((int16_t)row * scale)),
                        scale,
                        scale,
                        pixel_color);
      }
    }
  }

  if (scale == 1U)
  {
    TFTGFX_FillRect((int16_t)(x + 5), y, 1, 7, bg_color);
  }
  else
  {
    TFTGFX_FillRect((int16_t)(x + (5 * scale)), y, scale, (int16_t)(7 * scale), bg_color);
  }
}

void TFTGFX_DrawString(int16_t x, int16_t y, const char *str, uint16_t color, uint8_t scale)
{
  if (str == 0)
  {
    return;
  }

  if (scale == 0U)
  {
    scale = 1U;
  }

  while (*str != '\0')
  {
    TFTGFX_DrawChar(x, y, *str, color, scale);
    x = (int16_t)(x + (6 * scale));
    ++str;
  }
}

void TFTGFX_DrawStringOpaque(int16_t x, int16_t y, const char *str, uint16_t fg_color, uint16_t bg_color, uint8_t scale)
{
  if (str == 0)
  {
    return;
  }

  if (scale == 0U)
  {
    scale = 1U;
  }

  while (*str != '\0')
  {
    TFTGFX_DrawCharOpaque(x, y, *str, fg_color, bg_color, scale);
    x = (int16_t)(x + (6 * scale));
    ++str;
  }
}

uint16_t TFTGFX_MeasureStringWidth(const char *str, uint8_t scale)
{
  uint16_t length = 0U;

  if (str == 0)
  {
    return 0U;
  }

  if (scale == 0U)
  {
    scale = 1U;
  }

  while (*str != '\0')
  {
    ++length;
    ++str;
  }

  return (uint16_t)(length * 6U * scale);
}
