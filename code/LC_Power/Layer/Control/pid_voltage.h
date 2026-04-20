/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : pid_voltage.h
  * @brief          : Header for pid_voltage.c file.
  *                   This file contains the module definitions and interfaces.
  ******************************************************************************
  * @attention
  * The Control layer should not contain low-level driver code; it 
  * should only have pure C language arithmetic operations such as 
  * addition, subtraction, multiplication, and division.
  * 
  * Author: LCYX
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PID_VOLTAGE_H
#define __PID_VOLTAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
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
void PID_Voltage_Init(void);
void PID_Voltage_Process(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __PID_VOLTAGE_H */

/* Author: LCYX */
