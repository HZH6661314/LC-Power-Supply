/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : ui_display.c
  * @brief          : Dirty-flag based 240x240 UI drawing implementation.
  ******************************************************************************
  * @attention
  *
  * Author: LCYX
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#include "ui_display.h"

#include <stdio.h>
#include <string.h>

#include "state_machine.h"
#include "tft_gfx.h"
#include "tft_driver.h"  // For TFT_FillScreen
#include "tim.h"        // V10: 需要htim2用于光标闪烁
#include "bsp_hrtim.h"  // V10: 需要g_TIM2_Tick

#define UI_SCREEN_W             240
#define UI_SCREEN_H             240
#define UI_SCREEN_X             0
#define UI_SCREEN_Y             0

#define UI_TITLE_X              0
#define UI_TITLE_Y              0
#define UI_TITLE_W              240
#define UI_TITLE_H              32

#define UI_REAL_X               0
#define UI_REAL_Y               32
#define UI_REAL_W               138
#define UI_REAL_H               110

#define UI_SET_X                138
#define UI_SET_Y                32
#define UI_SET_W                102
#define UI_SET_H                110

#define UI_POWER_X              0
#define UI_POWER_Y              142
#define UI_POWER_W              138
#define UI_POWER_H              68

#define UI_FUNC_X               138
#define UI_FUNC_Y               142
#define UI_FUNC_W               102
#define UI_FUNC_H               68

#define UI_STATUS_X             0
#define UI_STATUS_Y             210
#define UI_STATUS_W             240
#define UI_STATUS_H             30

#define UI_PROGRESS_W           82
#define UI_PROGRESS_H           8
#define UI_PROGRESS_X           148
#define UI_PROGRESS_V_Y         42
#define UI_PROGRESS_I_Y         97

#define UI_COLOR_BG             TFT_COLOR_WHITE
#define UI_COLOR_FG             TFT_COLOR_BLACK
#define UI_COLOR_HL             TFT_COLOR_BLACK
#define UI_INVALID_FLOAT        (-99999.0f)
#define UI_INVALID_U8           (0xFFU)
#define UI_INVALID_U16          (0xFFFFU)

#define UI_BLINK_PERIOD_MS      500U
#define UI_BLINK_TICKS          10U

typedef struct {
    float vout;
    float iout;
    float power;
    float set_voltage;
    float set_current;
    UI_State_t ui_state;
    SM_Focus_t focus;
    uint8_t output_enabled;
    uint8_t cvcc_cc;
    uint16_t temperature_c;
    uint8_t initialized;
    uint8_t blink_visible;
    uint8_t system_status;
    uint8_t quick_set_cursor;       // Cache cursor position for dirty detection
    uint8_t quick_set_initialized;  // Track first entry to QUICK_SET state
    uint8_t quick_set_active;       // Cache active preset index
} UI_Cache_t;

static UI_Cache_t s_cache;

static void UI_ClearScreen(void);
static void UI_DrawStaticFrame(void);
static void UI_DrawTitle(void);
static void UI_DrawRealtime(float vout, float iout);
static void UI_DrawSettings(float voltage, float current);
static void UI_DrawPower(float power);
static void UI_DrawFunctions(void);
static void UI_DrawStatus(uint8_t output_enabled, uint8_t cc_mode, uint16_t temperature_c);
static void UI_DrawFocus(UI_State_t ui_state, SM_Focus_t focus);
static void UI_ClearFocus(void);
static void UI_RestoreSettingFrame(void);
static void UI_RestoreFunctionFrame(void);
static void UI_DrawProgressBar(int16_t x, int16_t y, float value, float max_value);
static void UI_DrawCenteredAscii(int16_t x, int16_t y, int16_t w, const char *text, uint16_t color, uint8_t scale);
static void UI_DrawValueText(int16_t x, int16_t y, int16_t width, const char *text, uint8_t scale);
static uint8_t UI_FloatChanged(float old_value, float new_value, float threshold);
static uint8_t UI_GetFocusRect(SM_Focus_t focus, int16_t *x, int16_t *y, int16_t *w, int16_t *h);
static void UI_DrawQuickSetMenu(void);
static void UI_DrawQuickSetItem(int16_t y, uint8_t index, uint8_t is_active, uint8_t has_cursor);

