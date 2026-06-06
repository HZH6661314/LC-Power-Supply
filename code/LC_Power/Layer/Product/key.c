/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : key.c
  * @brief          : Pure key scan, debounce, event queue and dispatch module.
  ******************************************************************************
  * @attention
  *
  * Author: LCYX
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#include "key.h"
#include "state_machine.h"
#include "bsp_hrtim.h"

static Button_t g_Buttons[BTN_NUM_MAX] = {
    { BTN_KEY_SET,  KEY_STATE_IDLE, 0U, Drv_Btn_Read_SET1  },
    { BTN_KEY_EXIT, KEY_STATE_IDLE, 0U, Drv_Btn_Read_EXIT  },
    { BTN_KEY_UP,   KEY_STATE_IDLE, 0U, Drv_Btn_Read_UP1   },
    { BTN_KEY_DOWN, KEY_STATE_IDLE, 0U, Drv_Btn_Read_DOWN1 },
    { BTN_WT_SET,   KEY_STATE_IDLE, 0U, Drv_Btn_Read_SET2  },
    { BTN_WT_UP,    KEY_STATE_IDLE, 0U, Drv_Btn_Read_UP2   },
    { BTN_WT_DOWN,  KEY_STATE_IDLE, 0U, Drv_Btn_Read_DOWN2 }
};

static const KeyMap_t MainPage_KeyMap[] = {
    { BTN_KEY_SET,  BTN_EVENT_SHORT_PRESS, SM_Action_Enter      },
    { BTN_KEY_SET,  BTN_EVENT_LONG_PRESS,  SM_Action_Enter      },
    { BTN_WT_SET,   BTN_EVENT_SHORT_PRESS, SM_Action_Enter      },
    { BTN_WT_SET,   BTN_EVENT_LONG_PRESS,  SM_Action_Enter      },
    { BTN_KEY_EXIT, BTN_EVENT_SHORT_PRESS, SM_Action_Exit_Short },
    { BTN_KEY_EXIT, BTN_EVENT_LONG_PRESS,  SM_Action_Exit_Long  },
    { BTN_KEY_UP,   BTN_EVENT_SHORT_PRESS, SM_Action_Up         },
    { BTN_KEY_UP,   BTN_EVENT_LONG_PRESS,  SM_Action_Up         },
    { BTN_WT_UP,    BTN_EVENT_SHORT_PRESS, SM_Action_Up         },
    { BTN_WT_UP,    BTN_EVENT_LONG_PRESS,  SM_Action_Up         },
    { BTN_KEY_DOWN, BTN_EVENT_SHORT_PRESS, SM_Action_Down       },
    { BTN_KEY_DOWN, BTN_EVENT_LONG_PRESS,  SM_Action_Down       },
    { BTN_WT_DOWN,  BTN_EVENT_SHORT_PRESS, SM_Action_Down       },
    { BTN_WT_DOWN,  BTN_EVENT_LONG_PRESS,  SM_Action_Down       }
};

#define MAIN_PAGE_MAP_SIZE (sizeof(MainPage_KeyMap) / sizeof(MainPage_KeyMap[0]))

static RingQueue_t g_BtnQueue;

void Key_Init(void)
{
    uint8_t i;

    g_BtnQueue.head = 0U;
    g_BtnQueue.tail = 0U;

    for (i = 0U; i < (uint8_t)BTN_NUM_MAX; ++i) {
        g_Buttons[i].state = KEY_STATE_IDLE;
        g_Buttons[i].press_timestamp = 0U;
    }
}

void Key_Process(void)
{
    uint8_t i;
    ButtonMsg_t msg;

    for (i = 0U; i < (uint8_t)BTN_NUM_MAX; ++i) {
        Button_Process_Engine(&g_Buttons[i]);
    }

    while (Pop_Event_From_Queue(&msg) != 0U) {
        uint16_t map_index;

        for (map_index = 0U; map_index < (uint16_t)MAIN_PAGE_MAP_SIZE; ++map_index) {
            if ((msg.id == MainPage_KeyMap[map_index].id) &&
                (msg.event == MainPage_KeyMap[map_index].event)) {
                if (MainPage_KeyMap[map_index].action != 0) {
                    MainPage_KeyMap[map_index].action();
                }
                break;
            }
        }
    }
}

