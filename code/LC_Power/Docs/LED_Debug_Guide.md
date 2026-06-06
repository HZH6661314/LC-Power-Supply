# 🔍 按键无响应 - LED调试指南

## 📅 2026-06-02
## 🎯 目标：通过LED闪烁模式定位问题层次

---

## 🚨 当前状态

**问题确认**：
- ✅ 长按1000ms：有响应
- ❌ 短按<1000ms：**完全无响应**
- ✅ UI性能已优化
- ✅ 设置区块不再闪烁

**结论**：问题不在UI层，在按键处理链路中

---

## 🔧 已添加的调试代码

### 调试点1：按键事件入队
**文件**：`key.c` - `Push_Event_To_Queue()`
```c
uint8_t Push_Event_To_Queue(BtnId_t id, BtnEvent_t event)
{
    // ...
    
    // 🔍 LED2闪烁 - 表示按键事件成功入队
    Drv_LED2_Toggle();
    
    g_BtnQueue.buffer[g_BtnQueue.head].id = id;
    g_BtnQueue.buffer[g_BtnQueue.head].event = event;
    // ...
}
```

### 调试点2：状态机函数被调用
**文件**：`state_machine.c` - `SM_Action_Up()`
```c
void SM_Action_Up(void)
{
    // 🔍 LED0闪烁 - 表示函数被调用
    Drv_LED0_Toggle();

    if (s_ui_state == UI_STATE_HOME_IDLE) {
        // 🔍 LED1点亮 - 表示进入此分支
        Drv_LED1_ON();
        
        s_ui_state = UI_STATE_HOME_MENU;
        s_focus = SM_FOCUS_SET_VOLTAGE;
        return;
    }
    // ...
}
```

---

## 🧪 测试步骤

### 步骤1：编译烧录
```bash
1. 在Keil MDK中按F7编译
2. 确认无错误和警告
3. 烧录到硬件
4. 上电等待TFT初始化完成
```

### 步骤2：短按UP键测试
```
操作：短按UP键（200-300ms后松开）
观察：LED0、LED1、LED2的状态
```

---

## 📊 LED模式判断表

### 场景A：LED2闪烁 + LED0闪烁 + LED1点亮
```
LED2: ✅ 闪烁  ← 事件入队成功
LED0: ✅ 闪烁  ← SM_Action_Up()被调用
LED1: ✅ 点亮  ← 进入了HOME_IDLE分支

判断：按键链路完全正常，问题在UI层
原因：UI未响应状态变化

解决方案：
1. 检查UI_Display_Process()是否被定时调用
2. 检查脏检查逻辑
3. 检查SM_Get_UI_State()返回值
```

---

### 场景B：LED2闪烁 + LED0闪烁，LED1不亮
```
LED2: ✅ 闪烁  ← 事件入队成功
LED0: ✅ 闪烁  ← SM_Action_Up()被调用
LED1: ❌ 不亮  ← 没有进入HOME_IDLE分支

判断：状态判断有问题
原因：s_ui_state != UI_STATE_HOME_IDLE

解决方案：
1. 检查s_ui_state初始化值
2. 检查是否被其他代码修改
3. 添加状态日志输出
```

---

### 场景C：LED2闪烁，LED0和LED1都不闪烁
```
LED2: ✅ 闪烁  ← 事件入队成功
LED0: ❌ 不闪  ← SM_Action_Up()未被调用
LED1: ❌ 不亮  ← 不可能进入

判断：事件映射/分发有问题
原因：MainPage_KeyMap映射表错误或遍历逻辑错误

解决方案：
1. 检查MainPage_KeyMap数组
2. 检查BTN_KEY_UP的ID值
3. 检查Key_Process()的遍历逻辑
4. 添加映射匹配日志
```

---

### 场景D：LED2不闪烁（最可能）⭐⭐⭐⭐⭐
```
LED2: ❌ 不闪  ← 事件未入队
LED0: ❌ 不闪  ← 不可能被调用
LED1: ❌ 不亮  ← 不可能进入

判断：按键状态机有问题（最可能！）
原因：短按事件根本没有被触发

可能的根因：
1. ✅ RELEASE_DEBOUNCE状态卡死（已修复，但可能还有问题）
2. ⚠️ 短按事件触发条件不满足
3. ⚠️ 按键状态机逻辑错误
4. ⚠️ 时钟未运行或运行异常
5. ⚠️ GPIO读取有问题

解决方案：
→ 见下面的"场景D深入诊断"
```

---

## 🔬 场景D深入诊断

### 假设1：短按事件触发条件不满足