void UI_Display_Init(void)
{
    extern TIM_HandleTypeDef htim2;  // V10: TIM2用于光标闪烁时基

    memset(&s_cache, 0, sizeof(s_cache));
    s_cache.vout = UI_INVALID_FLOAT;
    s_cache.iout = UI_INVALID_FLOAT;
    s_cache.power = UI_INVALID_FLOAT;
    s_cache.set_voltage = UI_INVALID_FLOAT;
    s_cache.set_current = UI_INVALID_FLOAT;
    s_cache.ui_state = (UI_State_t)UI_INVALID_U8;
    s_cache.focus = (SM_Focus_t)UI_INVALID_U8;
    s_cache.output_enabled = UI_INVALID_U8;
    s_cache.cvcc_cc = UI_INVALID_U8;
    s_cache.temperature_c = UI_INVALID_U16;
    s_cache.initialized = 1U;
    s_cache.blink_visible = 1U;
    s_cache.system_status = UI_INVALID_U8;
    s_cache.quick_set_cursor = UI_INVALID_U8;
    s_cache.quick_set_initialized = 0U;
    s_cache.quick_set_active = UI_INVALID_U8;

    UI_DrawStaticFrame();

    // 初始化时绘制完整状态栏（包括静态的"normal"文本）
    UI_DrawStatus(0U, 0U, 25U);

    UI_Display_Process();

    // V10: UI完全初始化后再启动TIM2中断，避免访问未初始化变量
    HAL_TIM_Base_Start_IT(&htim2);
}

