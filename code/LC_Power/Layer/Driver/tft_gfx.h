#ifndef TFT_GFX_H
#define TFT_GFX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "tft_driver.h"

void TFTGFX_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void TFTGFX_DrawRect(int16_t x, int16_t y, int16_t width, int16_t height, uint16_t color);
void TFTGFX_FillRect(int16_t x, int16_t y, int16_t width, int16_t height, uint16_t color);
void TFTGFX_DrawCircle(int16_t cx, int16_t cy, int16_t radius, uint16_t color);
void TFTGFX_FillCircle(int16_t cx, int16_t cy, int16_t radius, uint16_t color);
void TFTGFX_DrawRing(int16_t cx,
                     int16_t cy,
                     int16_t outer_radius,
                     int16_t inner_radius,
                     uint16_t color);
void TFTGFX_DrawArc(int16_t cx,
                    int16_t cy,
                    int16_t radius,
                    int16_t start_deg,
                    int16_t end_deg,
                    uint16_t color);
void TFTGFX_DrawArcThick(int16_t cx,
                         int16_t cy,
                         int16_t radius,
                         int16_t thickness,
                         int16_t start_deg,
                         int16_t end_deg,
                         uint16_t color);
void TFTGFX_DrawChar(int16_t x, int16_t y, char ch, uint16_t color, uint8_t scale);
void TFTGFX_DrawCharOpaque(int16_t x, int16_t y, char ch, uint16_t fg_color, uint16_t bg_color, uint8_t scale);
void TFTGFX_DrawString(int16_t x, int16_t y, const char *str, uint16_t color, uint8_t scale);
void TFTGFX_DrawStringOpaque(int16_t x, int16_t y, const char *str, uint16_t fg_color, uint16_t bg_color, uint8_t scale);
uint16_t TFTGFX_MeasureStringWidth(const char *str, uint8_t scale);
int16_t TFTGFX_SinDeg1000(int16_t degrees);
int16_t TFTGFX_CosDeg1000(int16_t degrees);
void TFTGFX_PolarToPoint(int16_t cx,
                         int16_t cy,
                         int16_t radius,
                         int16_t degrees,
                         int16_t *x_out,
                         int16_t *y_out);

#ifdef __cplusplus
}
#endif

#endif /* TFT_GFX_H */
