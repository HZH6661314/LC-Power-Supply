/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : power_control.h
  * @brief          : Header for power_control.c file.
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
#ifndef __POWER_CONTROL_H
#define __POWER_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>


/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

typedef enum {
    POWER_CONTROL_MODE_CV = 0U,
    POWER_CONTROL_MODE_CC = 1U
} PowerControlMode_t;

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
// 极简双环 PID 结构体 (去除了一切不必要的冗余变量)
typedef struct {
    float Kp;
    float Ki_dt;   // 已经预先乘过采样时间 dt 的 Ki
    float Kd_div_dt; // 已经预先除过采样时间 dt 的 Kd
    
    float Error_Last;
    float Integral;
    
    float Out_Max; // 积分抗饱和与输出限幅上限
    float Out_Min; // 积分抗饱和与输出限幅下限
} PID_TypeDef;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Power_Control_Init(void);
void Power_Control_Process(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __POWER_CONTROL_H */

/* Author: LCYX */
