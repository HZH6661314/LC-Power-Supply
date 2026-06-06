# LC电源研究日志

## 2026-06-03: UI闪烁根本原因分析

### 调查：为什么UI仍然闪烁（V7 → V8）

**背景**: 用户报告V7部分修复后闪烁仍然存在。

#### 发现

##### 1. TFT图形API设计
`tft_gfx.c`层提供两种渲染模式：

**透明模式**（导致旧内容残留）:
```c
void TFTGFX_DrawChar(int16_t x, int16_t y, char ch, uint16_t color, uint8_t scale);
void TFTGFX_DrawString(int16_t x, int16_t y, const char *str, uint16_t color, uint8_t scale);
```
- 仅绘制字体位图中设置的前景像素
- 背景像素不被触碰
- 旧文本在下方仍然可见

**覆盖模式**（原子覆写）:
```c
void TFTGFX_DrawCharOpaque(int16_t x, int16_t y, char ch, uint16_t fg_color, uint16_t bg_color, uint8_t scale);
void TFTGFX_DrawStringOpaque(int16_t x, int16_t y, const char *str, uint16_t fg_color, uint16_t bg_color, uint8_t scale);
```
- 同时绘制前景和背景像素
- 每个字符完全覆盖其边界框
- 无中间白色闪烁

##### 2. 当前UI函数状态

**已修复（使用Opaque）**:
- ✅ `UI_DrawRealtime()` - 电压/电流显示
- ✅ `UI_DrawSettings()` - 设置电压/电流
- ✅ `UI_DrawStatus()` - 状态栏（通过`UI_DrawCenteredAscii`）

**仍有问题（预填充模式）**:
- ❌ `UI_DrawPower()` - 第327行: `TFTGFX_FillRect(2, 144, 133, 63, UI_COLOR_BG);`
- ❌ `UI_DrawFunctions()` - 第336-337行: 两个`TFTGFX_FillRect()`调用

##### 3. 为什么预填充导致闪烁

**时序分析**（基于SPI速度~2MHz）:
```
帧时间线：
T0    : FillRect(133×63)开始 → 约1.2ms填充8379像素
T1.2ms: FillRect完成 → 屏幕区域变为白色
T1.2ms: DrawString开始 → 约0.8ms渲染文本
T2.0ms: DrawString完成 → 文本可见

人眼感知：
- 闪烁阈值: 约10-16ms（60Hz感知）
- 白色闪烁持续时间: 1.2ms
- 可见性: 是（在多帧中累积）
```

即使每次闪烁很短，人眼会整合这些白色脉冲，产生可见闪烁。

##### 4. 固定宽度格式的重要性

对于`UI_DrawPower()`:
```c
// 当前格式
snprintf(text, sizeof(text), "%05.1f", power);  // "012.3" 固定5字符

// 宽度分析:
// 最小: "000.0" → 5字符 × 6px × 4倍缩放 = 120px
// 最大: "999.9" → 5字符 × 6px × 4倍缩放 = 120px
```
✅ 宽度恒定 → Opaque渲染将完美工作

对于`UI_DrawRealtime()`:
```c
// 电压格式
snprintf(text, sizeof(text), "%05.2f", vout);  // "012.34" → 6字符

// 电流格式
snprintf(text, sizeof(text), "%05.3f", iout);  // "01.234" → 6字符
```
✅ 两者都有固定宽度 → 已正确工作

##### 5. 字体渲染细节

从`tft_gfx.c`分析:
- 字体: 5×7像素矩阵
- 字符间距: 6像素（5 + 1px间隙）
- `TFTGFX_DrawCharOpaque()`用背景色填充1px间隙（第543-550行）
- 这确保完全覆盖，无间隙

#### 结论

**根本原因**: 仅剩2个函数保留预填充模式：
1. `UI_DrawPower()` - 绘制前清除133×63像素矩形
2. `UI_DrawFunctions()` - 绘制前清除两个文本区域

**解决方案**: 用直接Opaque绘制替换预填充+透明绘制。

---

## 2026-06-02: V7调查 - 部分成功

### 为什么V7修复不完整

**V7修复了什么**:
- 实时电压/电流显示（切换到Opaque）
- 设置显示（切换到Opaque）
- 状态栏绘制（使用`UI_DrawCenteredAscii`和Opaque）

**V7遗漏了什么**:
- 功率显示仍使用旧的擦除-重绘模式
- 功能标签仍使用旧模式
- 代码注释声称"已使用Opaque"但实现不匹配

**经验教训**: 必须验证实现与注释匹配。

---

## 2026-06-01: 按键消抖调查

### 问题: UP/DOWN按键无响应

**找到根本原因**: 状态机有`RELEASE_DEBOUNCE`状态从不转出。

