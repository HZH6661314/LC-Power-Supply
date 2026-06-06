/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bsp_lcd.h
  * @brief          : BSP LCD hardware abstraction interface.
  ******************************************************************************
  * @attention
  *
  * Author: LCYX
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __BSP_LCD_H
#define __BSP_LCD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct {
    void *port;
    uint16_t pin;
} lcd_pin_t;

typedef struct {
    lcd_pin_t sck;
    lcd_pin_t sda;
    lcd_pin_t cs;
    lcd_pin_t dc;
    lcd_pin_t res;
} lcd_interface_t;

struct lcd_ops {
    void (*pin_write)(const lcd_pin_t *pin, uint8_t value);
    void (*spi_write_byte)(uint8_t data);
    void (*spi_write_buffer)(const uint8_t *data, uint32_t len);
    uint32_t (*get_tick_ms)(void);
};

void BSP_LCD_Init(void);
const lcd_interface_t* BSP_LCD_GetInterface(void);
const struct lcd_ops* BSP_LCD_GetOps(void);
void BSP_LCD_SetBacklight(uint8_t percent);
uint32_t BSP_LCD_GetTickMS(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_LCD_H */
