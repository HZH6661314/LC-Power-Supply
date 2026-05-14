/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bsp_adc.c
  * @brief          : BSP ADC implementation file.
  *                   This file contains ADC peripheral access stubs.
  ******************************************************************************
  * @attention
  *
  * Author: LCYX
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "bsp_adc.h"
#include "main.h"
#include "adc.h"

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
float g_adcValue[ADC_CHANNEL_MAX];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Exported functions --------------------------------------------------------*/
void BSP_ADC_Init(void)
{
  /* USER CODE BEGIN BSP_ADC_Init */
  
    // 1. 极其关键的“开机自检”：硬件自校准！
    // 抵消芯片出厂和温度变化带来的零点漂移，对电源的高精度采样至关重要
    if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK)
    {
        Error_Handler();
    }
    // 2. 启动注入通道，并开启中断（极速域的入口）
    // 执行完这句后，ADC 就进入了“挂起”状态，死死盯着 HRTIM 的触发信号
    if (HAL_ADCEx_InjectedStart_IT(&hadc1) != HAL_OK)
    {
        Error_Handler();
    }
    // 校准ADC2
    if (HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED) != HAL_OK)
    {
        Error_Handler();
    }
    // 启动ADC2注入通道
    if (HAL_ADCEx_InjectedStart_IT(&hadc2) != HAL_OK)
    {
        Error_Handler();
    }


    
  /* USER CODE END BSP_ADC_Init */
}

void BSP_ADC_Process(void)
{
  /* USER CODE BEGIN BSP_ADC_Process */

  /* USER CODE END BSP_ADC_Process */
}

/* USER CODE BEGIN 1 */
float Get_VIN(void)
{
    return ADC_VIN_VALUE/4096.0f * 3.3f * 16.0f; // 计算输入电压
}

float Get_IIN(void)
{
    return (ADC_IIN_VALUE/4096.0f * 3.3f - 1.65f) * 10.0f; // 输入电流
}

float Get_VOUT(void)
{
    return ADC_VOUT_VALUE/4096.0f * 3.3f * 16.0f; // 输出电压
}

float Get_IOUT(void)
{
    return (ADC_IOUT_VALUE/4096.0f * 3.3f - 1.65f) * 10.0f; // 输出电流
}
/* USER CODE END 1 */
