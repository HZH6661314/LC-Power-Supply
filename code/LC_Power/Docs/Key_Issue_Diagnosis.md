# 按键交互问题诊断与修复方案

## 📅 诊断时间
2026-06-02

---

## 🐛 问题描述

### 问题1: 短按无响应，长按才有反应
**现象**: 开机后短按UP或DOWN键无响应，需要长按1秒才能触发

**根本原因**: 
```c
// key.h 第26行
#define KEY_LONG_PRESS_TIME     1000U  // 长按判定时间：1000ms
```

**分析**:
- 按键状态机逻辑：`IDLE → DEBOUNCE(50ms) → PRESS_DETECT → 等待松开或超时1000ms`
- 短按事件触发时机：**松开按键时**触发 (key.c:123)
- 长按事件触发时机：**按住1000ms时**触发 (key.c:118)

**问题**:
用户短按（例如200ms）后松开，短按事件会正常触发，但**UI刷新周期是50ms**，状态机响应需要时间。如果你在按住不放超过1秒，才会触发长按事件，导致误以为"短按无响应"。

**真实情况推测**:
- 实际上短按是有响应的
- 但是由于**UI绘制问题**（见问题2和3），短按后光标没有正确显示，造成"无响应"的假象

---

### 问题2: 光标不是反相而是边框
**现象**: 光标显示为黑色边框，而非反相显示（黑底白字）

**根本原因**: 
当前实现**根本没有反相显示逻辑**，只有边框绘制！

```c
// ui_display.c:367-374
static void UI_DrawFocus(UI_State_t ui_state, SM_Focus_t focus)
{
    // ...
    
    // ❌ 只绘制了边框，没有反相显示！
    TFTGFX_DrawRect(x, y, w, h, UI_COLOR_HL);
    TFTGFX_DrawRect(x + 1, y + 1, w - 2, h - 2, UI_COLOR_HL);
    
    if (ui_state == UI_STATE_HOME_EDIT) {
        TFTGFX_DrawRect(x + 2, y + 2, w - 4, h - 4, UI_COLOR_HL);
    }
}
```

**期望行为**: 
- MENU状态：反相显示（黑底白字）
- EDIT状态：反相显示+2Hz闪烁

---

### 问题3: 旧光标不消失
**现象**: 移动光标时，上一个位置的光标边框仍然存在

**根本原因**: 
`UI_ClearFocus()`函数逻辑不完整

```c
// ui_display.c:377-402
static void UI_ClearFocus(void)
{
    // ...
    
    // ✅ 重新绘制了区域内容（数值）
    UI_DrawSettings(s_cache.set_voltage, s_cache.set_current);
    
    // ✅ 恢复了区域边框
    UI_RestoreSettingFrame();
    
    // ❌ 但是没有清除之前绘制的光标边框！
}
```

**分析**:
- `UI_DrawSettings()`会重绘数值文本
- `UI_RestoreSettingFrame()`会重绘区域分隔线
- **但是双层矩形框没有被清除**，因为它们在数值和边框的外面

---

## 🔧 修复方案

### 修复1: 降低长按判定时间（可选）

如果确实需要更灵敏的长按，可以调整：

```c
// key.h
#define KEY_LONG_PRESS_TIME     500U  // 改为500ms
```

**建议**: 暂时不修改，先修复UI问题后再测试

---

### 修复2: 实现真正的反相显示

需要完全重写`UI_DrawFocus()`函数：

```c
static void UI_DrawFocus(UI_State_t ui_state, SM_Focus_t focus)
{
    int16_t x, y, w, h;
    
    if (ui_state == UI_STATE_HOME_IDLE) {
        return;
    }
    
    if (UI_GetFocusRect(focus, &x, &y, &w, &h) == 0U) {
        return;
    }
    
    // 方案A: 反相填充（推荐）
    // 1. 填充黑色背景
    TFTGFX_FillRect(x, y, w, h, UI_COLOR_HL);
    
    // 2. 重新绘制白色文本
    if (focus == SM_FOCUS_SET_VOLTAGE) {
        UI_DrawSettingItem(x, y, w, h, s_cache.set_voltage, "V", UI_COLOR_BG);
    } else if (focus == SM_FOCUS_SET_CURRENT) {
        UI_DrawSettingItem(x, y, w, h, s_cache.set_current, "A", UI_COLOR_BG);
    } else if (focus == SM_FOCUS_QUICK_SET) {
        UI_DrawChinese(206, 151, "Q", UI_COLOR_BG);
    } else if (focus == SM_FOCUS_SETTINGS) {
        UI_DrawChinese(206, 184, "S", UI_COLOR_BG);
    }
    
    // EDIT状态：绘制闪烁框
    if (ui_state == UI_STATE_HOME_EDIT) {
        TFTGFX_DrawRect(x + 2, y + 2, w - 4, h - 4, UI_COLOR_BG);
    }
}
```

