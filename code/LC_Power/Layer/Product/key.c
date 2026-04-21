/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : key.c
  * @brief          : Key module implementation file.
  *                   This file contains key scan and event handling stubs.
  ******************************************************************************
  * @attention
  *
  * Author: LCYX
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "key.h"

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
Button_t g_Buttons[BTN_NUM_MAX] = {
    { .id = BTN_KEY_SET, .state = KEY_STATE_IDLE, .ReadLevel = Drv_Btn_Read_SET1   },
    { .id = BTN_KEY_EXIT, .state = KEY_STATE_IDLE, .ReadLevel = Drv_Btn_Read_EXIT   },
    { .id = BTN_KEY_UP, .state = KEY_STATE_IDLE, .ReadLevel = Drv_Btn_Read_UP1    },
    { .id = BTN_KEY_DOWN, .state = KEY_STATE_IDLE, .ReadLevel = Drv_Btn_Read_DOWN1  },
    { .id = BTN_WT_SET, .state = KEY_STATE_IDLE, .ReadLevel = Drv_Btn_Read_SET2   },
    { .id = BTN_WT_UP, .state = KEY_STATE_IDLE, .ReadLevel = Drv_Btn_Read_UP2    },
    { .id = BTN_WT_DOWN, .state = KEY_STATE_IDLE, .ReadLevel = Drv_Btn_Read_DOWN2  }
};

const KeyMap_t MainPage_KeyMap[] = {
    //  按键ID          按键动作                 执行什么函数
    { BTN_KEY_SET,   BTN_EVENT_SHORT_PRESS,  Drv_LED0_ON },
    { BTN_KEY_SET,   BTN_EVENT_LONG_PRESS,   Drv_LED0_OFF },
    { BTN_KEY_EXIT,  BTN_EVENT_SHORT_PRESS,  Drv_LED1_ON },
    { BTN_KEY_EXIT,  BTN_EVENT_LONG_PRESS,   Drv_LED1_OFF },
    { BTN_KEY_UP,    BTN_EVENT_SHORT_PRESS,  Drv_LED2_ON },
    { BTN_KEY_UP,    BTN_EVENT_LONG_PRESS,   Drv_LED2_OFF },
    { BTN_KEY_DOWN,  BTN_EVENT_SHORT_PRESS,  Drv_LED3_ON },
    { BTN_KEY_DOWN,  BTN_EVENT_LONG_PRESS,   Drv_LED3_OFF },
    { BTN_WT_SET,    BTN_EVENT_SHORT_PRESS,  Drv_LED0_ON },
    { BTN_WT_SET,    BTN_EVENT_LONG_PRESS,   Drv_LED0_OFF },
    { BTN_WT_UP,     BTN_EVENT_SHORT_PRESS,  Drv_LED2_ON },
    { BTN_WT_UP,     BTN_EVENT_LONG_PRESS,   Drv_LED2_OFF },
    { BTN_WT_DOWN,   BTN_EVENT_SHORT_PRESS,  Drv_LED3_ON },
    { BTN_WT_DOWN,   BTN_EVENT_LONG_PRESS,   Drv_LED3_OFF },

    // 后续增加任何功能，只需要在这里加一行，根本不需要改逻辑代码！
};

// 计算表中有多少个映射关系
#define MAIN_PAGE_MAP_SIZE (sizeof(MainPage_KeyMap) / sizeof(KeyMap_t))


//全局的按键队列
RingQueue_t g_BtnQueue = { .head = 0, .tail = 0 };
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Exported functions --------------------------------------------------------*/
void Key_Init(void)
{
  /* USER CODE BEGIN Key_Init */
 
  
  /* USER CODE END Key_Init */
}

void Key_Process(void)
{
    for (int i = 0; i < BTN_NUM_MAX; i++) {
        Button_Process_Engine(&g_Buttons[i]); // 驱动所有按键
    }

  /* USER CODE BEGIN Key_Process */
  ButtonMsg_t msg;
  // 如果队列里有按键消息，就取出来处理
    if (Pop_Event_From_Queue(&msg)) 
    {
        // 遍历当前页面的映射表，一旦 ID 和 Event 对上了，直接开枪（执行函数）！
        for (int i = 0; i < MAIN_PAGE_MAP_SIZE; i++) 
        {
            if (msg.id == MainPage_KeyMap[i].id && msg.event == MainPage_KeyMap[i].event) 
            {
               MainPage_KeyMap[i].action(); // 执行动作
               break;
          }
        }
    }
  /* USER CODE END Key_Process */
}

