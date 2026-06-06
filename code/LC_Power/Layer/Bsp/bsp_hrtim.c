/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bsp_hrtim.c
  * @brief          : BSP HRTIM implementation file.
  *                   This file contains high-resolution timer service stubs.
  ******************************************************************************
  * @attention
  *
  * Author: LCYX
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "bsp_hrtim.h"
#include "main.h"
#include "hrtim.h"
#include "tim.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
Tick_t g_Ticks[TICK_MAX] = {
    { .id = TICK_S, .Tick = NULL },
    { .id = TICK_MS, .Tick = HAL_GetTick },
    { .id = TICK_US, .Tick = NULL }
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static uint16_t BSP_HRTIM_ClampDuty(uint16_t duty);

/* USER CODE END PFP */

/* Exported functions --------------------------------------------------------*/
void BSP_HRTIM_Init(void)
{
  /* USER CODE BEGIN BSP_HRTIM_Init */
	// 1. 唤醒“四肢”：开启所有的 PWM 物理输出引脚
  // （此时因为定时器还没走，输出被死区保护和比较器按在安全电平）
  HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2 | HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TB2);

  // 2. 启动“心脏与总指挥”：Master, Timer A, Timer B 瞬间同步起跑！
  // 就是这句代码，让底层 4.6GHz 的时钟开始狂奔，触发信号开始射向 ADC
  HAL_HRTIM_WaveformCountStart(&hhrtim1, HRTIM_TIMERID_MASTER | HRTIM_TIMERID_TIMER_A | HRTIM_TIMERID_TIMER_B);

  HAL_TIM_Base_Start_IT(&htim2);

  /* USER CODE END BSP_HRTIM_Init */
}

void BSP_HRTIM_Process(void)
{
  /* USER CODE BEGIN BSP_HRTIM_Process */
  
  /* USER CODE END BSP_HRTIM_Process */
}

/* USER CODE BEGIN 1 */
static uint16_t BSP_HRTIM_ClampDuty(uint16_t duty)
{
    if (duty > BSP_HRTIM_DUTY_MAX) {
        return BSP_HRTIM_DUTY_MAX;
    }

    return duty;
}

void BSP_HRTIM_UpdateDutySymmetric(uint16_t duty)
{
    uint16_t limited_duty = BSP_HRTIM_ClampDuty(duty);
    uint16_t cmp_low = (uint16_t)(BSP_HRTIM_DUTY_CENTER - limited_duty);
    uint16_t cmp_high = (uint16_t)(BSP_HRTIM_DUTY_CENTER + limited_duty);

    HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].CMP1xR = cmp_low;
    HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].CMP2xR = cmp_high;
    HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].CMP3xR = BSP_HRTIM_DUTY_CENTER;

    HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_B].CMP1xR = cmp_low;
    HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_B].CMP2xR = cmp_high;
    HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_B].CMP3xR = BSP_HRTIM_DUTY_CENTER;

    SET_BIT(HRTIM1->sCommonRegs.CR2, HRTIM_CR2_TASWU | HRTIM_CR2_TBSWU | HRTIM_CR2_MSWU);
}

/* ==================== TIM2 UI Blink Timebase ==================== */
/* TIM2用于UI光标闪烁的精确时基（500ms周期，2Hz频率）              */
/* ============================================================== */
volatile uint8_t g_UI_Blink_Flag = 0;      // 当前闪烁状态 (0=隐藏, 1=显示)
volatile uint8_t g_UI_Blink_Changed = 0;   // 标志位变化通知

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) {
        static uint32_t s_blink_counter = 0;

        s_blink_counter++;

        // 每 500,000 次 = 500ms (TIM2频率1MHz)
        if (s_blink_counter >= 500000U) {
            s_blink_counter = 0U;

            // 翻转闪烁状态
            g_UI_Blink_Flag = (g_UI_Blink_Flag == 0U) ? 1U : 0U;
            g_UI_Blink_Changed = 1U;
        }
    }
}
/* ==================== End of TIM2 Blink ====================== */

/* USER CODE END 1 */
