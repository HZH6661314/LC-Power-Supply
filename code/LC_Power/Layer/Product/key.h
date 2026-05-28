/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : key.h
  * @brief          : Header for key.c file.
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
#ifndef __KEY_H
#define __KEY_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "state_machine.h"
#include "bsp_gpio.h"
#include "bsp_hrtim.h"
/* USER CODE END Includes */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */
#define KEY_DEBOUNCE_TIME       50   // 50ms 消抖时间
#define KEY_LONG_PRESS_TIME     1000 // 1000ms 长按判定

/* 定义队列的最大容量（必须是确定的常数，工程上常用 2 的幂次方）*/
#define QUEUE_MAX_SIZE 16

#define POWER_VOL_MAX_LIMIT               (48.0f) // 物理最大电压限制，单位伏特
#define POWER_VOL_MIN_LIMIT               (0.0f) // 物理最小电压限制，单位伏特
#define POWER_CUR_MAX_LIMIT               (5.0f) // 物理最大电流限制，单位安培
#define POWER_CUR_MIN_LIMIT               (0.0f)
/* USER CODE END Private defines */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
/* 按键 ID 枚举 */
typedef enum {
    BTN_KEY_SET = 0,
    BTN_KEY_EXIT = 1,
    BTN_KEY_UP = 2,
    BTN_KEY_DOWN = 3,
    BTN_WT_SET = 4,
    BTN_WT_UP = 5,
    BTN_WT_DOWN = 6,
    BTN_NUM_MAX = 7,
} BtnId_t;

typedef enum {
    KEY_STATE_IDLE = 0,          // 空闲
    KEY_STATE_DEBOUNCE,          // 按键消抖
    KEY_STATE_PRESS_DETECT,      // 按键按下检测
    KEY_STATE_LONG_PRESS,        // 长按检测
    KEY_STATE_RELEASE_DEBOUNCE,  // 按键释放消抖         
} Key_State_t;

// 定义按键事件枚举
typedef enum {
    BTN_EVENT_NONE = 0,     // 无事件
    BTN_EVENT_SHORT_PRESS,  // 短按
    BTN_EVENT_LONG_PRESS,   // 长按
    BTN_EVENT_DOUBLE_CLICK, //双击
    // 可以继续扩展双击、长按释放等
} BtnEvent_t;

/* 消息体：装载“是谁”和“怎么了” */
typedef struct {
    BtnId_t    id;       // 哪个按键
    BtnEvent_t event;    // 发生了什么动作
} ButtonMsg_t;

/* 环形队列管理器 */
typedef struct {
    ButtonMsg_t buffer[QUEUE_MAX_SIZE]; // 物理内存数组
    uint16_t    head;                   // 写指针（入队位置）
    uint16_t    tail;                   // 读指针（出队位置）
} RingQueue_t;

// 按键对象结构体（核心！）
typedef struct {
    BtnId_t  id;                 // 按键的唯一标识（如 BTN_UP, BTN_DOWN）
    Key_State_t  state;              // 当前状态机节点（私有状态）
    uint32_t press_timestamp;    // 按下的时间戳（私有计时）
    uint8_t  (*ReadLevel)(void); // 虚函数：指向底层硬件读取函数
} Button_t;

// 定义一个统一格式的动作函数指针（不带参数，不带返回值）
typedef void (*ActionFunc_t)(void);

// 定义按键映射条目
typedef struct {
    BtnId_t      id;       // 按键 ID
    BtnEvent_t   event;    // 按键事件
    ActionFunc_t action;   // 对应的执行函数（虚函数再次立功！）
} KeyMap_t;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Key_Init(void);
void Key_Process(void);

void Button_Process_Engine(Button_t *btn);
uint8_t Push_Event_To_Queue(BtnId_t id, BtnEvent_t event);
uint8_t Pop_Event_From_Queue(ButtonMsg_t *msg);

float Get_TargetVoltageFinal(void);
float Get_CurrentLimit(void);
float Get_PowerLimit(void);

void TargetVoltageFinal_ADD(void);
void TargetVoltageFinal_SUB(void);
/* USER CODE BEGIN EFP */

/* USER CODE END EFP */



#ifdef __cplusplus
}
#endif

#endif /* __KEY_H */

/* Author: LCYX */
