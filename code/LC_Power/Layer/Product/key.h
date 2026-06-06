/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : key.h
  * @brief          : Pure key interaction layer.
  ******************************************************************************
  * @attention
  *
  * Author: LCYX
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __KEY_H
#define __KEY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "bsp_gpio.h"

#define KEY_DEBOUNCE_TIME       50U
#define KEY_LONG_PRESS_TIME     1000U
#define QUEUE_MAX_SIZE          16U

typedef enum {
    BTN_KEY_SET = 0,
    BTN_KEY_EXIT,
    BTN_KEY_UP,
    BTN_KEY_DOWN,
    BTN_WT_SET,
    BTN_WT_UP,
    BTN_WT_DOWN,
    BTN_NUM_MAX
} BtnId_t;

typedef enum {
    KEY_STATE_IDLE = 0,
    KEY_STATE_DEBOUNCE,
    KEY_STATE_PRESS_DETECT,
    KEY_STATE_LONG_PRESS,
    KEY_STATE_RELEASE_DEBOUNCE
} Key_State_t;

typedef enum {
    BTN_EVENT_NONE = 0,
    BTN_EVENT_SHORT_PRESS,
    BTN_EVENT_LONG_PRESS
} BtnEvent_t;

typedef struct {
    BtnId_t id;
    BtnEvent_t event;
} ButtonMsg_t;

typedef struct {
    ButtonMsg_t buffer[QUEUE_MAX_SIZE];
    uint16_t head;
    uint16_t tail;
} RingQueue_t;

typedef struct {
    BtnId_t id;
    Key_State_t state;
    uint32_t press_timestamp;
    uint8_t (*ReadLevel)(void);
} Button_t;

typedef void (*ActionFunc_t)(void);

typedef struct {
    BtnId_t id;
    BtnEvent_t event;
    ActionFunc_t action;
} KeyMap_t;

void Key_Init(void);
void Key_Process(void);
void Button_Process_Engine(Button_t *btn);
uint8_t Push_Event_To_Queue(BtnId_t id, BtnEvent_t event);
uint8_t Pop_Event_From_Queue(ButtonMsg_t *msg);

#ifdef __cplusplus
}
#endif

#endif /* __KEY_H */

/* Author: LCYX */
