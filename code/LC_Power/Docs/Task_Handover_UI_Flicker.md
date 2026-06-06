# 数控电源UI闪烁问题 - 任务交接提示词

## 📅 任务日期
2026-06-02

---

## 🎯 当前任务目标

**彻底消除所有UI更新时的闪烁问题**

用户反馈：
1. ✅ 短按UP/DOWN键能响应了（之前的LED调试帮助定位了问题）
2. ❌ **状态栏切换仍有闪烁**（"OFF"↔"ON"时先擦除后更新）
3. ❌ **电压/电流/功率显示闪烁**（数值更新时出现擦除后重绘）
4. ⚠️ **用户明确指出**：UI与按键同频（10ms刷新）是没意义的，按键识别慢不是UI任务运行频率问题

**核心要求**：
- 确保所有UI切换**完全无闪烁**
- 数值变化应该是**直接覆盖**，而不是**擦除→重绘**

---

## 📂 项目信息

**项目路径**：`d:\0_DIY\LC-Power-Supply\code\LC_Power\`

**关键文件**：
- `Layer/Product/ui_display.c` - UI显示层（需要重点检查）
- `Layer/Product/ui_display.h` - UI接口定义
- `Layer/Scheduler/task_manager.c` - 任务调度器
- `Layer/Application/state_machine.c` - 状态机
- `Layer/Control/power_control.c` - 电源控制

**编码规范**：`Docs/数控电源代码规范.md`

---

## ✅ 已完成的工作（V1-V7）

### 1. UI性能优化（V3）
- ✅ 回滚了错误的"完全重绘"方案
- ✅ CPU占用率从24%降到2%
- ✅ 解决了"设置区块一直刷新"问题

### 2. UI闪烁优化（V4）
- ✅ 调整脏检查阈值（0.01V→0.05V）
- ✅ 重绘次数减少80%

### 3. 按键状态机修复（V5）
- ✅ 修复RELEASE_DEBOUNCE状态卡死
- ✅ 添加LED调试代码（已禁用）

### 4. 安全保护实现（V5）
- ✅ 添加Output_En检查防止待机输出
- ✅ 待机状态绝对不输出电压

### 5. 输出控制修复（V6）
- ✅ 实现SoftStart状态切换到RunNing
- ✅ 按SET键能正常启动输出

### 6. 部分UI闪烁优化（V7）
- ⚠️ 尝试移除状态栏预填充
- ⚠️ 使用Opaque绘制
- ❌ **用户反馈仍有闪烁**

---

## ❌ 当前存在的问题

### 问题1：状态栏切换闪烁
**文件**：`ui_display.c` - `UI_DrawStatus()`

**现象**：
- "OFF" → "ON" 时能看到白色擦除过程
- "CV" ↔ "CC" 切换时闪烁

**可能原因**：
- `UI_DrawCenteredAscii()`虽然改用了`DrawStringOpaque`，但可能没有完全覆盖旧文本
- 或者在其他地方仍有预填充操作

### 问题2：实时数据显示闪烁
**文件**：`ui_display.c` - `UI_DrawRealtime()`

**现象**：
- 电压/电流/功率数值更新时一闪一闪
- 能看到"擦除→更新"的过程

**需要检查**：
- `UI_DrawRealtime()`是否有FillRect调用
- 是否使用了Opaque绘制
- 数值格式化后的文本宽度是否会变化

### 问题3：设置值显示闪烁
**文件**：`ui_display.c` - `UI_DrawSettings()`

**现象**：
- 修改设置电压/电流时区块闪烁

**已修复但需验证**：
- 已移除预填充
- 已使用Opaque绘制
- 但用户反馈可能还有问题

---

## 🔍 需要系统检查的函数

### 高优先级（必须检查）
1. `UI_DrawRealtime(float vout, float iout)` - 实时数据显示
2. `UI_DrawPower(float power)` - 功率显示
3. `UI_DrawSettings(float voltage, float current)` - 设置值显示
4. `UI_DrawStatus(...)` - 状态栏显示
5. `UI_DrawCenteredAscii(...)` - 居中文本绘制
6. `UI_DrawValueText(...)` - 数值文本绘制

### 检查要点
对每个函数检查：
- [ ] 是否有`TFTGFX_FillRect(..., UI_COLOR_BG)`预填充？
- [ ] 是否使用`TFTGFX_DrawString()`透明绘制？
- [ ] 是否使用`TFTGFX_DrawStringOpaque()`覆盖绘制？
- [ ] 数值格式化后宽度是否固定？
- [ ] 是否有多余的重绘操作？

---

## 💡 修复原则（Karpathy原则）

### 原则1：永远不要预填充背景
```c
// ❌ 错误：先擦除后绘制
TFTGFX_FillRect(x, y, w, h, UI_COLOR_BG);  // 擦除
TFTGFX_DrawString(x, y, text, color, scale);  // 绘制

// ✅ 正确：直接覆盖
TFTGFX_DrawStringOpaque(x, y, text, color, UI_COLOR_BG, scale);
```

### 原则2：使用Opaque绘制
```c
// ❌ 透明绘制：不覆盖背景，旧内容残留
TFTGFX_DrawString(x, y, text, fg_color, scale);

// ✅ 覆盖绘制：同时绘制前景和背景
TFTGFX_DrawStringOpaque(x, y, text, fg_color, bg_color, scale);
```

### 原则3：固定宽度或完全覆盖
```c
// 如果数值宽度会变化（"1.23" → "12.34"）
// 方案A：固定格式宽度
snprintf(text, sizeof(text), "%05.2f", value);  // 固定5位

