/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : state_machine.c
  * @brief          : State machine implementation file.
  *                   This file contains application state handling stubs.
  ******************************************************************************
  * @attention
  *
  * Author: LCYX
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "state_machine.h"

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
State_Handler_t State_Standby = {Standby_Enter, Standby_Run, Standby_Exit};         // 待机模式
State_Handler_t State_SoftStart = {SoftStart_Enter, SoftStart_Run, SoftStart_Exit}; // 软启动模式
State_Handler_t State_RunNing = {RunNing_Enter, RunNing_Run, RunNing_Exit};         // 运行模式
State_Handler_t State_Fault = {Fault_Enter, Fault_Run, Fault_Exit};                 // 故障模式

State_Handler_t *CurrentState;        // 定义一个指针，永远指向当前正在运行的状态
void ChangeState(State_Handler_t *NextState); // 定义一个函数，用于切换状态，参数是下一个状态的指针

/* USER CODE END PFP */

/* Exported functions --------------------------------------------------------*/
void State_Machine_Init(void)
{
  /* USER CODE BEGIN State_Machine_Init */
  /* 初始化状态机，设置初始状态为待机模式 */
  CurrentState = &State_Standby; // 初始状态设置为待机模式
  if (CurrentState && CurrentState->Enter) {
      CurrentState->Enter(); // 初始状态进入函数
  }

  /* USER CODE END State_Machine_Init */
}

void State_Machine_Process(void)
{
  /* USER CODE BEGIN State_Machine_Process */
  /* Execute one state machine cycle and handle transitions here. */
  /* USER CODE END State_Machine_Process */
}

void StateMachine_Task(void) 
{

    if (CurrentState && CurrentState->Run) {
        CurrentState->Run(); // 主循环只管无脑调用当前状态的 Run 函数
    }

}

void ChangeState(State_Handler_t *NextState) {
    if (NextState == NULL) return; // 安全防御
    
    if (CurrentState && CurrentState->Exit) {
        CurrentState->Exit();
    }
    
    CurrentState = NextState;
    
    if (CurrentState && CurrentState->Enter) {
        CurrentState->Enter();
    }
}

/* -------------------- 待机模式状态机 ------------------- */

/**
  * @brief  待机模式进入函数
  * @retval None
  */
void Standby_Enter(void) 
{ 
  /* 关断 PWM */ 
  

}

/**
  * @brief  待机模式运行函数
  * @retval None
  */
void Standby_Run(void)   
{ 
  /* 监测 PD 握手情况 */ 


}

/**
  * @brief  待机模式退出函数
  * @retval None
  */
void Standby_Exit(void)  
{ 
  /* 准备开启输出 */


}
/* -------------------- 待机模式状态机 ------------------- */



/* -------------------- 软启动模式状态机 ------------------- */
/**
  * @brief  软启动模式进入函数
  * @retval None
  */
 void SoftStart_Enter(void)
  {
    /* 软启动模式进入时的初始化操作 */
    /* 例如：逐步增加 PWM 占空比，监测系统状态等 */
  }

  /**
  * @brief  软启动模式运行函数
  * @retval None
  */
  void SoftStart_Run(void)
  {
    /* 软启动模式运行中的操作 */
    /* 例如：逐步增加 PWM 占空比，监测系统状态等 */
  }

  /**
  * @brief  软启动模式退出函数
  * @retval None
  */
  void SoftStart_Exit(void)
  {
    /* 软启动模式退出时的清理操作 */
    /* 例如：确保 PWM 占空比达到预期值，准备进入运行模式等 */
  }
  /* -------------------- 软启动模式状态机 ------------------- */



  /* -------------------- 运行模式状态机 ------------------- */
  /**
  * @brief  运行模式进入函数
  * @retval None
  */
  void RunNing_Enter(void)
  {
    /* 运行模式进入时的初始化操作 */
    /* 例如：初始化 PWM 占空比，准备进入运行模式等 */
  }

  /**
  * @brief  运行模式运行函数
  * @retval None
  */
  void RunNing_Run(void)
  {
    /* 运行模式运行中的操作 */
    /* 例如：更新 PWM 占空比，监测系统状态等 */
  }

  /**
  * @brief  运行模式退出函数
  * @retval None
  */
  void RunNing_Exit(void)
  {
    /* 运行模式退出时的清理操作 */
    /* 例如：确保 PWM 占空比达到预期值，准备进入待机模式等 */
  }
  /* -------------------- 运行模式状态机 ------------------- */



  /* -------------------- 故障模式状态机 ------------------- */
  /**
  * @brief  故障模式进入函数
  * @retval None
  */
  void Fault_Enter(void)
  {
    /* 故障模式进入时的初始化操作 */
    /* 例如：初始化 PWM 占空比，准备进入故障模式等 */
  }

  /**
  * @brief  故障模式运行函数
  * @retval None
  */
  void Fault_Run(void)
  {
    /* 故障模式运行中的操作 */
    /* 例如：更新 PWM 占空比，监测系统状态等 */
  }

  /**
  * @brief  故障模式退出函数
  * @retval None
  */
  void Fault_Exit(void)
  {
    /* 故障模式退出时的清理操作 */
    /* 例如：确保 PWM 占空比达到预期值，准备进入待机模式等 */
  }
  /* -------------------- 故障模式状态机 ------------------- */



/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
