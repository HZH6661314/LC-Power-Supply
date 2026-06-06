#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../Layer/Application/state_machine.h"

typedef struct {
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
    uint16_t color;
} fill_rect_call_t;

typedef struct {
    int16_t x;
    int16_t y;
    uint8_t scale;
    char text[16];
} draw_string_call_t;

#define MAX_FILL_CALLS 256
#define MAX_STRING_CALLS 128

static fill_rect_call_t s_fill_calls[MAX_FILL_CALLS];
static uint32_t s_fill_call_count = 0U;
static draw_string_call_t s_string_calls[MAX_STRING_CALLS];
static uint32_t s_string_call_count = 0U;

static float s_vout = 0.0f;
static float s_iout = 0.0f;
static float s_set_voltage = 0.0f;
static float s_set_current = 1.0f;
static uint8_t s_output_enabled = 0U;
static uint8_t s_cc_mode = 0U;
static uint16_t s_temp = 36U;
static UI_State_t s_ui_state = UI_STATE_STANDBY;
static SM_Focus_t s_focus = SM_FOCUS_SET_VOLTAGE;

void TFTGFX_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
    (void)x0; (void)y0; (void)x1; (void)y1; (void)color;
}

void TFTGFX_DrawRect(int16_t x, int16_t y, int16_t width, int16_t height, uint16_t color)
{
    (void)x; (void)y; (void)width; (void)height; (void)color;
}

void TFTGFX_FillRect(int16_t x, int16_t y, int16_t width, int16_t height, uint16_t color)
{
    if (s_fill_call_count < MAX_FILL_CALLS) {
        s_fill_calls[s_fill_call_count].x = x;
        s_fill_calls[s_fill_call_count].y = y;
        s_fill_calls[s_fill_call_count].w = width;
        s_fill_calls[s_fill_call_count].h = height;
        s_fill_calls[s_fill_call_count].color = color;
        ++s_fill_call_count;
    }
}

void TFTGFX_DrawCircle(int16_t cx, int16_t cy, int16_t radius, uint16_t color)
{
    (void)cx; (void)cy; (void)radius; (void)color;
}

void TFTGFX_FillCircle(int16_t cx, int16_t cy, int16_t radius, uint16_t color)
{
    (void)cx; (void)cy; (void)radius; (void)color;
}

void TFTGFX_DrawRing(int16_t cx, int16_t cy, int16_t outer_radius, int16_t inner_radius, uint16_t color)
{
    (void)cx; (void)cy; (void)outer_radius; (void)inner_radius; (void)color;
}

void TFTGFX_DrawArc(int16_t cx, int16_t cy, int16_t radius, int16_t start_deg, int16_t end_deg, uint16_t color)
{
    (void)cx; (void)cy; (void)radius; (void)start_deg; (void)end_deg; (void)color;
}

void TFTGFX_DrawArcThick(int16_t cx, int16_t cy, int16_t radius, int16_t thickness, int16_t start_deg, int16_t end_deg, uint16_t color)
{
    (void)cx; (void)cy; (void)radius; (void)thickness; (void)start_deg; (void)end_deg; (void)color;
}

void TFTGFX_DrawChar(int16_t x, int16_t y, char ch, uint16_t color, uint8_t scale)
{
    (void)x; (void)y; (void)ch; (void)color; (void)scale;
}

void TFTGFX_DrawString(int16_t x, int16_t y, const char *str, uint16_t color, uint8_t scale)
{
    (void)x; (void)y; (void)str; (void)color; (void)scale;
}

void TFTGFX_DrawCharOpaque(int16_t x, int16_t y, char ch, uint16_t fg_color, uint16_t bg_color, uint8_t scale)
{
    (void)x; (void)y; (void)ch; (void)fg_color; (void)bg_color; (void)scale;
}