**检查短按事件触发条件**（key.c:115-127）：
```c
case KEY_STATE_PRESS_DETECT:
    if (current_level == PIN_LOW) {  // 按键仍按下
        if ((uint32_t)(now_ms - btn->press_timestamp) >= KEY_LONG_PRESS_TIME) {
            Push_Event_To_Queue(btn->id, BTN_EVENT_LONG_PRESS);  // ← 长按1000ms触发
            // ...
        }
    } else {  // 按键松开
        Push_Event_To_Queue(btn->id, BTN_EVENT_SHORT_PRESS);  // ← 短按应该在这触发
        // ...
    }
```

**问题**：
- 短按事件在**松开时**触发（`current_level == PIN_HIGH`）
- 如果`current_level`读取有问题，永远不会触发

**验证方法**：
添加GPIO读取调试：
```c
case KEY_STATE_PRESS_DETECT:
    Drv_LED3_Toggle();  // ← 添加：每次进入此状态闪烁一次
    
    if (current_level == PIN_LOW) {
        // ...
    } else {
        Push_Event_To_Queue(btn->id, BTN_EVENT_SHORT_PRESS);
        // ...
    }
```

**如果LED3闪烁但LED2不闪烁**：
- 说明进入了PRESS_DETECT状态
- 但`current_level != PIN_HIGH`，所以没有触发短按
- **问题在GPIO读取或电平定义**

---

### 假设2：按键永远卡在某个状态

**添加状态跟踪**：
```c
void Button_Process_Engine(Button_t *btn)
{
    static Key_State_t last_state[BTN_NUM_MAX] = {0};
    
    // ...
    
    // 调试：状态变化时闪烁LED
    if (btn->state != last_state[btn->id]) {
        Drv_LED3_Toggle();  // LED3闪烁表示状态变化
        last_state[btn->id] = btn->state;
    }
    
    switch (btn->state) {
        // ...
    }
}
```

**如果LED3不闪烁**：
- 按键状态机卡死在某个状态
- 按键按下时没有状态变化

---

### 假设3：时钟未运行

**验证方法**：
```c
// 在主循环中添加心跳
void SysCore_Run(void)
{
    static uint32_t last_heartbeat = 0;
    uint32_t now = g_Ticks[TICK_MS].Tick();
    
    // 每1000ms闪烁一次LED
    if ((uint32_t)(now - last_heartbeat) >= 1000U) {
        Drv_LED3_Toggle();
        last_heartbeat = now;
    }
    
    // 原有代码...
}
```

**如果LED3以1Hz频率闪烁**：
- 时钟正常运行

**如果LED3不闪烁**：
- 时钟未运行或卡死

---

## 🎯 推荐诊断流程

### 第1步：基础测试
```
1. 编译烧录（已添加LED2、LED0、LED1调试）
2. 短按UP键
3. 观察LED模式
4. 根据"LED模式判断表"定位问题层次
```

### 第2步：如果是场景D（LED2不闪烁）
```
添加更多调试代码：
1. 在PRESS_DETECT中添加LED3闪烁
2. 在状态变化时添加LED3闪烁
3. 重新编译烧录
4. 观察LED3行为
```

### 第3步：根据LED3行为进一步定位
```
LED3闪烁：说明状态机在运行，问题在触发条件
LED3不闪烁：说明状态机卡死或未运行
```

---

## 📝 反馈模板

请按以下格式反馈测试结果：

```
测试条件：
- 开机后等待TFT初始化完成
- 短按UP键（约200-300ms）

LED观察结果：
- LED0（状态机函数）: [ ] 闪烁  [ ] 不闪
- LED1（进入分支）: [ ] 点亮  [ ] 不亮
- LED2（事件入队）: [ ] 闪烁  [ ] 不闪

其他观察：
- 屏幕是否有任何变化：
- 是否听到继电器声音：
- 其他异常现象：
```

---

## 🔧 快速修复建议

### 如果是场景D（最可能）

基于之前的分析，问题很可能是：
**按键松开时`current_level`读取异常**

**快速测试修复**：
```c
// key.c:115-127
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
    
    // ✅ 添加超时保护：如果在PRESS_DETECT超过2秒，强制触发短按
    if ((uint32_t)(now_ms - btn->press_timestamp) >= 2000U) {
        Push_Event_To_Queue(btn->id, BTN_EVENT_SHORT_PRESS);
        btn->state = KEY_STATE_IDLE;
    }
    break;
```

---

## 🚀 下一步行动

**立即**：
1. ✅ 编译项目（已添加LED调试代码）
2. ✅ 烧录测试
3. ✅ 短按UP键
4. ✅ 观察LED模式
5. ✅ 按照"反馈模板"告诉我结果

**我会根据你的LED观察结果**：
- 精确定位问题在哪一层
- 提供针对性的修复方案
- 彻底解决短按无响应问题

---

*调试指南生成时间: 2026-06-02*  
*调试方法: LED闪烁模式分析*  
*预期: 场景D - 按键状态机问题*  
*等待你的LED观察结果！*
