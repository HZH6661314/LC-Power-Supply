/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : state_machine.h
  * @brief          : Application state machine and UI control brain.
  ******************************************************************************
  * @attention
  *
  * Author: LCYX
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __STATE_MACHINE_H
#define __STATE_MACHINE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>

#define SM_VOLTAGE_MIN          (0.0f)
#define SM_VOLTAGE_MAX          (48.0f)
#define SM_CURRENT_MIN          (0.0f)
#define SM_CURRENT_MAX          (5.0f)
#define SM_POWER_LIMIT_DEFAULT  (100.0f)
#define SM_VOLTAGE_STEP         (0.5f)
#define SM_CURRENT_STEP         (0.1f)

typedef struct {
    void (*Enter)(void);
    void (*Run)(void);
    void (*Exit)(void);
} State_Handler_t;

typedef enum {
    STATE_STANDBY = 0,
    STATE_SOFTSTART,
    STATE_RUNNING,
    STATE_FAULT,
    STATE_MAX
} State_t;

typedef enum {
    UI_STATE_HOME_IDLE = 0,     // 主界面-待机/运行态（无光标）
    UI_STATE_HOME_MENU,          // 主界面-菜单选择态（光标显示）
    UI_STATE_HOME_EDIT,          // 主界面-参数编辑态（光标闪烁2Hz）
    UI_STATE_QUICK_SET,          // 快速设置子菜单
    UI_STATE_SYS_SET,            // 系统设置子菜单
    UI_STATE_MAX
} UI_State_t;

typedef enum {
    SM_FOCUS_SET_VOLTAGE = 0,
    SM_FOCUS_SET_CURRENT,
    SM_FOCUS_QUICK_SET,
    SM_FOCUS_SETTINGS,
    SM_FOCUS_MAX
} SM_Focus_t;

typedef union {
    uint32_t all_flags;
    struct {
        uint32_t Output_En  : 1;
        uint32_t PD_Ready   : 1;
        uint32_t Fault_OVP  : 1;
        uint32_t Fault_OCP  : 1;
        uint32_t reserved   : 28;
    } bits;
} SystemFlags_t;

typedef struct {
    int32_t Target_Voltage;
    int32_t Real_Voltage;
    int32_t Target_Current;
    int32_t Real_Current;
    SystemFlags_t Flags;
    uint16_t PWM_DutyCycle;
    uint16_t Temperature;
    uint8_t Active_Mode;
    uint8_t Error_Code;
    uint8_t Pad[2];
} PowerSystemData_t;

extern PowerSystemData_t SysData;

void State_Machine_Init(void);
void State_Machine_Process(void);
void StateMachine_Task(void);

void SM_Action_Enter(void);
void SM_Action_Exit_Short(void);
void SM_Action_Exit_Long(void);
void SM_Action_Up(void);
void SM_Action_Down(void);

UI_State_t SM_Get_UI_State(void);
SM_Focus_t SM_Get_Focus(void);
uint8_t SM_Get_QuickSetCursor(void);
uint8_t SM_Get_SysSetCursor(void);
void SM_Get_QuickSetPreset(uint8_t index, float *voltage, float *current);
uint8_t SM_Get_ActivePresetIndex(void);
float SM_Get_TargetVoltageFinal(void);
float SM_Get_CurrentLimit(void);
float SM_Get_PowerLimit(void);
float SM_Get_MeasuredVoltage(void);
float SM_Get_MeasuredCurrent(void);
float SM_Get_MeasuredPower(void);
uint8_t SM_Get_CCMode(void);
uint8_t SM_Get_OutputEnabled(void);
uint16_t SM_Get_TemperatureC(void);

extern State_Handler_t State_Standby;
extern State_Handler_t State_SoftStart;
extern State_Handler_t State_RunNing;
extern State_Handler_t State_Fault;

void Standby_Enter(void);
void Standby_Run(void);
void Standby_Exit(void);
void SoftStart_Enter(void);
void SoftStart_Run(void);
void SoftStart_Exit(void);
void RunNing_Enter(void);
void RunNing_Run(void);
void RunNing_Exit(void);
void Fault_Enter(void);
void Fault_Run(void);
void Fault_Exit(void);

#ifdef __cplusplus
}
#endif

#endif /* __STATE_MACHINE_H */

/* Author: LCYX */
