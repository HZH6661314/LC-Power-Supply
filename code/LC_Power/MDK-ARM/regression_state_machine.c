#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../Layer/Application/state_machine.h"
#include "../Layer/Control/power_control.h"

static float s_vout = 0.0f;
static float s_iout = 0.0f;
static PowerControlMode_t s_mode = POWER_CONTROL_MODE_CV;

float Get_VOUT(void)
{
    return s_vout;
}

float Get_IOUT(void)
{
    return s_iout;
}

PowerControlMode_t Power_Control_GetActiveMode(void)
{
    return s_mode;
}

static void expect_focus(SM_Focus_t focus, const char *label)
{
    if (SM_Get_Focus() != focus) {
        fprintf(stderr, "unexpected focus after %s\n", label);
        exit(1);
    }
}

static void expect_range(float value, float min_value, float max_value, const char *label)
{
    if ((value < min_value) || (value > max_value)) {
        fprintf(stderr, "%s out of range: %.3f\n", label, value);
        exit(1);
    }
}

int main(void)
{
    int i;

    State_Machine_Init();

    if (SM_Get_UI_State() != UI_STATE_STANDBY) {
        fprintf(stderr, "initial ui state wrong\n");
        return 1;
    }

    SM_Action_Enter();
    if (SM_Get_UI_State() != UI_STATE_MENU) {
        fprintf(stderr, "menu enter failed\n");
        return 1;
    }
    expect_focus(SM_FOCUS_SET_VOLTAGE, "enter");

    SM_Action_Down();
    expect_focus(SM_FOCUS_SET_CURRENT, "down1");
    SM_Action_Down();
    expect_focus(SM_FOCUS_QUICK_SET, "down2");
    SM_Action_Down();
    expect_focus(SM_FOCUS_SETTINGS, "down3");
    SM_Action_Down();
    expect_focus(SM_FOCUS_SET_VOLTAGE, "down wrap");

    SM_Action_Enter();
    if (SM_Get_UI_State() != UI_STATE_ADJUST) {
        fprintf(stderr, "adjust enter failed\n");
        return 1;
    }

    for (i = 0; i < 200; ++i) {
        SM_Action_Up();
    }
    expect_range(SM_Get_TargetVoltageFinal(), 0.0f, 48.0f, "voltage");

    SM_Action_Exit_Short();
    SM_Action_Down();
    if (SM_Get_Focus() != SM_FOCUS_SET_CURRENT) {
        fprintf(stderr, "focus current failed\n");
        return 1;
    }
    SM_Action_Enter();

    for (i = 0; i < 200; ++i) {
        SM_Action_Up();
    }
    expect_range(SM_Get_CurrentLimit(), 0.0f, 5.0f, "current");

    if ((SM_Get_TargetVoltageFinal() * SM_Get_CurrentLimit()) > SM_Get_PowerLimit() + 0.05f) {
        fprintf(stderr, "power shield failed: %.3fW\n",
                SM_Get_TargetVoltageFinal() * SM_Get_CurrentLimit());
        return 1;
    }

    s_vout = 12.5f;
    s_iout = 1.25f;
    s_mode = POWER_CONTROL_MODE_CC;
    StateMachine_Task();

    if (SM_Get_MeasuredVoltage() != 12.5f) {
        fprintf(stderr, "measured voltage getter failed\n");
        return 1;
    }
    if (SM_Get_MeasuredCurrent() != 1.25f) {
        fprintf(stderr, "measured current getter failed\n");
        return 1;
    }
    if (SM_Get_CCMode() != 1U) {
        fprintf(stderr, "cc mode getter failed\n");
        return 1;
    }
    if (SM_Get_TemperatureC() != 36U) {
        fprintf(stderr, "temperature getter failed\n");
        return 1;
    }

    SM_Action_Exit_Long();
    if (SM_Get_UI_State() != UI_STATE_STANDBY) {
        fprintf(stderr, "long exit failed\n");
        return 1;
    }
    expect_focus(SM_FOCUS_SET_VOLTAGE, "long exit");

    printf("state_machine regression ok\n");
    return 0;
}