void Button_Process_Engine(Button_t *btn)
{
    uint8_t current_level;
    uint32_t now_ms;

    if ((btn == 0) || (btn->ReadLevel == 0)) {
        return;
    }

    current_level = btn->ReadLevel();
    now_ms = g_Ticks[TICK_MS].Tick();
	
	

    switch (btn->state) {
    case KEY_STATE_IDLE:
        if (current_level == PIN_LOW) {
            btn->state = KEY_STATE_DEBOUNCE;
            btn->press_timestamp = now_ms;
        }
        break;

    case KEY_STATE_DEBOUNCE:
        if ((uint32_t)(now_ms - btn->press_timestamp) >= KEY_DEBOUNCE_TIME) {
            btn->state = (current_level == PIN_LOW) ? KEY_STATE_PRESS_DETECT : KEY_STATE_IDLE;
        }
        break;

    case KEY_STATE_PRESS_DETECT:
        if (current_level == PIN_LOW) {
            if ((uint32_t)(now_ms - btn->press_timestamp) >= KEY_LONG_PRESS_TIME) {
                Push_Event_To_Queue(btn->id, BTN_EVENT_LONG_PRESS);
                btn->press_timestamp = now_ms;
                btn->state = KEY_STATE_LONG_PRESS;
            }
        } else {
            Push_Event_To_Queue(btn->id, BTN_EVENT_SHORT_PRESS);
            btn->press_timestamp = now_ms;
            btn->state = KEY_STATE_RELEASE_DEBOUNCE;
        }
        break;

    case KEY_STATE_LONG_PRESS:
        if (current_level == PIN_HIGH) {
            btn->press_timestamp = now_ms;
            btn->state = KEY_STATE_RELEASE_DEBOUNCE;
        }
        break;

    case KEY_STATE_RELEASE_DEBOUNCE:
        // Karpathy原则：防御性编程 - 处理按键抖动和快速重按

        // 检测新的按键按下（双击/快速重按）
        if (current_level == PIN_LOW) {
            btn->state = KEY_STATE_DEBOUNCE;
            btn->press_timestamp = now_ms;
            break;
        }

        // 超时强制回IDLE（不依赖current_level读取，防止卡死）
        if ((uint32_t)(now_ms - btn->press_timestamp) >= KEY_DEBOUNCE_TIME) {
            btn->state = KEY_STATE_IDLE;
        }
        break;

    default:
        btn->state = KEY_STATE_IDLE;
        break;
    }
}

uint8_t Push_Event_To_Queue(BtnId_t id, BtnEvent_t event)
{
    uint16_t next_head = (uint16_t)((g_BtnQueue.head + 1U) % QUEUE_MAX_SIZE);

    if (next_head == g_BtnQueue.tail) {
        return 0U;
    }

    // 🔍 调试：LED2闪烁表示事件入队（已禁用以恢复性能）
    // Drv_LED2_Toggle();

    g_BtnQueue.buffer[g_BtnQueue.head].id = id;
    g_BtnQueue.buffer[g_BtnQueue.head].event = event;
    g_BtnQueue.head = next_head;


    return 1U;
}

uint8_t Pop_Event_From_Queue(ButtonMsg_t *msg)
{
    if (msg == 0) {
        return 0U;
    }

    if (g_BtnQueue.head == g_BtnQueue.tail) {
        return 0U;
    }

    *msg = g_BtnQueue.buffer[g_BtnQueue.tail];
    g_BtnQueue.tail = (uint16_t)((g_BtnQueue.tail + 1U) % QUEUE_MAX_SIZE);

    return 1U;
}
