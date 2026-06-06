# 快速设置子菜单UI实现提示词

## 文档信息
- **创建日期**: 2026-06-06
- **基于版本**: V10 (光标闪烁已修复)
- **任务类型**: 功能新增 - 快速设置子菜单UI绘制
- **优先级**: P1
- **预计工时**: 3-4小时

---

## 严格遵守Agent工具与规范要求

**本任务必须严格遵守以下规范：**

1. **代码规范** (`Docs/数控电源代码规范.md`):
   - BSP层：只提供硬件抽象，不包含业务逻辑
   - Product层：实现UI渲染和交互逻辑
   - Application层：状态管理和业务流程控制
   - 变量命名：`g_`全局、`s_`静态、`p_`指针
   - 函数命名：`Module_Action_Target()`格式

2. **架构分层原则**:
   - 严禁跨层调用（如UI层直接访问BSP层）
   - 必须通过Application层（state_machine）中转
   - BSP层接口必须通过头文件导出

3. **Karpathy编码原则**:
   - 简洁性：最小化代码行数
   - 单一职责：一个函数只做一件事
   - 早返回：避免深度嵌套
   - 清晰命名：描述性名称，无缩写

4. **提交规范**:
   - 使用约定式提交：`feat(module): 描述`
   - 提交信息包含：修改内容、原因、影响
   - 每个逻辑完整的功能单独提交

5. **测试要求**:
   - 每次修改后必须编译验证
   - 功能完成后必须硬件测试
   - 记录测试结果

---

## 当前项目状态总结

### 已完成功能 ✅

1. **V1-V8**: 基础UI框架和闪烁修复
   - 240×240 TFT显示驱动
   - 基础UI布局（标题、实时数据、设定值、功率、状态栏）
   - Dirty-flag局部刷新机制
   - 按键输入处理（消抖、长按/短按识别）
   - 状态机框架（待机、菜单、编辑状态）

2. **V9**: 固定宽度Opaque渲染
   - 重写`UI_DrawCenteredAscii`为固定宽度Opaque模式
   - 消除文本绘制闪烁
   - **硬件验证失败**：边框擦除 + 光标闪烁缓慢

3. **V10**: 边框修复 + 光标闪烁重构 ✅
   - **Task 1**: 边框安全区 - `effective_chars = max_chars - 1`
   - **Task 2-4**: TIM2闪烁时基 - 符合BSP分层原则
   - **修复**: TIM2启动时机 - 在UI初始化后启动
   - **重构**: BSP层只提供`g_TIM2_Tick`计数器，Product层实现闪烁逻辑
   - **状态**: ✅ 屏幕正常、✅ 边框完整、✅ 光标闪烁2Hz

### 当前架构概览

```
┌─────────────────────────────────────────────────────────┐
│ Application Layer (state_machine.c)                     │
│ - UI状态管理：IDLE/MENU/EDIT/QUICK_SET/SYS_SET        │
│ - 焦点管理：SET_VOLTAGE/SET_CURRENT/QUICK_SET/SETTINGS │
│ - 设定值管理：电压/电流/功率限制                        │
│ - 动作接口：Enter/Exit/Up/Down                         │
└─────────────────────────────────────────────────────────┘
                          ↓ ↑
┌─────────────────────────────────────────────────────────┐
│ Product Layer (ui_display.c / key.c)                    │
│ - UI渲染：DrawRealtime/DrawSettings/DrawStatus...      │
│ - 按键处理：扫描、消抖、事件派发                        │
│ - Dirty-flag刷新：只重绘变化区域                        │
└─────────────────────────────────────────────────────────┘
                          ↓ ↑
┌─────────────────────────────────────────────────────────┐
│ Driver Layer (tft_driver.c / tft_gfx.c)                │
│ - TFT硬件接口：命令/数据发送                            │
│ - 图形原语：FillRect/DrawString/DrawChar               │
│ - 字体渲染：5×7点阵，支持缩放                          │
└─────────────────────────────────────────────────────────┘
                          ↓ ↑
┌─────────────────────────────────────────────────────────┐
│ BSP Layer (bsp_lcd.c / bsp_hrtim.c / bsp_gpio.c)       │
│ - 硬件抽象：SPI/GPIO/Timer                             │
│ - 只提供接口，不含业务逻辑                             │
│ - 例：g_TIM2_Tick计数器（不包含闪烁逻辑）              │
└─────────────────────────────────────────────────────────┘
```

### 已知约束

1. **显示硬件**:
   - ST7789 240×240 TFT
   - 软件SPI（位拍），速度约2MHz
   - 全屏刷新约86ms，小文本区域5-10ms

