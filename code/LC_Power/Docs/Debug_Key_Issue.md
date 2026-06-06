# 按键交互问题排查指南

## 问题描述
按下UP键时，光标没有出现，主界面无反应。

## 可能的原因分析

### 1. 按键硬件未正确读取 ⚠️
**检查方法**:
- 在`Button_Process_Engine()`中添加调试输出
- 检查`current_level`是否正确读取到`PIN_LOW`

**调试代码**:
```c
// 在 key.c 的 Button_Process_Engine() 中添加
void Button_Process_Engine(Button_t *btn)
{
    uint8_t current_level;
    
    if ((btn == 0) || (btn->ReadLevel == 0)) {
        return;
    }
    
    current_level = btn->ReadLevel();
    
    // 调试输出（可以通过LED或串口）
    if (current_level == PIN_LOW) {
        // 按键被按下
        Drv_LED0_Toggle();  // LED闪烁表示按键有响应
    }
}
```

---

### 2. 按键事件未入队 ⚠️
**检查方法**:
- 检查`Push_Event_To_Queue()`是否成功
- 检查队列是否满了

**调试代码**:
```c
// 在 key.c 的 Push_Event_To_Queue() 中添加
uint8_t Push_Event_To_Queue(BtnId_t id, BtnEvent_t event)
{
    uint16_t next_head = (uint16_t)((g_BtnQueue.head + 1U) % QUEUE_MAX_SIZE);
    
    if (next_head == g_BtnQueue.tail) {
        // 队列满了！
        Drv_LED1_ON();  // 点亮LED1表示队列满
        return 0U;
    }
    
    g_BtnQueue.buffer[g_BtnQueue.head].id = id;
    g_BtnQueue.buffer[g_BtnQueue.head].event = event;
    g_BtnQueue.head = next_head;
    
    // 调试输出
    Drv_LED2_Toggle();  // LED闪烁表示事件入队成功
    
    return 1U;
}
```

---

### 3. 按键映射未匹配 ⚠️
**检查方法**:
- 确认按键ID是否正确
- 检查映射表是否被正确遍历

**调试代码**:
```c
// 在 key.c 的 Key_Process() 中添加
void Key_Process(void)
{
    uint8_t i;
    ButtonMsg_t msg;
    
    for (i = 0U; i < (uint8_t)BTN_NUM_MAX; ++i) {
        Button_Process_Engine(&g_Buttons[i]);
    }
    
    while (Pop_Event_From_Queue(&msg) != 0U) {
        uint16_t map_index;
        uint8_t matched = 0U;
        
        for (map_index = 0U; map_index < (uint16_t)MAIN_PAGE_MAP_SIZE; ++map_index) {
            if ((msg.id == MainPage_KeyMap[map_index].id) &&
                (msg.event == MainPage_KeyMap[map_index].event)) {
                
                matched = 1U;
                Drv_LED3_Toggle();  // LED闪烁表示找到匹配
                
                if (MainPage_KeyMap[map_index].action != 0) {
                    MainPage_KeyMap[map_index].action();
                }
                break;
            }
        }
        
        if (matched == 0U) {
            // 未找到匹配的映射！
            Drv_LED1_ON();  // 点亮LED1表示映射失败
        }
    }
}
```

---

### 4. 状态机未正确切换 ⚠️
**检查方法**:
- 确认`SM_Action_Up()`是否被调用
- 确认`s_ui_state`是否从`UI_STATE_HOME_IDLE`变为`UI_STATE_HOME_MENU`

**调试代码**:
```c
// 在 state_machine.c 的 SM_Action_Up() 中添加
void SM_Action_Up(void)
{
    // HOME_IDLE: UP按键唤醒光标，进入MENU状态
    if (s_ui_state == UI_STATE_HOME_IDLE) {
        Drv_LED0_ON();  // 点亮LED0表示进入此分支
        
        s_ui_state = UI_STATE_HOME_MENU;
        s_focus = SM_FOCUS_SET_VOLTAGE;
        
        Drv_LED1_ON();  // 点亮LED1表示状态已切换
        return;
    }
    
    // ... 其他代码
}
```

