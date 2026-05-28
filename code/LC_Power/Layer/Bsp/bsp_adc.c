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

ADC_Channel_Data_t g_adc_dma_buffer[DECIMATION_RATE] = {0};

static float ADC_Value[ADC_CHANNEL_MAX] = {0}; // ADC 转换后的物理值，单位 V 或 A，根据通道不同而不同
static float s_adc_cali[ADC_CHANNEL_MAX] = {0,0,0.137f,0.005f,0,0}; // ADC 校准值，单位与 ADC_Value 数组相同，初始值为0

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Exported functions --------------------------------------------------------*/
void BSP_ADC_Init(void)
{
  /* USER CODE BEGIN BSP_ADC_Init */
  
  uint32_t dma_length = sizeof(g_adc_dma_buffer) / sizeof(uint32_t);
  HAL_ADCEx_Calibration_Start(&hadc1,ADC_SINGLE_ENDED);
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)g_adc_dma_buffer, dma_length);
    

  /* USER CODE END BSP_ADC_Init */
}

void BSP_ADC_Process(void)
{
  /* USER CODE BEGIN BSP_ADC_Process */
  uint32_t sum_vin = 0, sum_iin = 0, sum_vout = 0, sum_iout = 0;
        
  // 由于 ADC 是通过 DMA 直接搬运到内存的，所以我们在这里直接操作内存中的数据就好了，不需要再去读寄存器了。
  ADC_Channel_Data_t *pData = g_adc_dma_buffer;
  
  // 由于 ADC 的采样频率非常高，我们需要通过累加器和计数器来进行“降频抽取”，以降低控制环路的频率到一个合理的范围。
  for(uint8_t i = 0; i < DECIMATION_RATE; i++) {
      sum_vin  += pData->vin;
      sum_iin  += pData->iin;
      sum_vout += pData->vout;
      sum_iout += pData->iout;
      
      pData++; // 指针移动到下一个采样数据
  }
  
  
  ADC_Value[ADC_VIN]  = (sum_vin  / (float)DECIMATION_RATE) * VIN_COEF;
  ADC_Value[ADC_IIN]  =  IIN_COEF((sum_iin  / (float)DECIMATION_RATE));
  ADC_Value[ADC_VOUT] = (sum_vout / (float)DECIMATION_RATE) * VOUT_COEF;
  ADC_Value[ADC_IOUT] =  IOUT_COEF((sum_iout / (float)DECIMATION_RATE));
  /* USER CODE END BSP_ADC_Process */
}

/* USER CODE BEGIN 1 */
float Get_VIN(void)
{
    return ADC_Value[ADC_VIN]+s_adc_cali[ADC_VIN]; // 获取输入电压
}

float Get_IIN(void)
{
    return ADC_Value[ADC_IIN]+s_adc_cali[ADC_IIN]; // 获取输入电流
}

float Get_VOUT(void)
{
    return ADC_Value[ADC_VOUT]+s_adc_cali[ADC_VOUT]; // 获取输出电压
}

float Get_IOUT(void)
{
    return ADC_Value[ADC_IOUT]+s_adc_cali[ADC_IOUT]; // 获取输出电流
}
/* USER CODE END 1 */