void UI_Display_Process(void)
{
    float vout;
    float iout;
    float power;
    float set_voltage;
    float set_current;
    UI_State_t ui_state;
    SM_Focus_t focus;
    uint8_t output_enabled;
    uint8_t cc_mode;
    uint16_t temperature_c;
    uint8_t blink_state_changed = 0U;

    // V10: 使用TIM2 tick实现光标闪烁（Product层实现业务逻辑）
    extern volatile uint32_t g_TIM2_Tick;
    static uint32_t s_last_blink_tick = 0;

    if (s_cache.initialized == 0U) {
        UI_Display_Init();
        return;
    }

    vout = SM_Get_MeasuredVoltage();
    iout = SM_Get_MeasuredCurrent();
    power = SM_Get_MeasuredPower();
    set_voltage = SM_Get_TargetVoltageFinal();
    set_current = SM_Get_CurrentLimit();
    ui_state = SM_Get_UI_State();

    // Handle Quick Set submenu (full-screen overlay)
    if (ui_state == UI_STATE_QUICK_SET) {
        UI_DrawQuickSetMenu();
        s_cache.ui_state = ui_state;
        return;  // Skip main screen rendering
    }

    // Detect transition from QUICK_SET back to main screen
    if ((s_cache.ui_state == UI_STATE_QUICK_SET) && (ui_state != UI_STATE_QUICK_SET)) {
        // Reset Quick Set state
        s_cache.quick_set_initialized = 0U;
        // Force full main screen redraw by invalidating cache
        s_cache.vout = UI_INVALID_FLOAT;
        s_cache.iout = UI_INVALID_FLOAT;
        s_cache.power = UI_INVALID_FLOAT;
        s_cache.set_voltage = UI_INVALID_FLOAT;
        s_cache.set_current = UI_INVALID_FLOAT;
        s_cache.output_enabled = UI_INVALID_U8;
        s_cache.cvcc_cc = UI_INVALID_U8;
        s_cache.temperature_c = UI_INVALID_U16;
        s_cache.system_status = UI_INVALID_U8;
        // Redraw static frame
        UI_DrawStaticFrame();
        UI_DrawStatus(0U, 0U, 25U);
    }

    focus = SM_Get_Focus();
    output_enabled = SM_Get_OutputEnabled();
    cc_mode = SM_Get_CCMode();
    temperature_c = SM_Get_TemperatureC();

    // V10: 光标闪烁逻辑（Product层）- 使用TIM2 tick，500ms周期
    // TIM2配置：1kHz (1ms per tick)，需要500 tick = 500ms
    if (ui_state == UI_STATE_HOME_EDIT) {
        uint32_t current_tick = g_TIM2_Tick;
        if ((current_tick - s_last_blink_tick) >= 500U) {  // 500ms
            s_last_blink_tick = current_tick;
            s_cache.blink_visible = (s_cache.blink_visible == 0U) ? 1U : 0U;
            blink_state_changed = 1U;
        }
    } else {
        if (s_cache.blink_visible == 0U) {
            s_cache.blink_visible = 1U;
            blink_state_changed = 1U;
        }
        s_last_blink_tick = g_TIM2_Tick;  // 重置计时器
    }

    if (UI_FloatChanged(s_cache.vout, vout, 0.01f) ||
        UI_FloatChanged(s_cache.iout, iout, 0.001f)) {
        UI_DrawRealtime(vout, iout);
        s_cache.vout = vout;
        s_cache.iout = iout;
    }

    if (UI_FloatChanged(s_cache.set_voltage, set_voltage, 0.05f) ||
        UI_FloatChanged(s_cache.set_current, set_current, 0.01f)) {
        UI_DrawSettings(set_voltage, set_current);
        s_cache.set_voltage = set_voltage;
        s_cache.set_current = set_current;
    }

    if (UI_FloatChanged(s_cache.power, power, 0.01f)) {
        UI_DrawPower(power);
        s_cache.power = power;
    }

    // Karpathy原则：独立脏检查，只重绘变化的文本，避免不必要的闪烁
    if (s_cache.output_enabled != output_enabled) {
        UI_DrawCenteredAscii(0, 210, 60, (output_enabled != 0U) ? "ON" : "OFF", UI_COLOR_FG, 2U);
        s_cache.output_enabled = output_enabled;
    }

    if (s_cache.cvcc_cc != cc_mode) {
        UI_DrawCenteredAscii(60, 210, 60, (cc_mode != 0U) ? "CC" : "CV", UI_COLOR_FG, 2U);
        s_cache.cvcc_cc = cc_mode;
    }

    if (s_cache.temperature_c != temperature_c) {
        char temp_text[8];
        (void)snprintf(temp_text, sizeof(temp_text), "%uC", (unsigned int)temperature_c);
        UI_DrawCenteredAscii(120, 210, 60, temp_text, UI_COLOR_FG, 2U);
        s_cache.temperature_c = temperature_c;
    }

    // V9新增：系统状态文本脏检查（第4格）
    {
        uint8_t system_status = 0U;

        if (s_cache.system_status != system_status) {
            const char *status_text = "normal";
            UI_DrawCenteredAscii(180, 210, 60, status_text, UI_COLOR_FG, 1U);
            s_cache.system_status = system_status;
        }
    }

    // 状态或焦点变化时重绘光标
    if ((s_cache.ui_state != ui_state) || (s_cache.focus != focus)) {
        UI_ClearFocus();
        UI_DrawFocus(ui_state, focus);
        s_cache.ui_state = ui_state;
        s_cache.focus = focus;
    } else if ((blink_state_changed != 0U) && (ui_state == UI_STATE_HOME_EDIT)) {
        // EDIT状态下闪烁切换：只重绘光标边框，不重绘内容（性能优化）
        int16_t x, y, w, h;
        if (UI_GetFocusRect(focus, &x, &y, &w, &h) != 0U) {
            if (s_cache.blink_visible != 0U) {
                // 显示：绘制三层边框
                TFTGFX_DrawRect(x, y, w, h, UI_COLOR_HL);
                TFTGFX_DrawRect((int16_t)(x + 1), (int16_t)(y + 1), (int16_t)(w - 2), (int16_t)(h - 2), UI_COLOR_HL);
                TFTGFX_DrawRect((int16_t)(x + 2), (int16_t)(y + 2), (int16_t)(w - 4), (int16_t)(h - 4), UI_COLOR_HL);
            } else {
                // 隐藏：用背景色覆盖边框
                TFTGFX_DrawRect(x, y, w, h, UI_COLOR_BG);
                TFTGFX_DrawRect((int16_t)(x + 1), (int16_t)(y + 1), (int16_t)(w - 2), (int16_t)(h - 2), UI_COLOR_BG);
                TFTGFX_DrawRect((int16_t)(x + 2), (int16_t)(y + 2), (int16_t)(w - 4), (int16_t)(h - 4), UI_COLOR_BG);
                // 恢复被覆盖的边框线
                if ((focus == SM_FOCUS_SET_VOLTAGE) || (focus == SM_FOCUS_SET_CURRENT)) {
                    UI_RestoreSettingFrame();
                }
            }
        }
    }
}

void UI_DrawChinese(int16_t x, int16_t y, const char *str, uint16_t color)
{
    uint16_t text_len = 0U;
    int16_t width;

    if (str != 0) {
        text_len = (uint16_t)strlen(str);
    }

    width = (int16_t)(text_len * 8U);
    if (width < 24) {
        width = 24;
    }

    TFTGFX_DrawRect(x, y, width, 16, color);
}

