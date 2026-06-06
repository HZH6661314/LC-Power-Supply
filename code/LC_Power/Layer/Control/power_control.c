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
#include "bsp_adc.h"
#include "bsp_hrtim.h"
#include "bsp_gpio.h"
#include "state_machine.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define POWER_CONTROL_DUTY_MAX          ((float)BSP_HRTIM_DUTY_MAX)
#define POWER_CONTROL_DUTY_MIN          (200.0f)
#define POWER_CONTROL_DEFAULT_VOLTAGE   (0.0f)
#define POWER_CONTROL_DEFAULT_CURRENT   (0.0f)
#define POWER_CONTROL_DEFAULT_POWER     (100.0f)
#define POWER_CONTROL_VOLTAGE_RAMP_STEP (0.1f)


#define POWER_PID_VOL_KP           (24.0f)   //电压环比例系数
#define POWER_PID_VOL_KI           (12.0f)   //电压环积分系数
#define POWER_PID_VOL_KD           (0.0f)   //电压环微分系数
#define POWER_PID_CUR_KP           (24.0f)   //电流环比例系数
#define POWER_PID_CUR_KI           (12.0f)   //电流环积分系数
#define POWER_PID_CUR_KD           (0.0f)   //电流环微分系数

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
__attribute__((section(".ccmram"))) PID_TypeDef Voltage_Loop ;
__attribute__((section(".ccmram"))) PID_TypeDef Current_Loop ;

static float s_TargetVoltageFinal = POWER_CONTROL_DEFAULT_VOLTAGE;
static float s_TargetVoltageRamp = POWER_CONTROL_DEFAULT_VOLTAGE;
static float s_CurrentLimit = POWER_CONTROL_DEFAULT_CURRENT;
static float s_PowerLimit = POWER_CONTROL_DEFAULT_POWER;
static float s_VoltageRampStep = POWER_CONTROL_VOLTAGE_RAMP_STEP;
static float s_LastDutyFinal = POWER_CONTROL_DUTY_MIN;
static PowerControlMode_t s_ActiveMode = POWER_CONTROL_MODE_CV;

static float I_cp = 5.0f; // 初始电流上限，单位安培

// //ADC 中断频率极高，必须通过计数器进行“降频抽取”
// static uint8_t decimation_cnt = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static void PID_Init(PID_TypeDef *pid, float kp, float ki, float kd, float out_max, float out_min);
static float Power_Control_Clamp(float value, float min_value, float max_value);
static void Power_Control_ResetState(void);
static void Power_Control_UpdateVoltageRamp(void);
static uint16_t Power_Control_DutyToTicks(float duty);
float Power_Control_GetLastDuty(void);
PowerControlMode_t Power_Control_GetActiveMode(void);
/* USER CODE END PFP */

/* Exported functions --------------------------------------------------------*/
void Power_Control_Init(void)
{
  /* USER CODE BEGIN Power_Control_Init */
  PID_Init(&Voltage_Loop, POWER_PID_VOL_KP, POWER_PID_VOL_KI, POWER_PID_VOL_KD, POWER_CONTROL_DUTY_MAX, POWER_CONTROL_DUTY_MIN);
  PID_Init(&Current_Loop, POWER_PID_CUR_KP, POWER_PID_CUR_KI, POWER_PID_CUR_KD, POWER_CONTROL_DUTY_MAX, POWER_CONTROL_DUTY_MIN);

  Power_Control_ResetState();
  BSP_HRTIM_UpdateDutySymmetric(0U);
  /* USER CODE END Power_Control_Init */
}

/**
 * @brief 电源控制主处理函数
 * 
 * 该函数实现电源的闭环控制逻辑，包括：
 * 1. 获取目标电压和电流的实际值
 * 2. 计算功率限制下的电流上限
 * 3. 更新电压斜坡目标值
 * 4. 分别计算电压环和电流环的PID输出
 * 5. 根据CV(恒压)和CC(恒流)模式选择最终占空比
 * 6. 更新HRTIM的占空比输出
 * 
 * 函数通过比较电压环和电流环的输出来自动切换工作模式：
 * - 当电压环输出 <= 电流环输出时，工作在CV模式
 * - 当电压环输出 > 电流环输出时，工作在CC模式
 * 
 * @note 该函数不接受参数，也不返回值，所有状态通过全局变量维护
 * @retval None
 */
