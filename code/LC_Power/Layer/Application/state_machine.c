/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : state_machine.c
  * @brief          : Application state machine and UI control implementation.
  ******************************************************************************
  * @attention
  *
  * Author: LCYX
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#include "state_machine.h"

#include "bsp_adc.h"
#include "power_control.h"

PowerSystemData_t SysData;

State_Handler_t State_Standby = { Standby_Enter, Standby_Run, Standby_Exit };
State_Handler_t State_SoftStart = { SoftStart_Enter, SoftStart_Run, SoftStart_Exit };
State_Handler_t State_RunNing = { RunNing_Enter, RunNing_Run, RunNing_Exit };
State_Handler_t State_Fault = { Fault_Enter, Fault_Run, Fault_Exit };

static State_Handler_t *CurrentState;
static UI_State_t s_ui_state = UI_STATE_HOME_IDLE;
static SM_Focus_t s_focus = SM_FOCUS_SET_VOLTAGE;
static float s_TargetVoltageFinal = 0.0f;
static float s_CurrentLimit = 1.0f;
static float s_PowerLimit = SM_POWER_LIMIT_DEFAULT;
static uint8_t s_quick_set_cursor = 0U;
static uint8_t s_sys_set_cursor = 0U;
static uint8_t s_active_preset_index = 0xFFU;  // 0xFF = none active, 0-3 = valid preset

static float SM_ClampFloat(float value, float min_value, float max_value);
static void SM_ApplyPowerShield(SM_Focus_t changed_focus);
static void SM_MoveFocus(int8_t direction);
static void SM_ApplyQuickSet(uint8_t preset_index);
static void SM_ToggleOutput(void);
static void SM_SaveToFlash(void);
static void SM_ChangeState(State_Handler_t *next_state);

void State_Machine_Init(void)
{
    // 清零系统数据
    SysData.Flags.all_flags = 0U;
    SysData.Temperature = 36U;
    SysData.Active_Mode = 0U;
    SysData.Error_Code = 0U;
    SysData.Real_Voltage = 0;
    SysData.Real_Current = 0;
    SysData.Target_Voltage = 0;
    SysData.Target_Current = 1000;

    // 初始化UI状态
    s_ui_state = UI_STATE_HOME_IDLE;
    s_focus = SM_FOCUS_SET_VOLTAGE;
    s_quick_set_cursor = 0U;
    s_sys_set_cursor = 0U;

    // 初始化设定值
    s_TargetVoltageFinal = 0.0f;
    s_CurrentLimit = 1.0f;
    s_PowerLimit = SM_POWER_LIMIT_DEFAULT;

    // 使用标准切换函数进入初始状态（Karpathy原则：一致性）
    CurrentState = NULL;
    SM_ChangeState(&State_Standby);
}

void State_Machine_Process(void)
{
    StateMachine_Task();
}

void StateMachine_Task(void)
{
    SysData.Real_Voltage = (int32_t)(SM_Get_MeasuredVoltage() * 1000.0f);
    SysData.Real_Current = (int32_t)(SM_Get_MeasuredCurrent() * 1000.0f);
    SysData.Target_Voltage = (int32_t)(s_TargetVoltageFinal * 1000.0f);
    SysData.Target_Current = (int32_t)(s_CurrentLimit * 1000.0f);

    if ((CurrentState != 0) && (CurrentState->Run != 0)) {
        CurrentState->Run();
    }
}

void SM_Action_Enter(void)
{
    // Karpathy原则：早返回，避免深度嵌套

    // HOME_IDLE: SET按键切换输出状态
    if (s_ui_state == UI_STATE_HOME_IDLE) {
        SM_ToggleOutput();
        return;
    }

    // HOME_MENU: SET按键根据光标位置跳转
    if (s_ui_state == UI_STATE_HOME_MENU) {
        if ((s_focus == SM_FOCUS_SET_VOLTAGE) || (s_focus == SM_FOCUS_SET_CURRENT)) {
            s_ui_state = UI_STATE_HOME_EDIT;
            return;
        }

        if (s_focus == SM_FOCUS_QUICK_SET) {
            s_ui_state = UI_STATE_QUICK_SET;
            s_quick_set_cursor = 0U;
            return;
        }

        if (s_focus == SM_FOCUS_SETTINGS) {
            s_ui_state = UI_STATE_SYS_SET;
            s_sys_set_cursor = 0U;
            return;
        }

        return;
    }

    // HOME_EDIT: SET按键确认修改
    if (s_ui_state == UI_STATE_HOME_EDIT) {
        s_ui_state = UI_STATE_HOME_MENU;
        return;
    }

    // QUICK_SET: SET按键应用预设组
    if (s_ui_state == UI_STATE_QUICK_SET) {
        SM_ApplyQuickSet(s_quick_set_cursor);
        return;
    }

    // SYS_SET: SET按键进入子项编辑
    if (s_ui_state == UI_STATE_SYS_SET) {
        // TODO: 实现系统设置子项处理
        return;
    }
}

