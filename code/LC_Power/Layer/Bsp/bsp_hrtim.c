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

/* USER CODE END PFP */

/* Exported functions --------------------------------------------------------*/
void BSP_HRTIM_Init(void)
{
  /* USER CODE BEGIN BSP_HRTIM_Init */
	HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].CMP1xR = 5760-2880;
	HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].CMP2xR = 5760+2880;
	HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].CMP3xR = 5760;
	
	HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_B].CMP1xR = 5760-5529;
	HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_B].CMP2xR = 5760+5529;
	HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_B].CMP3xR = 5760;
	
	SET_BIT(HRTIM1->sCommonRegs.CR2, HRTIM_CR2_TASWU | HRTIM_CR2_TBSWU | HRTIM_CR2_MSWU);

	// 1. 唤醒“四肢”：开启所有的 PWM 物理输出引脚
    // （此时因为定时器还没走，输出被死区保护和比较器按在安全电平）
    HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2 | HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TB2);

    // 2. 启动“心脏与总指挥”：Master, Timer A, Timer B 瞬间同步起跑！
    // 就是这句代码，让底层 4.6GHz 的时钟开始狂奔，触发信号开始射向 ADC
    HAL_HRTIM_WaveformCountStart(&hhrtim1, HRTIM_TIMERID_MASTER | HRTIM_TIMERID_TIMER_A | HRTIM_TIMERID_TIMER_B);
  /* USER CODE END BSP_HRTIM_Init */
}

void BSP_HRTIM_Process(void)
{
  /* USER CODE BEGIN BSP_HRTIM_Process */
  
  /* USER CODE END BSP_HRTIM_Process */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