__attribute__((section(".ccmram"))) void Power_Control_Process(void)
{
  /* USER CODE BEGIN Power_Control_Process */
  Drv_LED3_ON(); // 调试用：指示正在执行控制过程

  // Karpathy原则：安全第一 - 待机状态不输出电压
  if (SysData.Flags.bits.Output_En == 0U) {
      // 待机状态：关闭PWM输出，复位PID积分
      BSP_HRTIM_UpdateDutySymmetric(0U);
      Voltage_Loop.Integral = 0.0f;
      Current_Loop.Integral = 0.0f;
      s_TargetVoltageRamp = 0.0f;
      Drv_LED3_OFF();
      return;
  }

  volatile float actual_voltage = Get_VOUT();
  volatile float actual_current = Get_IOUT();
  volatile float voltage_error;
  volatile float current_error;
  volatile float p_voltage;
  volatile float p_current;
  volatile float duty_voltage;
  volatile float duty_current;
  volatile float duty_final;

  s_TargetVoltageFinal = SM_Get_TargetVoltageFinal();
  s_CurrentLimit = SM_Get_CurrentLimit();
  s_PowerLimit = SM_Get_PowerLimit();

  /* 计算基于功率限制的电流上限，并与设定的电流限制取较小值 */
  // 静态计数器，保存在内存中
  static uint8_t decimation_cnt = 0;
  if (++decimation_cnt >= 20) {
  decimation_cnt = 0;
  I_cp = s_PowerLimit / (actual_voltage + 0.1f);
  // 绝对不能忘的防爆盾：强制限制物理最大电流！
  if (I_cp > 5.0f) I_cp = 5.0f;
  }

  s_CurrentLimit = s_CurrentLimit < I_cp ?  s_CurrentLimit : I_cp;


  Power_Control_UpdateVoltageRamp();

  voltage_error = s_TargetVoltageRamp - actual_voltage;
  current_error = s_CurrentLimit - actual_current;

  /* 电压环PID计算 */
  p_voltage = Voltage_Loop.Kp * voltage_error;
  Voltage_Loop.Integral += Voltage_Loop.Ki_dt * voltage_error;
  duty_voltage = p_voltage + Voltage_Loop.Integral;
  duty_voltage = Power_Control_Clamp(duty_voltage, Voltage_Loop.Out_Min, Voltage_Loop.Out_Max);

  /* 电流环PID计算 */
  p_current = Current_Loop.Kp * current_error;
  Current_Loop.Integral += Current_Loop.Ki_dt * current_error;
  duty_current = p_current + Current_Loop.Integral;
  duty_current = Power_Control_Clamp(duty_current, Current_Loop.Out_Min, Current_Loop.Out_Max);

  /* CV/CC模式选择：选择较小的占空比作为最终输出 */
  if (duty_voltage <= duty_current) {
      duty_final = duty_voltage;
      s_ActiveMode = POWER_CONTROL_MODE_CV;
  } else {
      duty_final = duty_current;
      s_ActiveMode = POWER_CONTROL_MODE_CC;
  }

  /* 积分项反向计算，确保输出一致性 */
  Voltage_Loop.Integral = Power_Control_Clamp(duty_final - p_voltage, Voltage_Loop.Out_Min, Voltage_Loop.Out_Max);
  Current_Loop.Integral = Power_Control_Clamp(duty_final - p_current, Current_Loop.Out_Min, Current_Loop.Out_Max);

  s_LastDutyFinal = duty_final;
  BSP_HRTIM_UpdateDutySymmetric(Power_Control_DutyToTicks(duty_final));
  Drv_LED3_OFF(); // 调试用：指示控制过程结束
  /* USER CODE END Power_Control_Process */
}

/* USER CODE BEGIN 1 */
static void PID_Init(PID_TypeDef *pid, float kp, float ki, float kd, float out_max, float out_min)
{
    pid->Kp = kp;
    pid->Ki_dt = ki;
    pid->Kd_div_dt = kd;
    pid->Error_Last = 0.0f;
    pid->Integral = 0.0f;
    pid->Out_Max = out_max;
    pid->Out_Min = out_min;
}

static float Power_Control_Clamp(float value, float min_value, float max_value)
{
    if (value > max_value) {
        return max_value;
    }

    if (value < min_value) {
        return min_value;
    }

    return value;
}

static void Power_Control_ResetState(void)
{
    s_TargetVoltageFinal = POWER_CONTROL_DEFAULT_VOLTAGE;
    s_TargetVoltageRamp = POWER_CONTROL_DEFAULT_VOLTAGE;
    s_CurrentLimit = POWER_CONTROL_DEFAULT_CURRENT;
    s_VoltageRampStep = POWER_CONTROL_VOLTAGE_RAMP_STEP;
    s_LastDutyFinal = POWER_CONTROL_DUTY_MIN;
    s_ActiveMode = POWER_CONTROL_MODE_CV;

    Voltage_Loop.Error_Last = 0.0f;
    Voltage_Loop.Integral = 0.0f;
    Current_Loop.Error_Last = 0.0f;
    Current_Loop.Integral = 0.0f;
}

static void Power_Control_UpdateVoltageRamp(void)
{
    if (s_TargetVoltageRamp < s_TargetVoltageFinal) {
        s_TargetVoltageRamp += s_VoltageRampStep;
        if (s_TargetVoltageRamp > s_TargetVoltageFinal) {
            s_TargetVoltageRamp = s_TargetVoltageFinal;
        }
    } else {
        s_TargetVoltageRamp = s_TargetVoltageFinal;
    }
}

static uint16_t Power_Control_DutyToTicks(float duty)
{
    float limited_duty = Power_Control_Clamp(duty, POWER_CONTROL_DUTY_MIN, POWER_CONTROL_DUTY_MAX);

    return (uint16_t)(limited_duty + 0.5f);
}

float Power_Control_GetLastDuty(void)
{
    return s_LastDutyFinal;
}

PowerControlMode_t Power_Control_GetActiveMode(void)
{
    return s_ActiveMode;
}
/* USER CODE END 1 */