// 这是一个公共的状态机，它只认传入的指针 btn，不关心具体是哪个按键
void Button_Process_Engine(Button_t *btn) 
{
    uint8_t current_level = btn->ReadLevel(); // 调用虚函数读取电平

    switch (btn->state) {
        case KEY_STATE_IDLE:
            if (current_level == PIN_LOW) 
            {
                btn->state = KEY_STATE_DEBOUNCE;
                btn->press_timestamp = g_Ticks[TICK_MS].Tick(); // 记录自己的时间戳
            }
            break;

        case KEY_STATE_DEBOUNCE:
            if ((g_Ticks[TICK_MS].Tick() - btn->press_timestamp) >= KEY_DEBOUNCE_TIME)
            {
                if (current_level == PIN_LOW)
                {
                    btn->state = KEY_STATE_PRESS_DETECT;
                }
                else 
                {
                    btn->state = KEY_STATE_IDLE; // 误触，回到空闲
                }
            }
            
            break;
            
        case KEY_STATE_PRESS_DETECT:
             // 重点：判断出长按或短按后，不要在这里直接调用函数！
             // 而是生成一个“事件”推送到队列里。
             if (current_level == PIN_LOW) 
             {
                if ((g_Ticks[TICK_MS].Tick() - btn->press_timestamp) >= KEY_LONG_PRESS_TIME) 
                {
                    btn->state = KEY_STATE_LONG_PRESS;
                }
             }
             else 
             {
                 Push_Event_To_Queue(btn->id, BTN_EVENT_SHORT_PRESS);
                 btn->state = KEY_STATE_IDLE;
             }
             break;
        
        case KEY_STATE_LONG_PRESS:
            if (current_level == PIN_LOW) 
            {
                Push_Event_To_Queue(btn->id, BTN_EVENT_LONG_PRESS);
                btn->press_timestamp = g_Ticks[TICK_MS].Tick(); // 记录自己的时间戳
                btn->state = KEY_STATE_RELEASE_DEBOUNCE;
            }
            break;       
        
        case KEY_STATE_RELEASE_DEBOUNCE:
            if ((g_Ticks[TICK_MS].Tick() - btn->press_timestamp) >= KEY_DEBOUNCE_TIME) 
            {
                if (current_level == PIN_HIGH) 
                {
                    btn->state = KEY_STATE_IDLE;
                }
            }  
    }
}


/**
 * @brief  将按键事件推入队列 (由 10ms 按键状态机调用)
 * @param  id: 按键ID
 * @param  event: 事件类型
 * @retval 1:成功，0:队列已满失败
 */
uint8_t Push_Event_To_Queue(BtnId_t id, BtnEvent_t event) {
    // 预测下一个写指针的位置
    uint16_t next_head = (g_BtnQueue.head + 1) % QUEUE_MAX_SIZE;
    
    // 如果写指针追上了读指针，说明队列满了，为了安全丢弃该事件
    if (next_head == g_BtnQueue.tail) {
        return 0; // Queue Full Warning!
    }
    
    // 把消息装进当前的头部槽位
    g_BtnQueue.buffer[g_BtnQueue.head].id = id;
    g_BtnQueue.buffer[g_BtnQueue.head].event = event;
    
    // 头指针往前走一步
    g_BtnQueue.head = next_head; 
    
    return 1; // 成功推入
}

/**
 * @brief  从队列中取出按键事件 (由 50ms UI 任务调用)
 * @param  pMsg: 传入一个空的结构体指针，用于接收取出的消息
 * @retval 1:成功取出，0:队列为空
 */
uint8_t Pop_Event_From_Queue(ButtonMsg_t *pMsg) {
    // 如果头指针等于尾指针，说明里面没东西
    if (g_BtnQueue.head == g_BtnQueue.tail) {
        return 0; // Queue Empty
    }
    
    // 把尾部槽位的消息拷贝给调用者
    pMsg->id    = g_BtnQueue.buffer[g_BtnQueue.tail].id;
    pMsg->event = g_BtnQueue.buffer[g_BtnQueue.tail].event;
    
    // 尾指针往前走一步（吃掉一个数据）
    g_BtnQueue.tail = (g_BtnQueue.tail + 1) % QUEUE_MAX_SIZE;
    
    return 1; // 成功取出
}