---

### 5. UI层未正确绘制光标 ⚠️
**检查方法**:
- 确认`UI_Display_Process()`是否检测到状态变化
- 确认`UI_DrawFocus()`是否被调用

**调试代码**:
```c
// 在 ui_display.c 的 UI_Display_Process() 中添加
void UI_Display_Process(void)
{
    // ... 前面的代码
    
    ui_state = SM_Get_UI_State();
    focus = SM_Get_Focus();
    
    // 调试输出：状态变化检测
    if (s_cache.ui_state != ui_state) {
        Drv_LED2_Toggle();  // LED闪烁表示检测到状态变化
    }
    
    // 状态或焦点变化时重绘光标
    if ((s_cache.ui_state != ui_state) || (s_cache.focus != focus)) {
        UI_ClearFocus();
        
        Drv_LED3_ON();  // 点亮LED3表示准备绘制光标
        
        UI_DrawFocus(ui_state, focus);
        
        s_cache.ui_state = ui_state;
        s_cache.focus = focus;
    }
}
```

---

## 快速排查步骤

### 步骤1: 验证按键硬件
1. 添加LED调试代码到`Button_Process_Engine()`
2. 按下UP键，观察LED0是否闪烁
3. **如果不闪烁** → 按键硬件或驱动有问题

### 步骤2: 验证事件入队
1. 添加LED调试代码到`Push_Event_To_Queue()`
2. 按下UP键，观察LED2是否闪烁
3. **如果不闪烁** → 按键状态机未正确工作

### 步骤3: 验证按键映射
1. 添加LED调试代码到`Key_Process()`
2. 按下UP键，观察LED3是否闪烁
3. **如果不闪烁** → 按键映射表有问题

### 步骤4: 验证状态切换
1. 添加LED调试代码到`SM_Action_Up()`
2. 按下UP键，观察LED0和LED1是否点亮
3. **如果不点亮** → 状态机函数未被调用

### 步骤5: 验证UI绘制
1. 添加LED调试代码到`UI_Display_Process()`
2. 按下UP键，观察LED2和LED3是否有响应
3. **如果不响应** → UI层未检测到状态变化

---

## 常见问题与解决方案

### 问题1: 按键硬件不响应
**原因**: GPIO引脚未正确配置或硬件连接问题

**解决**:
```c
// 检查 bsp_gpio.c 中的按键初始化
void BSP_GPIO_Init(void)
{
    // 确保UP按键GPIO已初始化为输入+上拉
    // STM32 HAL示例:
    // GPIO_InitStruct.Pin = BTN_UP_Pin;
    // GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    // GPIO_InitStruct.Pull = GPIO_PULLUP;
    // HAL_GPIO_Init(BTN_UP_GPIO_Port, &GPIO_InitStruct);
}
```

---

### 问题2: 按键消抖时间过长
**原因**: `KEY_DEBOUNCE_TIME`设置过大

**解决**:
```c
// 在 key.h 中检查
#define KEY_DEBOUNCE_TIME       50U  // 建议值：30-50ms

// 如果设置为500U或更大，会导致按键响应迟钝
```

---

### 问题3: UI刷新频率过低
**原因**: `UI_Display_Process()`调用频率<20Hz

**解决**:
```c
// 在 task_manager.c 中检查
void SysCore_Run(void)
{
    // ...
    
    if ((uint32_t)(currentTick - s_LastTick_50ms) >= 50U) {
        s_LastTick_50ms = currentTick;
        if (s_UI_Initialized != 0U) {
            UI_Display_Process();  // 确保这里被调用
        }
    }
}
```

---

### 问题4: 状态机初始状态错误
**原因**: 初始化时状态不是`UI_STATE_HOME_IDLE`