static void UI_DrawStaticFrame(void)
{
    UI_ClearScreen();

    TFTGFX_DrawRect(0, 0, UI_SCREEN_W, UI_SCREEN_H, UI_COLOR_FG);
    TFTGFX_DrawLine(0, 31, 239, 31, UI_COLOR_FG);
    TFTGFX_DrawLine(137, 32, 137, 209, UI_COLOR_FG);
    TFTGFX_DrawLine(0, 141, 239, 141, UI_COLOR_FG);
    TFTGFX_DrawLine(137, 209, 239, 209, UI_COLOR_FG);
    TFTGFX_DrawLine(0, 209, 239, 209, UI_COLOR_FG);
    TFTGFX_DrawLine(138, 86, 239, 86, UI_COLOR_FG);
    TFTGFX_DrawLine(138, 175, 239, 175, UI_COLOR_FG);
    TFTGFX_DrawLine(0, 210, 239, 210, UI_COLOR_FG);
    TFTGFX_DrawLine(59, 210, 59, 239, UI_COLOR_FG);
    TFTGFX_DrawLine(119, 210, 119, 239, UI_COLOR_FG);
    TFTGFX_DrawLine(179, 210, 179, 239, UI_COLOR_FG);

    UI_DrawTitle();
    UI_DrawFunctions();
}

static void UI_ClearScreen(void)
{
    TFTGFX_FillRect(UI_SCREEN_X, UI_SCREEN_Y, UI_SCREEN_W, UI_SCREEN_H, UI_COLOR_BG);
}

static void UI_DrawTitle(void)
{
    TFTGFX_FillRect(UI_TITLE_X + 1, UI_TITLE_Y + 1, UI_TITLE_W - 2, UI_TITLE_H - 2, UI_COLOR_BG);
    TFTGFX_DrawString(72, 9, "LC POWER", UI_COLOR_FG, 2U);
    UI_DrawChinese(14, 8, "TITLE", UI_COLOR_FG);
}

static void UI_DrawRealtime(float vout, float iout)
{
    char text[16];

    TFTGFX_DrawStringOpaque(8, 39, "VOUT", UI_COLOR_FG, UI_COLOR_BG, 1U);
    (void)snprintf(text, sizeof(text), "%05.2f", vout);
    UI_DrawValueText(8, 54, 90, text, 3U);
    TFTGFX_DrawStringOpaque(110, 60, "V", UI_COLOR_FG, UI_COLOR_BG, 2U);

    TFTGFX_DrawStringOpaque(8, 96, "IOUT", UI_COLOR_FG, UI_COLOR_BG, 1U);
    (void)snprintf(text, sizeof(text), "%05.3f", iout);
    UI_DrawValueText(8, 106, 90, text, 3U);
    TFTGFX_DrawStringOpaque(110, 112, "A", UI_COLOR_FG, UI_COLOR_BG, 2U);
}

static void UI_DrawSettings(float voltage, float current)
{
    char text[16];

    // Karpathy原则：只更新变化的内容，避免闪烁
    // 不填充背景，使用Opaque绘制覆盖旧内容

    // 绘制电压设置
    UI_DrawProgressBar(UI_PROGRESS_X, UI_PROGRESS_V_Y, voltage, SM_VOLTAGE_MAX);
    TFTGFX_DrawStringOpaque(144, 53, "SET V", UI_COLOR_FG, UI_COLOR_BG, 1U);
    (void)snprintf(text, sizeof(text), "%05.2f", voltage);
    UI_DrawValueText(148, 65, 90, text, 2U);  // 已使用Opaque绘制

    // 绘制电流设置
    UI_DrawProgressBar(UI_PROGRESS_X, UI_PROGRESS_I_Y, current, SM_CURRENT_MAX);
    TFTGFX_DrawStringOpaque(144, 108, "SET A", UI_COLOR_FG, UI_COLOR_BG, 1U);
    (void)snprintf(text, sizeof(text), "%04.2f", current);
    UI_DrawValueText(148, 120, 90, text, 2U);  // 已使用Opaque绘制

    // 重绘分隔线（防止被进度条覆盖）
    TFTGFX_DrawLine(138, 86, 239, 86, UI_COLOR_FG);
}

static void UI_DrawPower(float power)
{
    char text[16];

    // Karpathy原则：直接覆盖，无闪烁
    TFTGFX_DrawStringOpaque(8, 149, "POWER W", UI_COLOR_FG, UI_COLOR_BG, 1U);
    (void)snprintf(text, sizeof(text), "%05.1f", power);
    TFTGFX_DrawStringOpaque(10, 166, text, UI_COLOR_FG, UI_COLOR_BG, 4U);
}