**问题**: 需要新增`UI_DrawSettingItem()`辅助函数

---

### 修复3: 修复UI_ClearFocus()

**方案A: 完全重绘区域（简单但效率低）**

```c
static void UI_ClearFocus(void)
{
    if (s_cache.ui_state == UI_STATE_HOME_IDLE) {
        return;
    }
    
    // 完全重绘对应区域
    if ((s_cache.focus == SM_FOCUS_SET_VOLTAGE) ||
        (s_cache.focus == SM_FOCUS_SET_CURRENT)) {
        UI_DrawSettings(s_cache.set_voltage, s_cache.set_current);
    } else if ((s_cache.focus == SM_FOCUS_QUICK_SET) ||
               (s_cache.focus == SM_FOCUS_SETTINGS)) {
        UI_DrawFunctions();
    }
}
```

**方案B: 精确清除光标（推荐）**

```c
static void UI_ClearFocus(void)
{
    int16_t x, y, w, h;
    
    if (s_cache.ui_state == UI_STATE_HOME_IDLE) {
        return;
    }
    
    if (UI_GetFocusRect(s_cache.focus, &x, &y, &w, &h) == 0U) {
        return;
    }
    
    // 如果是反相显示，恢复原样
    if ((s_cache.focus == SM_FOCUS_SET_VOLTAGE) ||
        (s_cache.focus == SM_FOCUS_SET_CURRENT)) {
        // 重绘设置项（黑色背景白色字）
        UI_DrawSettings(s_cache.set_voltage, s_cache.set_current);
    } else if ((s_cache.focus == SM_FOCUS_QUICK_SET) ||
               (s_cache.focus == SM_FOCUS_SETTINGS)) {
        // 重绘功能区
        UI_DrawFunctions();
    }
}
```

---

## 🎯 推荐实施顺序

### 第1步: 简化光标样式（快速验证）

暂时放弃反相显示，先修复"旧光标不消失"问题：

**修改目标**: 
- 光标样式：单层边框（MENU）/ 双层边框（EDIT）
- 清除逻辑：完全重绘区域

**优点**:
- 修改量小
- 快速验证按键是否真的有响应

**实施**:
```c
// 1. 修改UI_DrawFocus() - 简化为单/双层边框
// 2. 修改UI_ClearFocus() - 完全重绘对应区域
```

---

### 第2步: 实现真正的反相显示（完整方案）

**修改目标**:
- MENU状态：反相显示（黑底白字）
- EDIT状态：反相显示+闪烁边框

**挑战**:
- 需要新增辅助函数绘制反色文本
- 需要在清除时恢复正常颜色
- 代码量较大

---

## 🧪 测试验证方案

### 测试1: 按键响应测试
```c
// 在SM_Action_Up()开头添加
void SM_Action_Up(void)
{
    Drv_LED0_Toggle();  // LED闪烁表示函数被调用
    
    // 原有代码...
}
```

**预期**: 短按UP键时LED应该闪烁

---

### 测试2: 状态切换测试
```c
// 在UI_Display_Process()中添加
void UI_Display_Process(void)
{
    // ...
    
    ui_state = SM_Get_UI_State();
    
    // 调试输出
    if (s_cache.ui_state != ui_state) {
        Drv_LED1_Toggle();  // 状态变化时LED闪烁
    }
    
    // 原有代码...
}
```

**预期**: 按UP键后LED1应该闪烁（状态从IDLE变MENU）

---

### 测试3: UI绘制测试
```c
// 在UI_DrawFocus()开头添加
static void UI_DrawFocus(UI_State_t ui_state, SM_Focus_t focus)
{
    Drv_LED2_Toggle();  // 绘制光标时LED闪烁
    
    // 原有代码...
}
```

**预期**: 状态变化时LED2应该闪烁

---

## 📊 问题根因总结

| 问题 | 根本原因 | 影响 | 优先级 |
|------|----------|------|--------|
| 短按无响应 | UI绘制问题导致的假象 | 中 | P1 |
| 光标是边框而非反相 | 从未实现反相显示 | 低 | P2 |
| 旧光标不消失 | UI_ClearFocus()逻辑不完整 | 高 | P0 |

**最优先修复**: 旧光标不消失（P0）

---

## 💡 建议

### 立即行动
1. ✅ 修复`UI_ClearFocus()`完全重绘区域
2. ✅ 添加LED调试代码验证按键响应
3. ✅ 测试短按是否真的有响应

### 后续优化
4. ⚠️ 实现真正的反相显示（可选）
5. ⚠️ 调整长按时间（可选）

---

*诊断报告生成时间: 2026-06-02*  
*问题根因: UI绘制逻辑缺陷*  
*修复难度: 中等*