**解决**:
```c
// 在 state_machine.c 的 State_Machine_Init() 中检查
void State_Machine_Init(void)
{
    // ...
    s_ui_state = UI_STATE_HOME_IDLE;  // 确保初始状态正确
    s_focus = SM_FOCUS_SET_VOLTAGE;
    // ...
}
```

---

### 问题5: UI_DrawFocus未正确实现
**原因**: 绘制函数逻辑错误或TFT驱动问题

**解决**:
```c
// 在 ui_display.c 的 UI_DrawFocus() 中添加测试代码
static void UI_DrawFocus(UI_State_t ui_state, SM_Focus_t focus)
{
    int16_t x, y, w, h;
    
    if (ui_state == UI_STATE_HOME_IDLE) {
        return;  // IDLE状态不绘制光标
    }
    
    // 强制测试：绘制一个明显的矩形
    TFTGFX_DrawRect(140, 34, 98, 52, TFT_COLOR_RED);
    TFTGFX_DrawRect(141, 35, 96, 50, TFT_COLOR_RED);
    
    // 如果能看到红色矩形，说明TFT驱动正常
    // 如果看不到，说明TFT绘制有问题
}
```

---

## 使用串口调试（推荐）

如果项目已配置串口，建议使用串口输出调试：

```c
// 在 state_machine.c 中
void SM_Action_Up(void)
{
    printf("SM_Action_Up called, current state = %d\r\n", s_ui_state);
    
    if (s_ui_state == UI_STATE_HOME_IDLE) {
        printf("Switching to UI_STATE_HOME_MENU\r\n");
        s_ui_state = UI_STATE_HOME_MENU;
        s_focus = SM_FOCUS_SET_VOLTAGE;
        return;
    }
    
    // ...
}

// 在 ui_display.c 中
void UI_Display_Process(void)
{
    // ...
    ui_state = SM_Get_UI_State();
    
    if (s_cache.ui_state != ui_state) {
        printf("UI state changed: %d -> %d\r\n", s_cache.ui_state, ui_state);
    }
    
    // ...
}
```

---

## 检查清单

在提交Bug报告前，请确认以下各项：

- [ ] 按键硬件连接正常
- [ ] GPIO引脚配置为输入+上拉模式
- [ ] `Key_Process()`每10ms被调用
- [ ] `UI_Display_Process()`每50ms被调用
- [ ] `State_Machine_Init()`已在启动时调用
- [ ] `UI_Display_Init()`已在TFT初始化后调用
- [ ] 按键消抖时间设置合理（30-50ms）
- [ ] 事件队列未满（`QUEUE_MAX_SIZE = 16`）
- [ ] TFT绘制函数正常工作

---

## 我的建议

根据您描述的"按下UP键时连光标都没有"，最可能的原因是：

### 可能性1: 按键硬件未响应（概率60%）
- 检查GPIO配置
- 检查硬件连接
- 用万用表测试按键是否短路

### 可能性2: 调度器未运行（概率30%）
- `Key_Process()`未被定时调用
- `UI_Display_Process()`未被定时调用

### 可能性3: TFT绘制问题（概率10%）
- `UI_DrawFocus()`内部的`TFTGFX_DrawRect()`未正确绘制

---

## 快速验证方案

**最简单的验证方法**：在`SM_Action_Up()`函数开头添加LED闪烁：

```c
void SM_Action_Up(void)
{
    // 添加这一行
    Drv_LED0_Toggle();
    
    if (s_ui_state == UI_STATE_HOME_IDLE) {
        s_ui_state = UI_STATE_HOME_MENU;
        s_focus = SM_FOCUS_SET_VOLTAGE;
        return;
    }
    // ...
}
```

**测试结果**:
- **LED闪烁** → 按键响应正常，问题在UI绘制
- **LED不闪烁** → 按键或事件系统有问题

---

*文档创建时间: 2026-06-02*  
*用于排查按键交互问题*