static void UI_DrawFunctions(void)
{
    // Karpathy原则：直接覆盖，避免闪烁
    // 注意：此函数在StaticFrame中调用，仅绘制一次，理论上不会闪烁
    // 但为了代码一致性和未来维护，仍使用Opaque模式
    TFTGFX_DrawStringOpaque(146, 152, "QUICK", UI_COLOR_FG, UI_COLOR_BG, 2U);
    TFTGFX_DrawStringOpaque(146, 185, "SETUP", UI_COLOR_FG, UI_COLOR_BG, 2U);
    UI_DrawChinese(206, 151, "Q", UI_COLOR_FG);
    UI_DrawChinese(206, 184, "S", UI_COLOR_FG);
}

static void UI_DrawStatus(uint8_t output_enabled, uint8_t cc_mode, uint16_t temperature_c)
{
    char temp_text[8];

    // 绘制所有状态栏文本（仅在初始化时调用）
    UI_DrawCenteredAscii(0, 210, 60, (output_enabled != 0U) ? "ON" : "OFF", UI_COLOR_FG, 2U);
    UI_DrawCenteredAscii(60, 210, 60, (cc_mode != 0U) ? "CC" : "CV", UI_COLOR_FG, 2U);
    (void)snprintf(temp_text, sizeof(temp_text), "%uC", (unsigned int)temperature_c);
    UI_DrawCenteredAscii(120, 210, 60, temp_text, UI_COLOR_FG, 2U);
    UI_DrawCenteredAscii(180, 210, 60, "normal", UI_COLOR_FG, 1U);
}

static void UI_DrawFocus(UI_State_t ui_state, SM_Focus_t focus)
{
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;

    if (ui_state == UI_STATE_HOME_IDLE) {
        return;
    }

    if (ui_state == UI_STATE_QUICK_SET) {
        // Quick Set子菜单的光标采用反色背景样式
        // 光标绘制已集成在UI_DrawQuickSetMenu()中，此处无需额外操作
        return;
    }

    if (ui_state == UI_STATE_SYS_SET) {
        // TODO: 绘制系统设置子菜单的光标
        return;
    }

    if (UI_GetFocusRect(focus, &x, &y, &w, &h) == 0U) {
        return;
    }

    // 绘制外层双线框（MENU和EDIT状态都有）
    TFTGFX_DrawRect(x, y, w, h, UI_COLOR_HL);
    TFTGFX_DrawRect((int16_t)(x + 1), (int16_t)(y + 1), (int16_t)(w - 2), (int16_t)(h - 2), UI_COLOR_HL);

    // EDIT状态：绘制第三层框（闪烁效果由Process控制可见性）
    if (ui_state == UI_STATE_HOME_EDIT) {
        TFTGFX_DrawRect((int16_t)(x + 2), (int16_t)(y + 2), (int16_t)(w - 4), (int16_t)(h - 4), UI_COLOR_HL);
    }
}

static void UI_ClearFocus(void)
{
    int16_t x, y, w, h;

    // Karpathy原则：只清除光标本身，不重绘内容（性能优化）

    if (s_cache.ui_state == UI_STATE_HOME_IDLE) {
        return;
    }

    if (s_cache.ui_state == UI_STATE_QUICK_SET) {
        // Quick Set子菜单的光标清除已集成在UI_DrawQuickSetMenu()中
        // （通过重绘整行实现，无需单独清除操作）
        return;
    }

    if (s_cache.ui_state == UI_STATE_SYS_SET) {
        // TODO: 清除系统设置子菜单的光标
        return;
    }

    // 获取上次光标位置
    if (UI_GetFocusRect(s_cache.focus, &x, &y, &w, &h) == 0U) {
        return;
    }

    // 用背景色覆盖光标边框
    TFTGFX_DrawRect(x, y, w, h, UI_COLOR_BG);
    TFTGFX_DrawRect((int16_t)(x + 1), (int16_t)(y + 1), (int16_t)(w - 2), (int16_t)(h - 2), UI_COLOR_BG);

    // EDIT状态有第三层边框
    if (s_cache.ui_state == UI_STATE_HOME_EDIT) {
        TFTGFX_DrawRect((int16_t)(x + 2), (int16_t)(y + 2), (int16_t)(w - 4), (int16_t)(h - 4), UI_COLOR_BG);
    }

    // 恢复被覆盖的边框线
    if ((s_cache.focus == SM_FOCUS_SET_VOLTAGE) || (s_cache.focus == SM_FOCUS_SET_CURRENT)) {
        UI_RestoreSettingFrame();
    } else if ((s_cache.focus == SM_FOCUS_QUICK_SET) || (s_cache.focus == SM_FOCUS_SETTINGS)) {
        UI_RestoreFunctionFrame();
    }
}