void SM_Action_Exit_Short(void)
{
    // HOME_IDLE: 无动作
    if (s_ui_state == UI_STATE_HOME_IDLE) {
        return;
    }

    // HOME_EDIT: 短按无效，需要长按才能取消
    if (s_ui_state == UI_STATE_HOME_EDIT) {
        return;
    }

    // HOME_MENU: 短按退回IDLE，触发参数保存
    if (s_ui_state == UI_STATE_HOME_MENU) {
        s_ui_state = UI_STATE_HOME_IDLE;
        SM_SaveToFlash();
        return;
    }

    // QUICK_SET: 短按隐藏光标或退回MENU
    if (s_ui_state == UI_STATE_QUICK_SET) {
        // TODO: 实现光标显示/隐藏逻辑
        s_ui_state = UI_STATE_HOME_MENU;
        return;
    }

    // SYS_SET: 短按返回上级
    if (s_ui_state == UI_STATE_SYS_SET) {
        s_ui_state = UI_STATE_HOME_MENU;
        return;
    }
}

void SM_Action_Exit_Long(void)
{
    // HOME_EDIT: 长按取消修改，直接退回IDLE
    if (s_ui_state == UI_STATE_HOME_EDIT) {
        // TODO: 恢复修改前的值
        s_ui_state = UI_STATE_HOME_IDLE;
        return;
    }

    // QUICK_SET: 长按直达IDLE
    if (s_ui_state == UI_STATE_QUICK_SET) {
        s_ui_state = UI_STATE_HOME_IDLE;
        return;
    }

    // SYS_SET: 长按直达IDLE
    if (s_ui_state == UI_STATE_SYS_SET) {
        s_ui_state = UI_STATE_HOME_IDLE;
        return;
    }

    // 其他状态: 直接回到IDLE
    s_ui_state = UI_STATE_HOME_IDLE;
    s_focus = SM_FOCUS_SET_VOLTAGE;
}

void SM_Action_Up(void)
{
    // 🔍 调试：LED0闪烁表示函数被调用（已禁用以恢复性能）
    // Drv_LED0_Toggle();

    // HOME_IDLE: UP按键唤醒光标，进入MENU状态
    if (s_ui_state == UI_STATE_HOME_IDLE) {
        // 🔍 调试：LED1点亮表示进入此分支（已禁用以恢复性能）
        // Drv_LED1_ON();

        s_ui_state = UI_STATE_HOME_MENU;
        s_focus = SM_FOCUS_SET_VOLTAGE;
        return;
    }

    // HOME_MENU: 光标上移
    if (s_ui_state == UI_STATE_HOME_MENU) {
        SM_MoveFocus(-1);
        return;
    }

    // HOME_EDIT: 增加参数值
    if (s_ui_state == UI_STATE_HOME_EDIT) {
        if (s_focus == SM_FOCUS_SET_VOLTAGE) {
            s_TargetVoltageFinal = SM_ClampFloat(s_TargetVoltageFinal + SM_VOLTAGE_STEP,
                                                 SM_VOLTAGE_MIN,
                                                 SM_VOLTAGE_MAX);
            SM_ApplyPowerShield(SM_FOCUS_SET_VOLTAGE);
        } else if (s_focus == SM_FOCUS_SET_CURRENT) {
            s_CurrentLimit = SM_ClampFloat(s_CurrentLimit + SM_CURRENT_STEP,
                                           SM_CURRENT_MIN,
                                           SM_CURRENT_MAX);
            SM_ApplyPowerShield(SM_FOCUS_SET_CURRENT);
        }
        return;
    }

    // QUICK_SET: 光标上移
    if (s_ui_state == UI_STATE_QUICK_SET) {
        if (s_quick_set_cursor > 0U) {
            s_quick_set_cursor--;
        } else {
            s_quick_set_cursor = 3U;
        }
        return;
    }

    // SYS_SET: 光标上移
    if (s_ui_state == UI_STATE_SYS_SET) {
        if (s_sys_set_cursor > 0U) {
            s_sys_set_cursor--;
        } else {
            s_sys_set_cursor = 7U;
        }
        return;
    }
}