// 方案B：或者确保覆盖足够宽的区域
// 使用Opaque绘制时，背景色会覆盖旧数值
```

---

## 🚀 下一步行动计划

### 步骤1：系统审查所有UI绘制函数（30分钟）
```
1. 读取ui_display.c完整代码
2. 找出所有UI绘制函数
3. 逐个检查是否符合"无闪烁原则"
4. 列出所有问题函数
```

### 步骤2：批量修复（30分钟）
```
1. 移除所有预填充FillRect
2. 将所有DrawString改为DrawStringOpaque
3. 确保数值格式化宽度固定
4. 统一使用覆盖绘制模式
```

### 步骤3：恢复合理的UI刷新周期（5分钟）
```
1. 将UI刷新周期从10ms改回20ms或30ms
2. 用户说得对：10ms刷新对按键响应没有实际帮助
3. 降低CPU占用到合理水平
```

### 步骤4：测试验证（10分钟）
```
1. 编译烧录
2. 测试所有UI切换场景
3. 确认完全无闪烁
```

---

## 📝 技术背景

### TFT绘制API
```c
// 透明绘制：只绘制前景像素，背景透明
void TFTGFX_DrawString(int16_t x, int16_t y, const char *text, 
                       uint16_t color, uint8_t scale);

// 覆盖绘制：同时绘制前景和背景像素
void TFTGFX_DrawStringOpaque(int16_t x, int16_t y, const char *text, 
                              uint16_t fg_color, uint16_t bg_color, 
                              uint8_t scale);

// 填充矩形：用指定颜色填充区域
void TFTGFX_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, 
                     uint16_t color);
```

### 闪烁产生原理
```
时间线：
T0:   TFTGFX_FillRect(x, y, w, h, WHITE)  ← 屏幕变白（1-2ms）
T2:   TFTGFX_DrawString(x, y, "12.34", BLACK, 2)  ← 绘制新文本（1ms）
      
用户看到：白色闪烁（人眼能察觉>10ms的变化）

正确做法：
T0:   TFTGFX_DrawStringOpaque(x, y, "12.34", BLACK, WHITE, 2)
      ↑ 一次性绘制前景和背景，无中间状态，无闪烁
```

---

## ⚠️ 重要提醒

### 1. 先阅读编码规范
在修改代码前，务必先阅读：
```
/d/0_DIY/LC-Power-Supply/code/LC_Power/Docs/数控电源代码规范.md
```

### 2. 使用Skill工具
**重要**：新对话开始时，使用skill工具加载项目上下文：
```
/remember
```
这会加载项目记忆和之前的工作历史

### 3. 保持代码风格
- 使用Karpathy原则：简洁、清晰、早返回
- 函数命名：模块前缀（如`UI_`）
- 静态变量：`s_`前缀
- 注释清晰：说明为什么这样做

### 4. 测试驱动
- 每次修改后编译测试
- 确认无编译错误
- 在硬件上验证效果
- 根据用户反馈调整

---

## 📚 参考文档

已生成的报告文档：
- `Final_Fix_Report_V7.md` - V7版本修复报告
- `Performance_Fix_Report_V3.md` - V3性能优化报告
- `UI_Flicker_Fix_Plan.md` - UI闪烁修复方案
- `LED_Debug_Guide.md` - LED调试指南

这些文档包含了详细的问题分析和修复历史。

---

## 🎯 成功标准

**任务完成的标志**：
1. ✅ 状态栏"OFF"↔"ON"切换**完全无闪烁**
2. ✅ "CV"↔"CC"模式切换**完全无闪烁**
3. ✅ 电压数值更新**直接变化**，无擦除过程
4. ✅ 电流数值更新**直接变化**，无擦除过程
5. ✅ 功率数值更新**直接变化**，无擦除过程
6. ✅ 温度数值更新**直接变化**，无擦除过程
7. ✅ CPU占用率保持在合理水平（<10%）

**用户的判断标准**：
> "所有UI切换完全无闪烁"

只有当用户明确表示"完全无闪烁"时，任务才算完成。

---

## 🤝 与用户沟通要点

1. **先理解，后行动**
   - 仔细听用户反馈
   - 不要假设问题原因
   - 验证你的理解是否正确

2. **承认错误**
   - 如果方向错了，坦率承认
   - 重新分析问题
   - 提出新的方案

3. **增量修复**
   - 一次解决一个问题
   - 每次修改后测试验证
   - 根据反馈调整方向

4. **技术解释要准确**
   - 不要过度解释
   - 用简洁的语言说明原理
   - 让用户能验证效果

---

## 📌 关键要点总结

1. **核心任务**：消除所有UI闪烁
2. **关键文件**：`ui_display.c`
3. **修复原则**：永远不预填充，永远用Opaque绘制
4. **成功标准**：用户确认"完全无闪烁"
5. **重要提醒**：新对话开始先用`/remember`加载上下文

---

*任务交接时间: 2026-06-02*  
*当前版本: V7（部分完成）*  
*待完成: 彻底消除所有UI闪烁*  
*优先级: 最高*

---

## 🚀 给下一个对话的第一条指令

```
请帮我检查并修复数控电源项目的所有UI闪烁问题。

项目路径: d:\0_DIY\LC-Power-Supply\code\LC_Power\

核心问题:
1. 状态栏"OFF"↔"ON"切换时闪烁
2. 电压/电流/功率数值更新时闪烁（能看到擦除→重绘过程）

要求: 确保所有UI切换完全无闪烁，数值应该直接覆盖而不是擦除后重绘。

请先用/remember加载项目上下文，然后系统检查ui_display.c中的所有绘制函数，找出并修复所有闪烁问题。
```
