/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bsp_hrtim.h
  * @brief          : Header for bsp_hrtim.c file.
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
#ifndef __BSP_HRTIM_H
#define __BSP_HRTIM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef enum {
    TICK_S =  0,
    TICK_MS = 1,
    TICK_US = 2,
    TICK_MAX = 3
} TIMId_t;

typedef struct {
    uint8_t   id;                 // 按键的唯一标识（如 BTN_UP, BTN_DOWN）
    uint32_t  (*Tick)(void);     // 虚函数：指向底层硬件读取函数
} Tick_t;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void BSP_HRTIM_Init(void);
void BSP_HRTIM_Process(void);

/* USER CODE BEGIN EFP */
extern Tick_t g_Ticks[TICK_MAX];
void BSP_HRTIM_UpdateDutySymmetric(uint16_t duty);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */
#define BSP_HRTIM_DUTY_MAX      (5760U)
#define BSP_HRTIM_DUTY_CENTER   (5760U)
#define BSP_HRTIM_PERIOD_TICKS  (11520U)

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __BSP_HRTIM_H */

/* Author: LCYX */