static void UI_RestoreSettingFrame(void)
{
    TFTGFX_DrawLine(137, 32, 137, 141, UI_COLOR_FG);
    TFTGFX_DrawLine(239, 32, 239, 141, UI_COLOR_FG);
    TFTGFX_DrawLine(138, 31, 239, 31, UI_COLOR_FG);
    TFTGFX_DrawLine(138, 86, 239, 86, UI_COLOR_FG);
    TFTGFX_DrawLine(138, 141, 239, 141, UI_COLOR_FG);
}

static void UI_RestoreFunctionFrame(void)
{
    TFTGFX_DrawLine(137, 142, 137, 209, UI_COLOR_FG);
    TFTGFX_DrawLine(239, 142, 239, 209, UI_COLOR_FG);
    TFTGFX_DrawLine(138, 141, 239, 141, UI_COLOR_FG);
    TFTGFX_DrawLine(138, 175, 239, 175, UI_COLOR_FG);
    TFTGFX_DrawLine(138, 209, 239, 209, UI_COLOR_FG);
}

static void UI_DrawProgressBar(int16_t x, int16_t y, float value, float max_value)
{
    int16_t fill_w = 0;

    TFTGFX_DrawRect(x, y, UI_PROGRESS_W, UI_PROGRESS_H, UI_COLOR_FG);
    TFTGFX_FillRect((int16_t)(x + 1), (int16_t)(y + 1), UI_PROGRESS_W - 2, UI_PROGRESS_H - 2, UI_COLOR_BG);

    if (max_value > 0.001f) {
        fill_w = (int16_t)(((value / max_value) * (float)(UI_PROGRESS_W - 2)) + 0.5f);
    }

    if (fill_w < 0) {
        fill_w = 0;
    } else if (fill_w > (UI_PROGRESS_W - 2)) {
        fill_w = UI_PROGRESS_W - 2;
    }

    if (fill_w > 0) {
        TFTGFX_FillRect((int16_t)(x + 1), (int16_t)(y + 1), fill_w, UI_PROGRESS_H - 2, UI_COLOR_FG);
    }
}

static void UI_DrawCenteredAscii(int16_t x, int16_t y, int16_t w, const char *text, uint16_t color, uint8_t scale)
{
    char padded[16];
    int16_t text_h = (int16_t)(7U * scale);
    int16_t draw_y = (int16_t)(y + ((UI_STATUS_H - text_h) / 2));
    int16_t max_chars = w / (6 * scale);
    int16_t effective_chars = (max_chars > 1) ? (max_chars - 1) : 0;  // V10: 预留安全区，防止覆盖边框
    int16_t text_len = 0;
    int16_t pad_count = 0;
    int16_t idx = 0;
    int16_t i;
    const char *p;

    // Karpathy原则：固定宽度Opaque渲染，无需FillRect，无需恢复边框
    // V10: 使用effective_chars(max_chars-1)预留安全区，防止字符间隙覆盖边框线

    // 计算实际文本长度
    if (text != NULL) {
        p = text;
        while (*p != '\0' && text_len < effective_chars) {  // 使用effective_chars
            text_len++;
            p++;
        }
    }

    // 计算左右填充（实现居中）
    pad_count = (effective_chars - text_len) / 2;

    // 构造填充字符串：左空格 + 文本 + 右空格
    idx = 0;

    // 左侧空格
    for (i = 0; i < pad_count && idx < (int16_t)(sizeof(padded) - 1); i++) {
        padded[idx++] = ' ';
    }

    // 文本内容
    if (text != NULL) {
        p = text;
        while (*p != '\0' && idx < (int16_t)(sizeof(padded) - 1)) {
            padded[idx++] = *p++;
        }
    }

    // 右侧空格填充到固定宽度
    while (idx < effective_chars && idx < (int16_t)(sizeof(padded) - 1)) {
        padded[idx++] = ' ';
    }

    padded[idx] = '\0';

    // Opaque 模式左对齐绘制 → 完整覆盖无残留，无需恢复边框
    TFTGFX_DrawStringOpaque(x, draw_y, padded, color, UI_COLOR_BG, scale);
}

static void UI_DrawValueText(int16_t x, int16_t y, int16_t width, const char *text, uint8_t scale)
{
    uint16_t text_w = TFTGFX_MeasureStringWidth(text, scale);
    int16_t draw_x = x;

    if ((int16_t)text_w < width) {
        draw_x = (int16_t)(x + (width - (int16_t)text_w));
    }

    TFTGFX_DrawStringOpaque(draw_x, y, text, UI_COLOR_FG, UI_COLOR_BG, scale);
}

