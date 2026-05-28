/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bsp_adc.h
  * @brief          : Header for bsp_adc.c file.
  *                   This file contains the module definitions and interfaces.
  ******************************************************************************
  * @attention
  *
  * Author: LCYX
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_ADC_H
#define __BSP_ADC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdint.h"
/* USER CODE END Includes */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */
#define VIN_COEF 0.01289f   // 输入电压转换系数，单位 V/count
#define IIN_COEF(x) (x * 0.00805664f)-16.5f  // 输入电流转换系数，单位 A/count，x 是分流电阻值，单位欧姆 
#define VOUT_COEF 0.01289f  // 输出电压转换系数，单位 V/count
#define IOUT_COEF(x) (x * 0.00805664f)-16.5f
#define TEMP_COEF 0
#define VREF_COEF 0



#define DECIMATION_RATE  20  // 400kHz / 20 = 20kHz 的控制环路频率
/* USER CODE END Private defines */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef enum {
    ADC_VIN = 0,
    ADC_IIN,
    ADC_VOUT,
    ADC_IOUT,
    ADC_TEMP,
    ADC_VREF,
    ADC_CHANNEL_MAX
} ADC_ID_t;

typedef struct {
    uint32_t vin;   // Rank 1: 输入电压
    uint32_t iin;   // Rank 2: 输入电流
    uint32_t vout;  // Rank 3: 输出电压
    uint32_t iout;  // Rank 4: 输出电流
} ADC_Channel_Data_t;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void BSP_ADC_Init(void);
void BSP_ADC_Process(void);

float Get_VIN(void);
float Get_IIN(void);
float Get_VOUT(void);
float Get_IOUT(void);


/* USER CODE BEGIN EFP */

/* USER CODE END EFP */



#ifdef __cplusplus
}
#endif

#endif /* __BSP_ADC_H */

/* Author: LCYX */
