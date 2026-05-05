/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : task_manager.c
  * @brief          : Task manager implementation file.
  *                   Handles scheduled module initialization and processing.
  ******************************************************************************
  * @attention
  *
  * Author: LCYX
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "task_manager.h"
#include <stdint.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "key.h"
#include "tft_driver.h"
#include "tft_dashboard.h"
#include "state_machine.h"
#include "bsp_lcd.h"
#include "bsp_hrtim.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static uint32_t s_LastTick_1ms = 0U;
static uint32_t s_LastTick_10ms = 0U;
static uint32_t s_LastTick_50ms = 0U;
static uint32_t s_LastTick_100ms = 0U;
static uint8_t s_TFT_Initialized = 0U;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Exported functions --------------------------------------------------------*/
void SysCore_Init()
{
    // 初始化BSP层
    BSP_Init();

	Hardware_Init();

    // 注意：TFT初始化会在SysCore_Run()中完成，不会阻塞这里
}

void SysCore_Run()
{
	uint32_t currentTick = g_Ticks[TICK_MS].Tick();

	// TFT初始化状态机（非阻塞）
	if (!s_TFT_Initialized) {
		if (TFT_InitProcess()) {
			s_TFT_Initialized = 1U;
			TFT_SetBacklight(100U);
			// TFT_Dashboard_Init();  // 初始化完成后再初始化UI
		}
	}

	if ((currentTick - s_LastTick_1ms) >= 1U) {      // 1ms通道处理最基本的任务，如PWM输出，ADC采样
        s_LastTick_1ms = currentTick;

        StateMachine_Task();
    }

    if ((currentTick - s_LastTick_10ms) >= 10U) {    // 10ms通道处理稍微慢点的通信
        s_LastTick_10ms = currentTick;

        Key_Process();
    }

    if ((currentTick - s_LastTick_50ms) >= 50U) {    // 50ms通道处理更新UI等慢速任务
        s_LastTick_50ms = currentTick;

        if (s_TFT_Initialized) {
            // TFT_Dashboard_Task(currentTick);
        }
    }

    if ((currentTick - s_LastTick_100ms) >= 100U) {  // 100ms通道处理非常慢的任务，如参数保存等
        s_LastTick_100ms = currentTick;


    }
}

/* Private functions ---------------------------------------------------------*/
void Hardware_Init()
{
	// 启动TFT非阻塞初始化
    TFT_InitStart();

}

void BSP_Init()
{
	BSP_GPIO_Init();
	
	// 初始化LCD BSP层
    BSP_LCD_Init();
	
    BSP_ADC_Init();
    
    BSP_HRTIM_Init();
	
	s_LastTick_1ms = g_Ticks[TICK_MS].Tick();
	s_LastTick_10ms = g_Ticks[TICK_MS].Tick();
	s_LastTick_50ms = g_Ticks[TICK_MS].Tick();
	s_LastTick_100ms = g_Ticks[TICK_MS].Tick();
}

/* USER CODE BEGIN 1 */
/* Additional static helper functions can be added here */
/* USER CODE END 1 */