2. **性能要求**:
   - UI刷新周期：50ms (20Hz)
   - CPU占用：<5%
   - 使用Dirty-flag避免无意义重绘

3. **UI布局**:
   - 标题栏：y=0-32
   - 实时数据：y=32-142（左侧）
   - 设定值：y=32-142（右侧）
   - 功率显示：y=142-210（左侧）
   - 功能菜单：y=142-210（右侧）
   - 状态栏：y=210-240

4. **字体限制**:
   - 当前只支持ASCII（5×7点阵）
   - 中文字体渲染待实现
   - 支持缩放：1x, 2x, 3x, 4x

---

## 待实现任务分析（仅分析，不设计解决方案）

### 任务1：快速设置子菜单UI绘制 🎯 当前任务

**背景**:
- 用户需要快速应用常用电压/电流预设
- 当前主界面的"快速设置"按钮只是占位，点击后无实际功能
- 需要实现一个子菜单界面显示预设列表

**涉及的状态**:
- `UI_STATE_QUICK_SET` - 已在state_machine.h中定义但未实现
- `SM_FOCUS_QUICK_SET` - 主界面焦点位置已定义

**需要分析的问题**:

1. **UI设计问题**:
   - 子菜单界面布局？（全屏覆盖 vs 弹窗式）
   - 预设项显示方式？（列表 vs 网格）
   - 每个预设项显示什么信息？（电压、电流、名称）
   - 如何指示当前选中项？（高亮 vs 边框 vs 光标）
   - 如何显示"返回"或"取消"选项？

2. **交互逻辑问题**:
   - UP/DOWN键如何移动选择？（循环 vs 边界停止）
   - ENTER键确认后做什么？（应用预设并返回 vs 应用预设继续编辑）
   - EXIT键如何处理？（直接返回 vs 询问确认）
   - 长按是否有特殊功能？（快速返回主界面）

3. **数据管理问题**:
   - 预设数据存储在哪里？（state_machine vs 独立模块）
   - 预设项数量？（固定3-5个 vs 可扩展）
   - 预设内容是什么？（电压+电流 vs 电压+电流+功率限制）
   - 预设是否可编辑？（当前阶段固定 vs 未来支持自定义）

4. **状态机扩展问题**:
   - 如何进入QUICK_SET状态？（从MENU状态Enter）
   - 如何退出QUICK_SET状态？（Exit回到MENU vs 直接回IDLE）
   - QUICK_SET状态下需要哪些子状态？（浏览 vs 确认）
   - 是否需要新的Focus枚举？（子菜单项的焦点）

5. **渲染性能问题**:
   - 子菜单切换是否流畅？（全屏重绘耗时）
   - 是否需要动画效果？（淡入淡出 vs 立即切换）
   - 选项切换时重绘策略？（整体重绘 vs 局部更新）

6. **代码架构问题**:
   - 子菜单UI绘制放在哪里？（ui_display.c新增函数）
   - 预设数据结构如何定义？（结构体数组）
   - 是否需要新的头文件？（quick_set.h vs 复用现有）
   - 与主界面代码如何隔离？（避免耦合）

---

## 关键API接口

### Application层（state_machine.h）

```c
// 状态查询
UI_State_t SM_Get_UI_State(void);
SM_Focus_t SM_Get_Focus(void);
float SM_Get_TargetVoltageFinal(void);
float SM_Get_CurrentLimit(void);

// 动作接口
void SM_Action_Enter(void);       // SET/WT_SET短按
void SM_Action_Exit_Short(void);  // EXIT短按
void SM_Action_Exit_Long(void);   // EXIT长按
void SM_Action_Up(void);          // UP键
void SM_Action_Down(void);        // DOWN键
```

### Product层（ui_display.h）

```c
void UI_Display_Init(void);
void UI_Display_Process(void);  // 50ms调用一次
```

### Driver层（tft_gfx.h）

```c
// 图形原语
void TFTGFX_FillScreen(uint16_t color);
void TFTGFX_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void TFTGFX_DrawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

// 字符渲染（Opaque模式避免闪烁）
void TFTGFX_DrawCharOpaque(int16_t x, int16_t y, char ch, uint16_t fg, uint16_t bg, uint8_t scale);
void TFTGFX_DrawStringOpaque(int16_t x, int16_t y, const char *str, uint16_t fg, uint16_t bg, uint8_t scale);
```

---

## 重要提醒

1. **本文档只分析任务，不提供解决方案**
2. **实施前必须先制定详细设计方案**
3. **必须严格遵守代码规范和架构分层**
4. **每次修改必须编译验证和硬件测试**
5. **所有提交必须遵循约定式提交规范**

---

*文档创建时间: 2026-06-06*  
*基于版本: V10*  
*下一任务: 快速设置子菜单UI绘制*