/**
 * @brief  绘制单个快速设置菜单项
 * @note   根据状态显示：箭头指示器（已应用）+ 反色光标（当前选中）
 * @param  y: 行的Y坐标
 * @param  index: 预设索引 (0-3)
 * @param  is_active: 是否为当前应用的预设（显示箭头）
 * @param  has_cursor: 是否有光标（反色显示）
 * @retval None
 */
static void UI_DrawQuickSetItem(int16_t y, uint8_t index, uint8_t is_active, uint8_t has_cursor)
{
    float voltage = 0.0f;
    float current = 0.0f;
    char text[32];
    int16_t x = 20;
    uint16_t fg_color;
    uint16_t bg_color;

    // 获取预设数据
    SM_Get_QuickSetPreset(index, &voltage, &current);

    // 反色显示：光标所在行使用反色
    if (has_cursor != 0U) {
        fg_color = UI_COLOR_BG;  // 白色文字
        bg_color = UI_COLOR_FG;  // 黑色背景
    } else {
        fg_color = UI_COLOR_FG;  // 黑色文字
        bg_color = UI_COLOR_BG;  // 白色背景
    }

    // 填充行背景（确保覆盖之前的内容）
    TFTGFX_FillRect((int16_t)(x - 5), y, 200, 14, bg_color);

    // 绘制箭头指示器（如果此预设已应用）
    if (is_active != 0U) {
        TFTGFX_DrawStringOpaque(x, y, ">", fg_color, bg_color, 2U);
    }

    // 绘制预设内容："N. XX.XXV  X.XXA"
    (void)snprintf(text, sizeof(text), "%u. %05.2fV  %04.2fA",
                   (unsigned int)(index + 1U), voltage, current);
    TFTGFX_DrawStringOpaque((int16_t)(x + 15), y, text, fg_color, bg_color, 2U);
}

/**
 * @brief  绘制完整的快速设置子菜单（全屏覆盖）
 * @note   优化策略：光标移动时只改变背景填充+覆盖文字，箭头独立更新
 * @retval None
 */
