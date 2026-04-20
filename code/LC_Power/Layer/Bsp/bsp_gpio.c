/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bsp_gpio.c
  * @brief          : BSP GPIO implementation file.
  *                   This file contains GPIO access and service stubs.
  ******************************************************************************
  * @attention
  *
  * Author: LCYX
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "bsp_gpio.h"

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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Exported functions --------------------------------------------------------*/
void BSP_GPIO_Init(void)
{
  /* USER CODE BEGIN BSP_GPIO_Init */
  /* Initialize board-level GPIO directions and default output states here. */
  /* USER CODE END BSP_GPIO_Init */
}

void BSP_GPIO_Process(void)
{
  /* USER CODE BEGIN BSP_GPIO_Process */
  /* Execute periodic GPIO service logic here. */
  /* USER CODE END BSP_GPIO_Process */
}

uint8_t Drv_Btn_Read_SET1(void)
{
  /* USER CODE BEGIN Drv_Btn_Read_SET1 */
  return (HAL_GPIO_ReadPin(KEY1_SET_GPIO_Port, KEY1_SET_Pin) == GPIO_PIN_RESET) ? LEVEL_LOW : LEVEL_HIGH;
  /* USER CODE END Drv_Btn_Read_SET1 */
}

uint8_t Drv_Btn_Read_SET2(void)
{
  /* USER CODE BEGIN Drv_Btn_Read_SET2 */
  return (HAL_GPIO_ReadPin(KEY2_SET_GPIO_Port, KEY2_SET_Pin) == GPIO_PIN_RESET) ? LEVEL_LOW : LEVEL_HIGH;
  /* USER CODE END Drv_Btn_Read_SET2 */
}

uint8_t Drv_Btn_Read_UP1(void)
{
  /* USER CODE BEGIN Drv_Btn_Read_UP1 */
  return (HAL_GPIO_ReadPin(KEY1_UP_GPIO_Port, KEY1_UP_Pin) == GPIO_PIN_RESET) ? LEVEL_LOW : LEVEL_HIGH;
  /* USER CODE END Drv_Btn_Read_UP1 */
}

uint8_t Drv_Btn_Read_DOWN1(void)
{
  /* USER CODE BEGIN Drv_Btn_Read_DOWN1 */
  return (HAL_GPIO_ReadPin(KEY1_DOWN_GPIO_Port, KEY1_DOWN_Pin) == GPIO_PIN_RESET) ? LEVEL_LOW : LEVEL_HIGH;
  /* USER CODE END Drv_Btn_Read_DOWN1 */
}

uint8_t Drv_Btn_Read_UP2(void)
{
  /* USER CODE BEGIN Drv_Btn_Read_UP2 */
  return (HAL_GPIO_ReadPin(KEY2_UP_GPIO_Port, KEY2_UP_Pin) == GPIO_PIN_RESET) ? LEVEL_LOW : LEVEL_HIGH;
  /* USER CODE END Drv_Btn_Read_UP2 */
}

uint8_t Drv_Btn_Read_DOWN2(void)
{
  /* USER CODE BEGIN Drv_Btn_Read_DOWN2 */
  return (HAL_GPIO_ReadPin(KEY2_DOWN_GPIO_Port, KEY2_DOWN_Pin) == GPIO_PIN_RESET) ? LEVEL_LOW : LEVEL_HIGH;
  /* USER CODE END Drv_Btn_Read_DOWN2 */
}

uint8_t Drv_Btn_Read_EXIT(void)
{
  /* USER CODE BEGIN Drv_Btn_Read_EXIT1 */
  return (HAL_GPIO_ReadPin(KEY1_EXIT_GPIO_Port, KEY1_EXIT_Pin) == GPIO_PIN_RESET) ? LEVEL_LOW : LEVEL_HIGH;
  /* USER CODE END Drv_Btn_Read_EXIT1 */
}


/* ------------------------------------------实验区------------------------------------------*/

void Drv_LED0_ON()
{
  HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_RESET);
}

void Drv_LED0_OFF()
{
  HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_SET);
}
void Drv_LED1_ON()
{
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
}

void Drv_LED1_OFF()
{
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
}

void Drv_LED2_ON()
{
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
}

void Drv_LED2_OFF()
{
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
}

void Drv_LED3_ON()
{
  HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);
}

void Drv_LED3_OFF()
{
  HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET);
}

/* ------------------------------------------实验区------------------------------------------*/

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
