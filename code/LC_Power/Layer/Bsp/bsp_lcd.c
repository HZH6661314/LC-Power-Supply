/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bsp_lcd.c
  * @brief          : BSP LCD硬件抽象层实现（STM32平台）
  ******************************************************************************
  * @attention
  *
  * 本文件是平台相关的实现，移植到其他MCU时需要修改
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "bsp_lcd.h"
#include "bsp_hrtim.h"  // 使用BSP层统一的时间接口
#include "main.h"
#include "tim.h"

/* Private variables ---------------------------------------------------------*/
static uint8_t s_backlight_ready = 0;
static uint8_t s_backlight_fault = 0;

/* LCD接口定义 */
static const lcd_interface_t s_lcd_interface = {
    .sck = { LCD_SCL_GPIO_Port, LCD_SCL_Pin },
    .sda = { LCD_SDA_GPIO_Port, LCD_SDA_Pin },
    .cs  = { LCD_CS_GPIO_Port,  LCD_CS_Pin  },
    .dc  = { LCD_DC_GPIO_Port,  LCD_DC_Pin  },
    .res = { LCD_RES_GPIO_Port, LCD_RES_Pin }
};

/* Private function prototypes -----------------------------------------------*/
static void LCD_Pin_Write(const lcd_pin_t *pin, uint8_t value);
static void LCD_SPI_WriteByte(uint8_t data);
static void LCD_SPI_WriteBuffer(const uint8_t *data, uint32_t len);
static uint32_t LCD_GetTickMS(void);

/* LCD操作集实现 */
static const struct lcd_ops s_lcd_ops = {
    .pin_write = LCD_Pin_Write,
    .spi_write_byte = LCD_SPI_WriteByte,
    .spi_write_buffer = LCD_SPI_WriteBuffer,
    .get_tick_ms = LCD_GetTickMS
};

/* Exported functions --------------------------------------------------------*/

void BSP_LCD_Init(void)
{
    // GPIO已经在MX_GPIO_Init中初始化，这里不需要额外操作
    s_backlight_ready = 0;
    s_backlight_fault = 0;
}

const lcd_interface_t* BSP_LCD_GetInterface(void)
{
    return &s_lcd_interface;
}

const struct lcd_ops* BSP_LCD_GetOps(void)
{
    return &s_lcd_ops;
}

void BSP_LCD_SetBacklight(uint8_t percent)
{
    uint32_t compare;

    if (percent > 100U) {
        percent = 100U;
    }

    // 确保PWM已就绪
    if (!s_backlight_ready) {
        HAL_TIM_PWM_Stop(&htim16, TIM_CHANNEL_1);

        htim16.Init.Prescaler = 0U;
        htim16.Init.Period = 3599U;
        htim16.Instance->PSC = 0U;
        htim16.Instance->ARR = 3599U;
        __HAL_TIM_SET_COMPARE(&htim16, TIM_CHANNEL_1, 0U);
        __HAL_TIM_SET_COUNTER(&htim16, 0U);
        htim16.Instance->EGR = TIM_EGR_UG;

        if (HAL_TIM_PWM_Start(&htim16, TIM_CHANNEL_1) != HAL_OK) {
            s_backlight_fault = 1U;
            return;
        }

        s_backlight_ready = 1U;
    }

    if (s_backlight_fault) {
        return;
    }

    compare = ((uint32_t)(htim16.Instance->ARR + 1U) * percent) / 100U;
    if (compare > htim16.Instance->ARR) {
        compare = htim16.Instance->ARR;
    }

    __HAL_TIM_SET_COMPARE(&htim16, TIM_CHANNEL_1, compare);
}

uint32_t BSP_LCD_GetTickMS(void)
{
    // 复用BSP层统一的时间接口
    return g_Ticks[TICK_MS].Tick();
}

/* Private functions ---------------------------------------------------------*/

static void LCD_Pin_Write(const lcd_pin_t *pin, uint8_t value)
{
    GPIO_TypeDef *port = (GPIO_TypeDef *)pin->port;

    if (value) {
        port->BSRR = pin->pin;  // 置位
    } else {
        port->BSRR = ((uint32_t)pin->pin << 16U);  // 复位
    }
}

static void LCD_SPI_WriteByte(uint8_t data)
{
    uint8_t mask;
    const lcd_interface_t *iface = &s_lcd_interface;

    for (mask = 0x80U; mask != 0U; mask >>= 1U) {
        // SCK拉低
        LCD_Pin_Write(&iface->sck, 0);

        // 设置数据线
        if (data & mask) {
            LCD_Pin_Write(&iface->sda, 1);
        } else {
            LCD_Pin_Write(&iface->sda, 0);
        }

        // SCK拉高（上升沿采样）
        LCD_Pin_Write(&iface->sck, 1);
    }

    // SCK拉低
    LCD_Pin_Write(&iface->sck, 0);
}

static void LCD_SPI_WriteBuffer(const uint8_t *data, uint32_t len)
{
    while (len--) {
        LCD_SPI_WriteByte(*data++);
    }
}

static uint32_t LCD_GetTickMS(void)
{
    // 复用BSP层统一的时间接口
    return g_Ticks[TICK_MS].Tick();
}
