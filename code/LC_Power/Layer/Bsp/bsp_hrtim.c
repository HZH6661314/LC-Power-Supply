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
  /* Initialize HRTIM resources and PWM base configuration here. */
  /* USER CODE END BSP_HRTIM_Init */
}

void BSP_HRTIM_Process(void)
{
  /* USER CODE BEGIN BSP_HRTIM_Process */
  /* Handle periodic HRTIM runtime service logic here. */
  /* USER CODE END BSP_HRTIM_Process */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