void SM_Action_Down(void)
{
    // HOME_IDLE: DOWN按键唤醒光标，进入MENU状态
    if (s_ui_state == UI_STATE_HOME_IDLE) {
        s_ui_state = UI_STATE_HOME_MENU;
        s_focus = SM_FOCUS_SET_VOLTAGE;
        return;
    }

    // HOME_MENU: 光标下移
    if (s_ui_state == UI_STATE_HOME_MENU) {
        SM_MoveFocus(1);
        return;
    }

    // HOME_EDIT: 减少参数值
    if (s_ui_state == UI_STATE_HOME_EDIT) {
        if (s_focus == SM_FOCUS_SET_VOLTAGE) {
            s_TargetVoltageFinal = SM_ClampFloat(s_TargetVoltageFinal - SM_VOLTAGE_STEP,
                                                 SM_VOLTAGE_MIN,
                                                 SM_VOLTAGE_MAX);
            SM_ApplyPowerShield(SM_FOCUS_SET_VOLTAGE);
        } else if (s_focus == SM_FOCUS_SET_CURRENT) {
            s_CurrentLimit = SM_ClampFloat(s_CurrentLimit - SM_CURRENT_STEP,
                                           SM_CURRENT_MIN,
                                           SM_CURRENT_MAX);
            SM_ApplyPowerShield(SM_FOCUS_SET_CURRENT);
        }
        return;
    }

    // QUICK_SET: 光标下移
    if (s_ui_state == UI_STATE_QUICK_SET) {
        if (s_quick_set_cursor < 3U) {
            s_quick_set_cursor++;
        } else {
            s_quick_set_cursor = 0U;
        }
        return;
    }

    // SYS_SET: 光标下移
    if (s_ui_state == UI_STATE_SYS_SET) {
        if (s_sys_set_cursor < 7U) {
            s_sys_set_cursor++;
        } else {
            s_sys_set_cursor = 0U;
        }
        return;
    }
}

UI_State_t SM_Get_UI_State(void)
{
    return s_ui_state;
}

SM_Focus_t SM_Get_Focus(void)
{
    return s_focus;
}

uint8_t SM_Get_QuickSetCursor(void)
{
    return s_quick_set_cursor;
}

uint8_t SM_Get_SysSetCursor(void)
{
    return s_sys_set_cursor;
}

float SM_Get_TargetVoltageFinal(void)
{
    return s_TargetVoltageFinal;
}

float SM_Get_CurrentLimit(void)
{
    return s_CurrentLimit;
}

float SM_Get_PowerLimit(void)
{
    return s_PowerLimit;
}

float SM_Get_MeasuredVoltage(void)
{
    return Get_VOUT();
}

float SM_Get_MeasuredCurrent(void)
{
    return Get_IOUT();
}

float SM_Get_MeasuredPower(void)
{
    return SM_Get_MeasuredVoltage() * SM_Get_MeasuredCurrent();
}

uint8_t SM_Get_CCMode(void)
{
    return (Power_Control_GetActiveMode() == POWER_CONTROL_MODE_CC) ? 1U : 0U;
}

uint8_t SM_Get_OutputEnabled(void)
{
    return (uint8_t)SysData.Flags.bits.Output_En;
}

uint16_t SM_Get_TemperatureC(void)
{
    return SysData.Temperature;
}

static float SM_ClampFloat(float value, float min_value, float max_value)
{
    if (value > max_value) {
        return max_value;
    }

    if (value < min_value) {
        return min_value;
    }

    return value;
}

static void SM_ApplyPowerShield(SM_Focus_t changed_focus)
{
    float power = s_TargetVoltageFinal * s_CurrentLimit;

    if (power <= s_PowerLimit) {
        return;
    }

    if (changed_focus == SM_FOCUS_SET_VOLTAGE) {
        if (s_CurrentLimit > 0.001f) {
            s_TargetVoltageFinal = SM_ClampFloat(s_PowerLimit / s_CurrentLimit,
                                                 SM_VOLTAGE_MIN,
                                                 SM_VOLTAGE_MAX);
        } else {
            s_TargetVoltageFinal = SM_VOLTAGE_MAX;
        }
    } else if (changed_focus == SM_FOCUS_SET_CURRENT) {
        if (s_TargetVoltageFinal > 0.001f) {
            s_CurrentLimit = SM_ClampFloat(s_PowerLimit / s_TargetVoltageFinal,
                                           SM_CURRENT_MIN,
                                           SM_CURRENT_MAX);
        } else {
            s_CurrentLimit = SM_CURRENT_MAX;
        }
    }
}

static void SM_MoveFocus(int8_t direction)
{
    int8_t next = (int8_t)s_focus + direction;

    if (next < 0) {
        next = (int8_t)SM_FOCUS_SETTINGS;
    } else if (next >= (int8_t)SM_FOCUS_MAX) {
        next = (int8_t)SM_FOCUS_SET_VOLTAGE;
    }

    s_focus = (SM_Focus_t)next;
}

