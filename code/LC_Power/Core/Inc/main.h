/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f3xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void Hardware_Init(void);
void Power_Control_Init(void);
void BSP_Init(void);



/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED2_Pin GPIO_PIN_13
#define LED2_GPIO_Port GPIOC
#define KEY2_SET_Pin GPIO_PIN_14
#define KEY2_SET_GPIO_Port GPIOC
#define LED3_Pin GPIO_PIN_15
#define LED3_GPIO_Port GPIOC
#define ADC_VIN_Pin GPIO_PIN_0
#define ADC_VIN_GPIO_Port GPIOA
#define ADC_IIN_Pin GPIO_PIN_1
#define ADC_IIN_GPIO_Port GPIOA
#define ADC_VOUT_Pin GPIO_PIN_2
#define ADC_VOUT_GPIO_Port GPIOA
#define ADC_IOUT_Pin GPIO_PIN_3
#define ADC_IOUT_GPIO_Port GPIOA
#define ADC_TEMP_Pin GPIO_PIN_4
#define ADC_TEMP_GPIO_Port GPIOA
#define W25Q256_SCK_Pin GPIO_PIN_5
#define W25Q256_SCK_GPIO_Port GPIOA
#define W25Q256_MISO_Pin GPIO_PIN_6
#define W25Q256_MISO_GPIO_Port GPIOA
#define W25Q256_MOSI_Pin GPIO_PIN_7
#define W25Q256_MOSI_GPIO_Port GPIOA
#define W25Q256_CS_Pin GPIO_PIN_0
#define W25Q256_CS_GPIO_Port GPIOB
#define KEY1_SET_Pin GPIO_PIN_1
#define KEY1_SET_GPIO_Port GPIOB
#define KEY1_EXIT_Pin GPIO_PIN_2
#define KEY1_EXIT_GPIO_Port GPIOB
#define KEY1_UP_Pin GPIO_PIN_10
#define KEY1_UP_GPIO_Port GPIOB
#define KEY1_DOWN_Pin GPIO_PIN_11
#define KEY1_DOWN_GPIO_Port GPIOB
#define KEY2_UP_Pin GPIO_PIN_12
#define KEY2_UP_GPIO_Port GPIOB
#define KEY2_DOWN_Pin GPIO_PIN_13
#define KEY2_DOWN_GPIO_Port GPIOB
#define BUZZER_Pin GPIO_PIN_14
#define BUZZER_GPIO_Port GPIOB
#define FAN_PWM_Pin GPIO_PIN_15
#define FAN_PWM_GPIO_Port GPIOB
#define PWM1H_Pin GPIO_PIN_8
#define PWM1H_GPIO_Port GPIOA
#define PWM1L_Pin GPIO_PIN_9
#define PWM1L_GPIO_Port GPIOA
#define PWM2H_Pin GPIO_PIN_10
#define PWM2H_GPIO_Port GPIOA
#define PWM2L_Pin GPIO_PIN_11
#define PWM2L_GPIO_Port GPIOA
#define LED1_Pin GPIO_PIN_12
#define LED1_GPIO_Port GPIOA
#define LCD_RES_Pin GPIO_PIN_15
#define LCD_RES_GPIO_Port GPIOA
#define LCD_SDA_Pin GPIO_PIN_3
#define LCD_SDA_GPIO_Port GPIOB
#define LCD_SCL_Pin GPIO_PIN_4
#define LCD_SCL_GPIO_Port GPIOB
#define LCD_CS_Pin GPIO_PIN_5
#define LCD_CS_GPIO_Port GPIOB
#define LCD_DC_Pin GPIO_PIN_6
#define LCD_DC_GPIO_Port GPIOB
#define LED0_Pin GPIO_PIN_7
#define LED0_GPIO_Port GPIOB
#define LCD_BL_Pin GPIO_PIN_8
#define LCD_BL_GPIO_Port GPIOB
#define DC_OUT_CTRL_Pin GPIO_PIN_9
#define DC_OUT_CTRL_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
