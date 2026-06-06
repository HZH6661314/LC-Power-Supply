#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../Layer/Driver/tft_driver.h"
#include "../Layer/Bsp/bsp_lcd.h"

typedef struct {
    uint8_t type;
    uint8_t value;
} trace_entry_t;

#define TRACE_MAX 512

static trace_entry_t s_trace[TRACE_MAX];
static uint32_t s_trace_count = 0U;
static uint32_t s_fake_tick = 0U;
static lcd_interface_t s_iface;

static void trace_push(uint8_t type, uint8_t value)
{
    if (s_trace_count < TRACE_MAX) {
        s_trace[s_trace_count].type = type;
        s_trace[s_trace_count].value = value;
        ++s_trace_count;
    }
}

static void fake_pin_write(const lcd_pin_t *pin, uint8_t value)
{
    (void)pin;
    trace_push(0xF0U, value);
}

static void fake_spi_write_byte(uint8_t data)
{
    trace_push(0xA0U, data);
}

static void fake_spi_write_buffer(const uint8_t *data, uint32_t len)
{
    while (len-- > 0U) {
        fake_spi_write_byte(*data++);
    }
}

static uint32_t fake_get_tick_ms(void)
{
    return s_fake_tick;
}

static const struct lcd_ops s_ops = {
    .pin_write = fake_pin_write,
    .spi_write_byte = fake_spi_write_byte,
    .spi_write_buffer = fake_spi_write_buffer,
    .get_tick_ms = fake_get_tick_ms
};

const lcd_interface_t* BSP_LCD_GetInterface(void)
{
    return &s_iface;
}

const struct lcd_ops* BSP_LCD_GetOps(void)
{
    return &s_ops;
}

void BSP_LCD_Init(void) {}
void BSP_LCD_SetBacklight(uint8_t percent) { (void)percent; }
uint32_t BSP_LCD_GetTickMS(void) { return s_fake_tick; }

static void expect_trace_contains(uint8_t value, const char *label)
{
    uint32_t i;

    for (i = 0U; i < s_trace_count; ++i) {
        if ((s_trace[i].type == 0xA0U) && (s_trace[i].value == value)) {
            return;
        }
    }

    fprintf(stderr, "missing %s (0x%02X)\n", label, value);
    exit(1);
}

static void expect_panel_config(void)
{
    const TFT_PanelConfig_t *cfg = TFT_GetPanelConfig();

    if (cfg->madctl != 0x00U) {
        fprintf(stderr, "unexpected madctl: 0x%02X\n", cfg->madctl);
        exit(1);
    }
    if ((cfg->x_offset != 0U) || (cfg->y_offset != 0U)) {
        fprintf(stderr, "unexpected offsets: %u,%u\n", cfg->x_offset, cfg->y_offset);
        exit(1);
    }
}

int main(void)
{
    expect_panel_config();

    TFT_InitStart();
    if (TFT_InitProcess() != 0U) {
        fprintf(stderr, "init completed too early\n");
        return 1;
    }

    s_fake_tick = 10U;
    (void)TFT_InitProcess();
    s_fake_tick = 15U;
    (void)TFT_InitProcess();
    s_fake_tick = 135U;
    (void)TFT_InitProcess();
    s_fake_tick = 155U;
    if (TFT_InitProcess() != 1U) {
        fprintf(stderr, "init did not complete\n");
        return 1;
    }

    expect_trace_contains(0x11U, "SLPOUT");
    expect_trace_contains(0x20U, "INVOFF");
    expect_trace_contains(0xB7U, "B7");
    expect_trace_contains(0x56U, "B7 data");
    expect_trace_contains(0xBBU, "BB");
    expect_trace_contains(0x18U, "BB data");
    expect_trace_contains(0xC3U, "C3");
    expect_trace_contains(0x1FU, "C3 data");
    expect_trace_contains(0xD0U, "D0");
    expect_trace_contains(0xA6U, "D0 data 1");
    expect_trace_contains(0x21U, "INVON");
    expect_trace_contains(0x29U, "DISPON");
    expect_trace_contains(0x36U, "MADCTL");
    expect_trace_contains(0x00U, "MADCTL data");

    printf("tft_driver regression ok\n");
    return 0;
}
