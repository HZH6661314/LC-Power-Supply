/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : state_machine.h
  * @brief          : Header for state_machine.c file.
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
#ifndef __STATE_MACHINE_H
#define __STATE_MACHINE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
// 状态接口定义
typedef struct {
    void (*Enter)(void);  // 进入函数指针
    void (*Run)(void);    // 运行函数指针
    void (*Exit)(void);   // 退出函数指针
} State_Handler_t;

typedef enum {
    STATE_STANDBY = 0,   // 待机模式
    STATE_SOFTSTART,     // 软启动模式
    STATE_RUNNING,       // 运行模式
    STATE_FAULT,         // 故障模式
    STATE_MAX,           // 状态机最大状态数
} State_t;

//利用共用体和位域，将 32 个标志位无缝压缩进 1 个 32 位变量中
typedef union {
    uint32_t all_flags;        // 用于一键清零所有标志：Flags.all_flags = 0;
    struct {
        uint32_t Output_En  : 1; // 仅占 1 bit
        uint32_t PD_Ready   : 1; // 仅占 1 bit
        uint32_t Fault_OVP  : 1; // 过压保护标志
        uint32_t Fault_OCP  : 1; // 过流保护标志
        uint32_t reserved   : 28;// 保留位，凑齐 32 bit (4字节对齐)
    } bits;
} SystemFlags_t;

//全局核心数据结构 (严格按大小降序排列，天然满足 4 字节对齐)
typedef struct {
    // --- 32-bit 变量区 (4 bytes) ---
    int32_t  Target_Voltage;   // 目标电压 (Q格式定点数)
    int32_t  Real_Voltage;     // 实际电压 (ADC采样滤波后)
    int32_t  Target_Current;   // 目标电流
    int32_t  Real_Current;     // 实际电流
    
    SystemFlags_t Flags;       // 系统标志位 (32-bit)
    
    // --- 16-bit 变量区 (2 bytes) ---
    uint16_t PWM_DutyCycle;    // PWM 占空比
    uint16_t Temperature;      // 实时温度
    
    // --- 8-bit 变量区 (1 byte) ---
    uint8_t  Active_Mode;      // 当前模式 (CV / CC)
    uint8_t  Error_Code;       // 故障代码
    uint8_t  Pad[2];           // 手动填充 2 字节，强制整个结构体大小为 4 的整数倍
} PowerSystemData_t;

// 声明一个全局唯一的实例
extern PowerSystemData_t SysData;


/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void State_Machine_Init(void);
void State_Machine_Process(void);
void StateMachine_Task(void);

// 定义状态处理机数组 (将所有模式挂载在这里)
extern State_Handler_t State_Standby;   // 待机模式
extern State_Handler_t State_SoftStart; // 软启动模式
extern State_Handler_t State_RunNing;   // 运行模式
extern State_Handler_t State_Fault;     // 故障模式

/* 待机模式状态机 接口定义 */
void Standby_Enter(void); // 待机模式进入函数
void Standby_Run(void);   // 待机模式运行函数
void Standby_Exit(void);  // 待机模式退出函数

/* 软启动模式状态机 接口定义 */
void SoftStart_Enter(void); // 软启动模式进入函数
void SoftStart_Run(void);   // 软启动模式运行函数
void SoftStart_Exit(void);  // 软启动模式退出函数

/* 运行模式状态机 接口定义 */
void RunNing_Enter(void);  // 运行模式进入函数
void RunNing_Run(void);    // 运行模式运行函数
void RunNing_Exit(void);   // 运行模式退出函数

/* 故障模式状态机 接口定义 */
void Fault_Enter(void);    // 故障模式进入函数
void Fault_Run(void);      // 故障模式运行函数
void Fault_Exit(void);     // 故障模式退出函数
/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __STATE_MACHINE_H */

/* Author: LCYX */
