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
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define POWER_CONTROL_DUTY_MAX          ((float)BSP_HRTIM_DUTY_MAX)
#define POWER_CONTROL_DUTY_MIN          (0.0f)
#define POWER_CONTROL_DEFAULT_VOLTAGE   (0.0f)
#define POWER_CONTROL_DEFAULT_CURRENT   (0.0f)
#define POWER_CONTROL_VOLTAGE_RAMP_STEP (0.1f)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
PID_TypeDef Voltage_Loop __attribute__((at(0x10000000)));
PID_TypeDef Current_Loop __attribute__((at(0x10000040)));

static float s_TargetVoltageFinal = POWER_CONTROL_DEFAULT_VOLTAGE;
static float s_TargetVoltageRamp = POWER_CONTROL_DEFAULT_VOLTAGE;
static float s_CurrentLimit = POWER_CONTROL_DEFAULT_CURRENT;
static float s_VoltageRampStep = POWER_CONTROL_VOLTAGE_RAMP_STEP;
static float s_LastDutyFinal = POWER_CONTROL_DUTY_MIN;
static PowerControlMode_t s_ActiveMode = POWER_CONTROL_MODE_CV;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static void PID_Init(PID_TypeDef *pid, float kp, float ki, float kd, float out_max, float out_min);
static float Power_Control_Clamp(float value, float min_value, float max_value);
static void Power_Control_ResetState(void);
static void Power_Control_UpdateVoltageRamp(void);
static uint16_t Power_Control_DutyToTicks(float duty);
/* USER CODE END PFP */

/* Exported functions --------------------------------------------------------*/
void Power_Control_Init(void)
{
  /* USER CODE BEGIN Power_Control_Init */
  PID_Init(&Voltage_Loop, 0.1f, 0.0f, 0.0f, POWER_CONTROL_DUTY_MAX, POWER_CONTROL_DUTY_MIN);
  PID_Init(&Current_Loop, 0.2f, 0.0f, 0.0f, POWER_CONTROL_DUTY_MAX, POWER_CONTROL_DUTY_MIN);

  Power_Control_ResetState();
  BSP_HRTIM_UpdateDutySymmetric(0U);
  /* USER CODE END Power_Control_Init */
}

void Power_Control_Process(void)
{
  /* USER CODE BEGIN Power_Control_Process */
  float actual_voltage = Get_VOUT();
  float actual_current = Get_IOUT();
  float voltage_error;
  float current_error;
  float p_voltage;
  float p_current;
  float duty_voltage;
  float duty_current;
  float duty_final;

  Power_Control_UpdateVoltageRamp();

  voltage_error = s_TargetVoltageRamp - actual_voltage;
  current_error = s_CurrentLimit - actual_current;

  p_voltage = Voltage_Loop.Kp * voltage_error;
  Voltage_Loop.Integral += Voltage_Loop.Ki_dt * voltage_error;
  duty_voltage = p_voltage + Voltage_Loop.Integral;
  duty_voltage = Power_Control_Clamp(duty_voltage, Voltage_Loop.Out_Min, Voltage_Loop.Out_Max);

  p_current = Current_Loop.Kp * current_error;
  Current_Loop.Integral += Current_Loop.Ki_dt * current_error;
  duty_current = p_current + Current_Loop.Integral;
  duty_current = Power_Control_Clamp(duty_current, Current_Loop.Out_Min, Current_Loop.Out_Max);

  if (duty_voltage <= duty_current) {
      duty_final = duty_voltage;
      s_ActiveMode = POWER_CONTROL_MODE_CV;
  } else {
      duty_final = duty_current;
      s_ActiveMode = POWER_CONTROL_MODE_CC;
  }

  Voltage_Loop.Integral = Power_Control_Clamp(duty_final - p_voltage, Voltage_Loop.Out_Min, Voltage_Loop.Out_Max);
  Current_Loop.Integral = Power_Control_Clamp(duty_final - p_current, Current_Loop.Out_Min, Current_Loop.Out_Max);

  s_LastDutyFinal = duty_final;
  BSP_HRTIM_UpdateDutySymmetric(Power_Control_DutyToTicks(duty_final));
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
/* USER CODE END 1 */