static void SM_ApplyQuickSet(uint8_t preset_index)
{
    // TODO: 从Flash读取预设组
    // 临时硬编码4个预设组
    const float quick_presets[4][2] = {
        {5.0f, 1.0f},   // 预设1: 5V/1A
        {9.0f, 2.0f},   // 预设2: 9V/2A
        {12.0f, 3.0f},  // 预设3: 12V/3A
        {20.0f, 3.0f}   // 预设4: 20V/3A
    };

    if (preset_index >= 4U) {
        return;
    }

    s_TargetVoltageFinal = quick_presets[preset_index][0];
    s_CurrentLimit = quick_presets[preset_index][1];
    SM_ApplyPowerShield(SM_FOCUS_SET_CURRENT);

    // Track which preset is currently active (for UI arrow indicator)
    s_active_preset_index = preset_index;
}

static void SM_ToggleOutput(void)
{
    // Karpathy原则：清晰的逻辑分支，避免嵌套
    if (CurrentState == &State_Standby) {
        // 从待机切换到软启动
        SM_ChangeState(&State_SoftStart);
        return;
    }

    if (CurrentState == &State_RunNing) {
        // 从运行切换到待机
        SM_ChangeState(&State_Standby);
        return;
    }

    // 其他状态不允许切换（安全防护）
}

static void SM_SaveToFlash(void)
{
    // TODO: 实现Flash参数保存
    // 1. 检查Dirty Flag
    // 2. 序列化参数
    // 3. 写入W25Q256
    // 4. 清除Dirty Flag
}

void Standby_Enter(void)
{
    SysData.Flags.bits.Output_En = 0U;
}

void Standby_Run(void)
{
    SysData.Active_Mode = (uint8_t)POWER_CONTROL_MODE_CV;
}

void Standby_Exit(void)
{
}

void SoftStart_Enter(void)
{
    // Karpathy原则：软启动也需要启用输出
    SysData.Flags.bits.Output_En = 1U;
}

void SoftStart_Run(void)
{
    // TODO: 实现真正的软启动逻辑（电压斜坡上升）
    // 目前简化为直接切换到运行状态

    SysData.Active_Mode = (uint8_t)Power_Control_GetActiveMode();

    // 软启动完成，切换到运行状态
    SM_ChangeState(&State_RunNing);
}

void SoftStart_Exit(void)
{
}

void RunNing_Enter(void)
{
    SysData.Flags.bits.Output_En = 1U;
}

void RunNing_Run(void)
{
    SysData.Active_Mode = (uint8_t)Power_Control_GetActiveMode();
}

void RunNing_Exit(void)
{
}

void Fault_Enter(void)
{
    SysData.Flags.bits.Output_En = 0U;
    SysData.Active_Mode = (uint8_t)POWER_CONTROL_MODE_CV;
}

void Fault_Run(void)
{
}

void Fault_Exit(void)
{
}

// ============================================================================
// 状态切换函数（Karpathy原则：优雅、安全、可读）
// ============================================================================

/**
 * @brief  安全地切换到新状态
 * @note   遵循状态机设计原则：Exit旧状态 -> 切换 -> Enter新状态
 * @param  next_state: 目标状态处理器指针
 * @retval None
 */
static void SM_ChangeState(State_Handler_t *next_state)
{
    // 安全防御：空指针检查
    if (next_state == NULL) {
        return;
    }

    // 1. 退出当前状态（清理资源）
    if ((CurrentState != NULL) && (CurrentState->Exit != NULL)) {
        CurrentState->Exit();
    }

    // 2. 切换到新状态
    CurrentState = next_state;

    // 3. 进入新状态（初始化资源）
    if ((CurrentState != NULL) && (CurrentState->Enter != NULL)) {
        CurrentState->Enter();
    }
}

/**
 * @brief  获取指定索引的快速设置预设值
 * @note   UI层通过此接口读取预设数据用于显示
 * @param  index: 预设索引 (0-3)
 * @param  voltage: 输出电压指针 (V)
 * @param  current: 输出电流指针 (A)
 * @retval None
 */
void SM_Get_QuickSetPreset(uint8_t index, float *voltage, float *current)
{
    // 预设数组（与SM_ApplyQuickSet保持一致）
    const float quick_presets[4][2] = {
        {5.0f, 1.0f},   // 预设1: 5V/1A
        {9.0f, 2.0f},   // 预设2: 9V/2A
        {12.0f, 3.0f},  // 预设3: 12V/3A
        {20.0f, 3.0f}   // 预设4: 20V/3A
    };

    // Karpathy原则：早返回，避免嵌套
    if (index >= 4U) {
        return;
    }

    if ((voltage == NULL) || (current == NULL)) {
        return;
    }

    *voltage = quick_presets[index][0];
    *current = quick_presets[index][1];
}

/**
 * @brief  获取当前激活的预设索引
 * @note   用于UI显示箭头指示器
 * @retval 预设索引 (0-3 = 有效预设, 0xFF = 无预设激活)
 */
uint8_t SM_Get_ActivePresetIndex(void)
{
    return s_active_preset_index;
}

