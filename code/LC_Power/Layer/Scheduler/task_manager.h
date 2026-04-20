/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : task_manager.h
  * @brief          : Task manager header file.
  *                   Provides scheduler task initialization and processing.
  ******************************************************************************
  * @attention
  *
  * Author: LCYX
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __TASK_MANAGER_H
#define __TASK_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ui_display.h"

/* Exported functions prototypes ---------------------------------------------*/
void SysCore_Init(void);
void SysCore_Run(void);

#ifdef __cplusplus
}
#endif

#endif /* __TASK_MANAGER_H */