static void UI_DrawQuickSetMenu(void)
{
    uint8_t cursor;
    uint8_t active;
    uint8_t old_cursor;
    uint8_t old_active;
    int16_t y;
    uint8_t i;

    cursor = SM_Get_QuickSetCursor();
    active = SM_Get_ActivePresetIndex();

    // 首次进入QUICK_SET状态：清屏并绘制所有项目
    if (s_cache.quick_set_initialized == 0U) {
        TFT_FillScreen(UI_COLOR_BG);
        s_cache.quick_set_initialized = 1U;

        // 绘制全部4个预设项
        y = 80;
        for (i = 0; i < 4U; i++) {
            float voltage = 0.0f;
            float current = 0.0f;
            char text[32];
            int16_t x = 20;
            uint8_t has_cursor = (cursor == i);

            SM_Get_QuickSetPreset(i, &voltage, &current);

            // 如果有光标，填充反色背景
            if (has_cursor != 0U) {
                TFTGFX_FillRect((int16_t)(x - 5), y, 220, 14, UI_COLOR_FG);
            }

            // 绘制箭头（如果已激活）
            if (active == i) {
                uint16_t arrow_color = has_cursor ? UI_COLOR_BG : UI_COLOR_FG;
                TFTGFX_DrawString(x, y, ">", arrow_color, 2U);
            }

            // 绘制预设内容
            (void)snprintf(text, sizeof(text), "%u. %05.2fV  %04.2fA",
                           (unsigned int)(i + 1U), voltage, current);
            uint16_t text_color = has_cursor ? UI_COLOR_BG : UI_COLOR_FG;
            TFTGFX_DrawString((int16_t)(x + 15), y, text, text_color, 2U);

            y += 22;
        }

        s_cache.quick_set_cursor = cursor;
        s_cache.quick_set_active = active;
        return;
    }

    old_cursor = s_cache.quick_set_cursor;
    old_active = s_cache.quick_set_active;

    // 情况1：光标位置变化（UP/DOWN按键）
    // 策略：只重绘变化行，但使用Opaque模式一次性完成，避免闪烁
    if (old_cursor != cursor) {
        // 移除旧光标（重绘为正常样式：黑字白底）
        if (old_cursor < 4U) {
            y = (int16_t)(80 + old_cursor * 22);
            int16_t x = 20;
            float voltage = 0.0f;
            float current = 0.0f;
            char text[32];

            SM_Get_QuickSetPreset(old_cursor, &voltage, &current);

            // 填充白色背景（宽度220以显示完整文字包括"A"）
            TFTGFX_FillRect((int16_t)(x - 5), y, 220, 14, UI_COLOR_BG);

            // 如果有箭头，绘制箭头
            if (active == old_cursor) {
                TFTGFX_DrawString(x, y, ">", UI_COLOR_FG, 2U);
            }

            // 绘制文字（黑色）
            (void)snprintf(text, sizeof(text), "%u. %05.2fV  %04.2fA",
                           (unsigned int)(old_cursor + 1U), voltage, current);
            TFTGFX_DrawString((int16_t)(x + 15), y, text, UI_COLOR_FG, 2U);
        }

        // 添加新光标（重绘为反色样式：白字黑底）
        if (cursor < 4U) {
            y = (int16_t)(80 + cursor * 22);
            int16_t x = 20;
            float voltage = 0.0f;
            float current = 0.0f;
            char text[32];

            SM_Get_QuickSetPreset(cursor, &voltage, &current);

            // 填充黑色背景（宽度220以显示完整文字包括"A"）
            TFTGFX_FillRect((int16_t)(x - 5), y, 220, 14, UI_COLOR_FG);

            // 如果有箭头，绘制箭头（白色）
            if (active == cursor) {
                TFTGFX_DrawString(x, y, ">", UI_COLOR_BG, 2U);
            }

            // 绘制文字（白色）
            (void)snprintf(text, sizeof(text), "%u. %05.2fV  %04.2fA",
                           (unsigned int)(cursor + 1U), voltage, current);
            TFTGFX_DrawString((int16_t)(x + 15), y, text, UI_COLOR_BG, 2U);
        }

        s_cache.quick_set_cursor = cursor;
    }

    // 情况2：激活预设变化（SET按键应用新预设）
    if (old_active != active) {
        int16_t arrow_x = 20;
        int16_t arrow_w = 12;

        // 清除旧箭头
        if (old_active < 4U) {
            y = (int16_t)(80 + old_active * 22);
            uint8_t has_cursor = (cursor == old_active);

            if (has_cursor != 0U) {
                // 光标行：填充黑色背景
                TFTGFX_FillRect(arrow_x, y, arrow_w, 14, UI_COLOR_FG);
            } else {
                // 普通行：填充白色背景
                TFTGFX_FillRect(arrow_x, y, arrow_w, 14, UI_COLOR_BG);
            }
        }

        // 绘制新箭头
        if (active < 4U) {
            y = (int16_t)(80 + active * 22);
            uint8_t has_cursor = (cursor == active);

            if (has_cursor != 0U) {
                // 光标行：白色箭头
                TFTGFX_DrawString(arrow_x, y, ">", UI_COLOR_BG, 2U);
            } else {
                // 普通行：黑色箭头
                TFTGFX_DrawString(arrow_x, y, ">", UI_COLOR_FG, 2U);
            }
        }

        s_cache.quick_set_active = active;
    }
}

static uint8_t UI_FloatChanged(float old_value, float new_value, float threshold)
{
    float diff = old_value - new_value;

    if (diff < 0.0f) {
        diff = -diff;
    }

    return (diff >= threshold) ? 1U : 0U;
}

static uint8_t UI_GetFocusRect(SM_Focus_t focus, int16_t *x, int16_t *y, int16_t *w, int16_t *h)
{
    if ((x == 0) || (y == 0) || (w == 0) || (h == 0)) {
        return 0U;
    }

    switch (focus) {
    case SM_FOCUS_SET_VOLTAGE:
        *x = UI_SET_X;
        *y = UI_SET_Y;
        *w = UI_SET_W;
        *h = UI_SET_H / 2;
        return 1U;

    case SM_FOCUS_SET_CURRENT:
        *x = UI_SET_X;
        *y = (int16_t)(UI_SET_Y + (UI_SET_H / 2));
        *w = UI_SET_W;
        *h = UI_SET_H / 2;
        return 1U;

    case SM_FOCUS_QUICK_SET:
        *x = UI_FUNC_X;
        *y = UI_FUNC_Y;
        *w = UI_FUNC_W;
        *h = UI_FUNC_H / 2;
        return 1U;

    case SM_FOCUS_SETTINGS:
        *x = UI_FUNC_X;
        *y = (int16_t)(UI_FUNC_Y + (UI_FUNC_H / 2));
        *w = UI_FUNC_W;
        *h = UI_FUNC_H / 2;
        return 1U;

    default:
        break;
    }

    return 0U;
}
