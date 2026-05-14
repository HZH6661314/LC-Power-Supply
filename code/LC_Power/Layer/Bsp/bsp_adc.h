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

/* USER CODE END Includes */

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

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */
#define ADC_VIN_VALUE READ_REG(hadc1.Instance->JDR1)
#define ADC_IIN_VALUE READ_REG(hadc1.Instance->JDR2)
#define ADC_VOUT_VALUE READ_REG(hadc1.Instance->JDR3)
#define ADC_IOUT_VALUE READ_REG(hadc1.Instance->JDR4)
#define ADC_TEMP_VALUE READ_REG(hadc2.Instance->JDR1)
#define ADC_VREF_VALUE READ_REG(hadc2.Instance->JDR2)
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __BSP_ADC_H */

/* Author: LCYX */
