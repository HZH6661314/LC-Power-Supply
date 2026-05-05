/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : power_control.c
  * @brief          : Power control implementation file.
  *                   This file contains power control calculation stubs.
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

/* Includes ------------------------------------------------------------------*/
#include "power_control.h"

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
// 实例化外环(电压)和内环(电流)控制器，它们存放在极速 CCM RAM 中
// 直接指定内存地址即可
PID_TypeDef Voltage_Loop __attribute__((at(0x10000000))); 
PID_TypeDef Current_Loop __attribute__((at(0x10000040))); // 注意地址偏移
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
/**
 * @brief 极速 PID 计算核心 (强制内联优化，消除函数调用的进出栈开销)
 * @note  必须放置在 CCM RAM 中执行
 */
static inline float PID_Calc(PID_TypeDef *pid, float target, float measure) {
    float error = target - measure;
    
    // 1. 比例项 (P)
    float out = pid->Kp * error;
    
    // 2. 积分项 (I) + 极其关键的抗饱和机制 (Anti-Windup)
    pid->Integral += pid->Ki_dt * error;
    if (pid->Integral > pid->Out_Max) {
        pid->Integral = pid->Out_Max;
    } else if (pid->Integral < pid->Out_Min) {
        pid->Integral = pid->Out_Min;
    }
    out += pid->Integral;
    
    // 3. 微分项 (D) - 如果内环不需要D，可将 Kd_div_dt 设为 0.0f
    out += pid->Kd_div_dt * (error - pid->Error_Last);
    pid->Error_Last = error;
    
    // 4. 总输出限幅
    if (out > pid->Out_Max) {
        return pid->Out_Max;
    } else if (out < pid->Out_Min) {
        return pid->Out_Min;
    }
    
    return out;
}
/* USER CODE END PFP */

/* Exported functions --------------------------------------------------------*/
void Power_Control_Init(void)
{
  /* USER CODE BEGIN Power_Control_Init */
  
  /* USER CODE END Power_Control_Init */
}

void Power_Control_Process(void)
{
  /* USER CODE BEGIN Power_Control_Process */
  
  /* USER CODE END Power_Control_Process */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