void TFTGFX_DrawStringOpaque(int16_t x, int16_t y, const char *str, uint16_t fg_color, uint16_t bg_color, uint8_t scale)
{
    (void)fg_color; (void)bg_color;

    if (s_string_call_count < MAX_STRING_CALLS) {
        s_string_calls[s_string_call_count].x = x;
        s_string_calls[s_string_call_count].y = y;
        s_string_calls[s_string_call_count].scale = scale;
        strncpy(s_string_calls[s_string_call_count].text, str, sizeof(s_string_calls[s_string_call_count].text) - 1U);
        s_string_calls[s_string_call_count].text[sizeof(s_string_calls[s_string_call_count].text) - 1U] = '\0';
        ++s_string_call_count;
    }
}

uint16_t TFTGFX_MeasureStringWidth(const char *str, uint8_t scale)
{
    return (uint16_t)(strlen(str) * 6U * scale);
}

int16_t TFTGFX_SinDeg1000(int16_t degrees) { (void)degrees; return 0; }
int16_t TFTGFX_CosDeg1000(int16_t degrees) { (void)degrees; return 0; }
void TFTGFX_PolarToPoint(int16_t cx, int16_t cy, int16_t radius, int16_t degrees, int16_t *x_out, int16_t *y_out)
{
    (void)cx; (void)cy; (void)radius; (void)degrees;
    if (x_out != 0) { *x_out = 0; }
    if (y_out != 0) { *y_out = 0; }
}

float SM_Get_MeasuredVoltage(void) { return s_vout; }
float SM_Get_MeasuredCurrent(void) { return s_iout; }
float SM_Get_MeasuredPower(void) { return s_vout * s_iout; }
float SM_Get_TargetVoltageFinal(void) { return s_set_voltage; }
float SM_Get_CurrentLimit(void) { return s_set_current; }
UI_State_t SM_Get_UI_State(void) { return s_ui_state; }
SM_Focus_t SM_Get_Focus(void) { return s_focus; }
uint8_t SM_Get_OutputEnabled(void) { return s_output_enabled; }
uint8_t SM_Get_CCMode(void) { return s_cc_mode; }
uint16_t SM_Get_TemperatureC(void) { return s_temp; }

#include "../Layer/Product/ui_display.c"

static void reset_fill_calls(void)
{
    s_fill_call_count = 0U;
    memset(s_fill_calls, 0, sizeof(s_fill_calls));
    s_string_call_count = 0U;
    memset(s_string_calls, 0, sizeof(s_string_calls));
}

static void expect_no_large_realtime_clear(void)
{
    uint32_t i;

    for (i = 0U; i < s_fill_call_count; ++i) {
        if ((s_fill_calls[i].x == 2) &&
            (s_fill_calls[i].y == 34) &&
            (s_fill_calls[i].w == 133) &&
            (s_fill_calls[i].h == 105)) {
            fprintf(stderr, "realtime area still clears whole block\n");
            exit(1);
        }
    }
}

static const draw_string_call_t *find_string_call(const char *text)
{
    uint32_t i;

    for (i = 0U; i < s_string_call_count; ++i) {
        if (strcmp(s_string_calls[i].text, text) == 0) {
            return &s_string_calls[i];
        }
    }

    return 0;
}

static void expect_no_unit_overlap(void)
{
    const draw_string_call_t *v_value = find_string_call("12.34");
    const draw_string_call_t *v_unit = find_string_call("V");
    const draw_string_call_t *i_value = find_string_call("0.567");
    const draw_string_call_t *i_unit = find_string_call("A");

    if ((v_value == 0) || (v_unit == 0) || (i_value == 0) || (i_unit == 0)) {
        fprintf(stderr, "missing expected draw calls\n");
        exit(1);
    }

    if ((v_value->x + (int16_t)TFTGFX_MeasureStringWidth(v_value->text, v_value->scale)) >= v_unit->x) {
        fprintf(stderr, "voltage unit overlaps value\n");
        exit(1);
    }

    if ((i_value->x + (int16_t)TFTGFX_MeasureStringWidth(i_value->text, i_value->scale)) >= i_unit->x) {
        fprintf(stderr, "current unit overlaps value\n");
        exit(1);
    }
}

int main(void)
{
    UI_Display_Init();
    reset_fill_calls();

    s_vout = 12.34f;
    s_iout = 0.567f;
    UI_Display_Process();

    expect_no_large_realtime_clear();
    expect_no_unit_overlap();

    printf("ui_display regression ok\n");
    return 0;
}