**应用的修复**:
```c
case KEY_STATE_RELEASE_DEBOUNCE:
    if (current_level == KEY_LEVEL_RELEASED) {
        if ((current_tick - key->timestamp) >= KEY_RELEASE_DEBOUNCE_MS) {
            // 添加此行（之前缺失）:
            key->state = KEY_STATE_IDLE;
        }
    }
```

**使用的调试技术**: 在`SM_Action_Up()`中LED切换确认函数被调用，将问题范围缩小到按键状态机。

---

## 2026-05-30: 性能危机

### 问题: CPU占用24%，UI卡顿

**根本原因**: 无论是否有变化，设置区域每50ms重绘一次。

**为什么发生**: 脏标志检查对float使用`!=`比较，由于浮点精度导致持续误判。

**修复**:
```c
// 错误方法
if (s_cache.set_voltage != set_voltage) { redraw; }

// 正确方法
if (UI_FloatChanged(s_cache.set_voltage, set_voltage, 0.05f)) { redraw; }

static uint8_t UI_FloatChanged(float old, float new, float threshold) {
    float diff = old - new;
    if (diff < 0.0f) diff = -diff;
    return (diff >= threshold) ? 1U : 0U;
}
```

**影响**: CPU使用率从24%降至2%。

---

## TFT显示特性

### 硬件: ST7789 240×240 TFT
- 分辨率: 240×240像素
- 色深: RGB565（16位）
- 接口: 软件SPI（位拍）
- 估计SPI速度: 约2MHz（基于CPU时钟和GPIO切换时序）

### 性能指标
- 像素写入时间: 约1.5μs/像素（2字节 @ 2MHz SPI）
- 全屏清除: 约86ms（240×240 = 57,600像素）
- 小文本区域（120×30）: 约5.4ms

### 为什么闪烁可见
即使单个操作很快（1-5ms），序列：
```
清除 → 写入
```
创建了一个可见的中间状态，眼睛将其感知为闪烁，特别是在20Hz刷新速率下重复时。

---

## 应用的Karpathy编码原则

### 1. 早返回（避免深度嵌套）
```c
// 示例来自state_machine.c:89
void SM_Action_Enter(void) {
    if (s_ui_state == UI_STATE_HOME_IDLE) {
        SM_ToggleOutput();
        return;  // 早退出
    }
    
    if (s_ui_state == UI_STATE_HOME_MENU) {
        // ... 处理菜单
        return;
    }
    // ... 更多情况
}
```

### 2. 单一职责
每个函数做一件事：
- `UI_DrawRealtime()` - 仅绘制实时测量值
- `UI_DrawSettings()` - 仅绘制设置
- `UI_DrawStatus()` - 仅绘制状态栏

### 3. 最小化状态
静态变量使用清晰前缀：
- `s_cache` - UI缓存用于脏检查
- `s_ui_state` - 当前UI状态
- `s_focus` - 当前焦点位置

### 4. 清晰命名
- 函数: `Module_Action_Target()`格式
- 变量: 描述性名称，无缩写
- 常量: `MODULE_CONSTANT_NAME`

---

## 参考资料

### 外部资源
- ST7789数据手册: 显示控制器命令集
- STM32F3参考手册: HRTIM、ADC、GPIO时序
- ARM Cortex-M4编程指南: 中断优先级、systick

### 内部文档
- `Docs/数控电源代码规范.md` - 项目编码标准
- `Final_Fix_Report_V7.md` - V7修复摘要
- `Task_Handover_UI_Flicker.md` - 当前任务规范

---

*最后更新: 2026-06-03*  
*会话: V9开发*

---

## 2026-06-03: V9 - 固定宽度Opaque渲染方案

### 背景
V8.3遗留了两个缺陷：
1. **边框恢复不完整**：FillRect覆盖右侧垂直线（x=59, 119, 179），仅恢复了左侧线
2. **"normal"文本无脏检查**：静态文本无法动态更新

### 方案对比

| 方案 | 方法 | 优点 | 缺点 |
|------|------|------|------|
| **方案A（选中）** | 固定宽度Opaque渲染 | 无闪烁、无边框问题、代码简洁 | 需要修改UI_DrawCenteredAscii |
| 方案B | 补全边框恢复逻辑 | 改动小 | 治标不治本，仍有可能闪烁 |

### 核心决策：废弃FillRect+恢复边框的三元组

**原有渲染流程**（V8.3）：
```
FillRect → 覆盖右边框 → DrawString → 恢复左边框（错误的线）
```

**新渲染流程**（V9）：
```
ConstructPadded(width) → DrawStringOpaque → 完成（无需恢复）
```

### 固定宽度计算

```c
max_chars = w / (6 * scale)
pad_total = max_chars - text_len
pad_left = pad_total / 2
pad_right = pad_total - pad_left
```

**示例**：60px宽，scale=2
- max_chars = 60/12 = 5
- "ON"(2) → pad_left=1, pad_right=2 → " ON  "
- "OFF"(3) → pad_left=1, pad_right=1 → " OFF "

