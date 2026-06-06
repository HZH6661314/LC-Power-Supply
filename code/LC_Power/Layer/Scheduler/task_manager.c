/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : task_manager.c
  * @brief          : Task manager implementation file.
  ******************************************************************************
  * @attention
  *
  * Author: LCYX
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#include "task_manager.h"
#include <stdint.h>

#include "key.h"
#include "tft_driver.h"
#include "state_machine.h"
#include "bsp_gpio.h"
#include "bsp_adc.h"
#include "bsp_lcd.h"
#include "bsp_hrtim.h"
#include "power_control.h"

static uint32_t s_LastTick_1ms = 0U;
static uint32_t s_LastTick_10ms = 0U;
static uint32_t s_LastTick_50ms = 0U;
static uint32_t s_LastTick_100ms = 0U;
static uint8_t s_TFT_Initialized = 0U;
static uint8_t s_UI_Initialized = 0U;

void Hardware_Init(void);
void BSP_Init(void);

void SysCore_Init(void)
{
    BSP_Init();
    Power_Control_Init();
    State_Machine_Init();
    Key_Init();
    Hardware_Init();
}

void SysCore_Run(void)
{
    volatile uint32_t currentTick = g_Ticks[TICK_MS].Tick();

    if (s_TFT_Initialized == 0U) {
        if (TFT_InitProcess() != 0U) {
            s_TFT_Initialized = 1U;
            TFT_SetBacklight(100U);
            UI_Display_Init();
            s_UI_Initialized = 1U;
        }
    }

    if ((uint32_t)(currentTick - s_LastTick_1ms) >= 1U) {
        s_LastTick_1ms = currentTick;
        StateMachine_Task();
    }

    if ((uint32_t)(currentTick - s_LastTick_10ms) >= 10U) {
        s_LastTick_10ms = currentTick;
        Key_Process();
    }

    if ((uint32_t)(currentTick - s_LastTick_50ms) >= 50U) {
        s_LastTick_50ms = currentTick;
        if (s_UI_Initialized != 0U) {
            UI_Display_Process();
        }
    }

    if ((uint32_t)(currentTick - s_LastTick_100ms) >= 100U) {
        s_LastTick_100ms = currentTick;
    }
}

void Hardware_Init(void)
{
    TFT_InitStart();
}

void BSP_Init(void)
{
    BSP_GPIO_Init();
    BSP_LCD_Init();
    BSP_ADC_Init();
    BSP_HRTIM_Init();

    s_LastTick_1ms = g_Ticks[TICK_MS].Tick();
    s_LastTick_10ms = g_Ticks[TICK_MS].Tick();
    s_LastTick_50ms = g_Ticks[TICK_MS].Tick();
    s_LastTick_100ms = g_Ticks[TICK_MS].Tick();
}
