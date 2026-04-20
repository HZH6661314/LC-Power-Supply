/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bsp_gpio.h
  * @brief          : Header for bsp_gpio.c file.
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
#ifndef __BSP_GPIO_H
#define __BSP_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
#include "gpio.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef enum
{
  LEVEL_LOW = 0u,
  LEVEL_HIGH
} Pin_State;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void BSP_GPIO_Init(void);
void BSP_GPIO_Process(void);

uint8_t Drv_Btn_Read_SET1(void);
uint8_t Drv_Btn_Read_SET2(void);
uint8_t Drv_Btn_Read_UP1(void);
uint8_t Drv_Btn_Read_DOWN1(void);
uint8_t Drv_Btn_Read_UP2(void);
uint8_t Drv_Btn_Read_DOWN2(void);
uint8_t Drv_Btn_Read_EXIT(void);

void Drv_LED0_ON();
void Drv_LED0_OFF();  
void Drv_LED1_ON();
void Drv_LED1_OFF();
void Drv_LED2_ON();
void Drv_LED2_OFF();
void Drv_LED3_ON();  
void Drv_LED3_OFF();


/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __BSP_GPIO_H */

/* Author: LCYX */