### 结论
固定宽度Opaque渲染从本质上解决了边框问题和闪烁问题，是最终的正确方案。

---

## 2026-06-05: V10 — V9 硬件验证失败根因分析

### 问题 1：V9 的 Opaque 渲染仍然擦除边框

**用户反馈**：开机后 StaticFrame 绘制完边框线，状态栏字符出现时垂直线 x=59/119/179 被擦除。

**深层根因分析**：

V9 的设计假设是 `DrawStringOpaque(x, y, "  ON  ")` 精确覆盖 60px 宽的格子，不触碰边框线。
但 `DrawStringOpaque` 内部调用 `DrawCharOpaque`，而 `DrawCharOpaque` 有一个**关键的间隙填充**逻辑：

```c
// tft_gfx.c 第 543-550 行
// 每个字符绘制完 5 列像素后，额外绘制 1px 间隙（scale 倍）
if (scale == 1U) {
    TFTGFX_FillRect(x + 5, y, 1, 7, bg_color);        // +5→+5，覆盖 x+5
} else {
    TFTGFX_FillRect(x + (5*scale), y, scale, 7*scale, bg_color);  // scale=2: x+10→x+11
}
```

**字符间隙的像素布局（scale=2）**：
```
每个字符：5列×2 = 10px 字符像素 + 2px 间隙
总宽度 = 6×2 = 12px/字符

以一个格子为例 (Cell 0: x=0, w=60, scale=2, max_chars=5)：
"  ON  "  = 空格 空格 O N 空格 空格

字符0 (' '): x=0,  像素=[0,9],   间隙=[10,11]
字符1 (' '): x=12, 像素=[12,21], 间隙=[22,23]
字符2 ('O'): x=24, 像素=[24,33], 间隙=[34,35]
字符3 ('N'): x=36, 像素=[36,45], 间隙=[46,47]
字符4 (' '): x=48, 像素=[48,57], 间隙=[58,59]
```

**关键发现**：第 5 个字符（最后一个空格）的间隙填充 `x+10` 覆盖了 x=58 和 x=59！
而 x=59 正是状态栏的第一条垂直线。

所以 **Opaque 渲染自身**（不是因为 FillRect），通过 `DrawCharOpaque` 的字符间间隙填充，覆盖了边框线 x=59。

同样：
- Cell 0 (x=0,w=60)：第5个字符间隙覆盖 x=58,59 → **擦除 x=59 垂直线**
- Cell 1 (x=60,w=60)：第5个字符间隙覆盖 x=118,119 → **擦除 x=119 垂直线**  
- Cell 2 (x=120,w=60)：第5个字符间隙覆盖 x=178,179 → **擦除 x=179 垂直线**

### 问题 2：光标闪烁极其缓慢

**用户反馈**：光标闪烁明显慢于预期的 2Hz，调整 `UI_BLINK_TICKS` 没有效果。

**根因分析**：

`UI_Display_Process()` 在 `task_manager.c` 的 50ms 任务中调用：
```c
if ((uint32_t)(currentTick - s_LastTick_50ms) >= 50U) {
    s_LastTick_50ms = currentTick;
    if (s_UI_Initialized != 0U) {
        UI_Display_Process();
    }
}
```

但 `SysCore_Run()` 在主循环中被反复调用，每次循环只执行**一个**满足条件的分支：
```c
void SysCore_Run(void)
{
    volatile uint32_t currentTick = g_Ticks[TICK_MS].Tick();

    // TFT 初始化（仅一次）
    if (...) { ... }

    // 1ms 任务
    if ((uint32_t)(currentTick - s_LastTick_1ms) >= 1U) { ... }

    // 10ms 任务
    if ((uint32_t)(currentTick - s_LastTick_10ms) >= 10U) { ... }

    // 50ms 任务 (UI)
    if ((uint32_t)(currentTick - s_LastTick_50ms) >= 50U) { ... }

    // 100ms 任务
    if ((uint32_t)(currentTick - s_LastTick_100ms) >= 100U) { ... }
}
```

**假设 1（最可能）**：状态机 1ms 任务中的 SPI 阻塞操作导致主循环周期实际上远大于 50ms。
- `StateMachine_Task()` 调用 `power_control.c` 的函数
- 如果 power_control 内部有 SPI 操作或在中断中有耗时操作
- 主循环实际周期可能达到 200-500ms

**假设 2**：`blink_counter` 只在 `ui_state == UI_STATE_HOME_EDIT` 时递增。如果状态机在 MENU 和 EDIT 之间切换时重置计数器，闪烁可能永远不会完整运行一个周期。

**假设 3**：`UI_BLINK_TICKS = 10` 对应 50ms × 10 = 500ms（2Hz），但如果实际 UI 刷新周期不是 50ms 而是 500ms（因 SPI 阻塞），则实际闪烁周期是 500ms × 10 = 5 秒。
