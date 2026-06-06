# 数控电源 UI 闪烁问题 + 光标闪烁 — V10 阶段提示词

## 📅 任务日期
2026-06-05

---

## 📊 上一阶段（V9）进度报告

### ✅ V9 代码修复（已应用）

| 修改 | 位置 | 内容 |
|------|------|------|
| 修改 1 | `UI_DrawCenteredAscii()` | 重写为固定宽度 Opaque 渲染，废弃 FillRect |
| 修改 2 | `UI_Cache_t` 结构体 | 添加 `uint8_t system_status` 字段 |
| 修改 3 | `UI_Display_Init()` | 初始化 `s_cache.system_status = UI_INVALID_U8` |
| 修改 4 | `UI_Display_Process()` | 添加 system_status 脏检查逻辑 |

### ❌ V9 硬件验证失败（2 个缺陷）

**缺陷 1：边框仍然被擦除**
- 现象：开机后 StaticFrame 绘制完边框线，状态栏字符出现时，底部垂直线（x=59, 119, 179）被擦除
- 说明：V9 的"固定宽度 Opaque 渲染"方案本身的假设有误

**缺陷 2：光标闪烁极其缓慢**
- 现象：进入 EDIT 模式后光标闪烁明显慢于预期的 2Hz
- 说明：调整 `UI_BLINK_TICKS` 常量无效，存在更深层的机制问题

---

# Agent 工具与规范要求 (极度重要)
为了保证工程质量，请你严格开启并遵循以下模式：
1. 开启 `using-superpowers` 模式以深度理解我的需求。
2. 开启 `planning-with-files` 和 `writing-plans`：在 Docs 目录生成并维护 `Project_Plan.md`（跟踪阶段和进度）、`Research_Log.md`（存储研究和发现）、`Test_Results.md`（会话日志和测试结果）。
3. 严格遵循 `karpathy-guidelines` 代码规范，保证代码的极致优雅、高内聚低耦合。

然后根据提示词 "Docs/UI_Flicker_V10_Prompt.md" 来工作。

---

# 数控电源 UI 问题 — V10 阶段提示词

## 🎯 当前任务目标

**分析 V9 硬件验证失败的两个核心问题，找出真正的根因，然后实施修复**

---

## 🔴 P0 — 修复边框仍然被擦除

### 缺陷描述

**文件**：`Layer/Product/ui_display.c`
**函数**：`UI_DrawCenteredAscii()`

**现象**：V9 已废弃显式的 `FillRect` 调用，改用 `DrawStringOpaque`。但硬件验证发现，初始化后状态栏字符绘制仍然擦除了垂直线边框。

### V9 失败根因分析（已由前置分析完成）

V9 的假设是 `DrawStringOpaque(x, y, padded_text, FG, BG, scale)` 精确覆盖 60px 宽的格子区域。但问题出在 **`DrawCharOpaque` 内部的字符间隙填充逻辑**：

```
文件：Layer/Driver/tft_gfx.c，函数 TFTGFX_DrawCharOpaque()，第 543-550 行

// 每个字符绘制完 5 列像素后，额外用 bg_color 填充 1px × scale 宽的间隙
if (scale == 1U) {
    TFTGFX_FillRect(x + 5, y, 1, 7, bg_color);
} else {
    TFTGFX_FillRect(x + (5 * scale), y, scale, (7 * scale), bg_color);
}
```

**scale=2 时的像素布局分析**：
```
每个字符 = 5列×2 + 1×2间隙 = 12px

Cell 0 (x=0, w=60, scale=2, max_chars=5) 填充为 "  ON  "：
字符0 (' '): x=0,  字符像素=[0,9],  间隙=[10,11]
字符1 (' '): x=12, 字符像素=[12,21], 间隙=[22,23]
字符2 ('O'): x=24, 字符像素=[24,33], 间隙=[34,35]
字符3 ('N'): x=36, 字符像素=[36,45], 间隙=[46,47]
字符4 (' '): x=48, 字符像素=[48,57], 间隙=[58,59]  ← 覆盖了 x=59 边框线！
```

**结论**：Opaque 渲染自身通过最后一个字符的间隙填充，用背景色覆盖了垂直线边框 x=59, x=119, x=179。

### 修复方向

**思路**：在 `UI_DrawCenteredAscii()` 中，构造填充字符串时预留 1 个字符宽度的边距（减少 max_chars），使 Opaque 渲染不触碰到格子边界。

```c
// 核心修改：max_chars 减少 1 个字符宽度，为边框线留出安全区
// 原：max_chars = w / (6 * scale)
// 改：max_chars = (w / (6 * scale)) - 1  或直接使用实际可用宽度
```

或者在构造的空白填充时右侧少填一格，让 DrawStringOpaque 覆盖范围比格子窄 1 个字符宽度。

**约束**：
- 保持函数签名不变
- 保持文本视觉居中
- 不修改 tft_gfx.c 驱动层（影响面太大）

---

## 🔴 P0 — 修复光标闪烁缓慢

### 缺陷描述

**文件**：`Layer/Product/ui_display.c`
**函数**：`UI_Display_Process()`（光标闪烁逻辑在第 167-182 行，光标绘制在第 238-257 行）
**调度文件**：`Layer/Scheduler/task_manager.c`

**现象**：进入 EDIT 模式后光标闪烁极其缓慢。用户已尝试调整 `UI_BLINK_TICKS` 但无效。

### 需要诊断的方向

**方向 1：主循环实际周期**
系统使用软件 SPI 驱动 TFT，每次 `DrawRect`/`FillRect` 都需要大量 SPI 传输。需检查：
- 1ms 任务中的 `StateMachine_Task()` 是否有阻塞操作
- `power_control.c` 中是否有 SPI 操作或长延迟
- 主循环的实际周期是多少（可通过 LED 翻转测量）

**文件**：`task_manager.c` 中的调度结构：
```c
// 每次 SysCore_Run() 调用只执行**一个**分支（叠加的 if 不是互斥的，但时间被稀释）
if (1ms)  { StateMachine_Task(); }   // 控制环路
if (10ms) { Key_Process(); }         // 按键
if (50ms) { UI_Display_Process(); }  // UI 刷新 ← 闪烁依赖此频率
```

关键问题：如果 `StateMachine_Task()` 每次执行耗时超过 1ms，或者 `Key_Process()` 有 SPI 读取，主循环的 50ms 判定可能出现显著偏差。

**方向 2：闪烁计数器逻辑**
当前代码在第 167-174 行：
```c
if (ui_state == UI_STATE_HOME_EDIT) {
    s_cache.blink_counter++;
    if (s_cache.blink_counter >= UI_BLINK_TICKS) {  // UI_BLINK_TICKS = 10
        s_cache.blink_counter = 0U;
        s_cache.blink_visible = !s_cache.blink_visible;
        blink_state_changed = 1U;
    }
}
```

如果主循环实际周期远大于 50ms（例如 200ms），则实际闪烁周期 = 200ms × 10 = 2 秒，给人"极其缓慢"的感觉。

**方向 3：闪烁绘制机制**
当前闪烁通过重绘三层 DrawRect（约 306 个像素的 SPI 传输）实现。如果 SPI 速度慢（软件 SPI ~500kHz），每次闪烁切换可能需要约 5-10ms 的 SPI 时间。

---

## 🟡 P1 — 全量回归验证

### 需要验证的点（V8–V9 已修复 + 新增）
1. 功率显示（V8.0）：数值平滑、无闪烁
2. 设置值显示（V8.0）：调节时无闪烁
3. 居中文本叠影（V8.1）：OFF↔ON 切换无重叠
4. 独立更新（V8.2）：未变化的格子保持静止
5. 初始化渲染（V8.3 + V9）：所有 4 个状态栏文本正常显示
6. 边框完整性（V10 新）：所有垂直线完整、水平线完整
7. 光标闪烁（V10 新）：2Hz 正常闪烁

---

## 📂 关键文件清单

| 文件 | 用途 | 状态 |
|------|------|------|
| `Layer/Product/ui_display.c` | UI 绘制逻辑（**主要修改目标**） | 🔴 待修复 |
| `Layer/Driver/tft_gfx.c` | 图形原语 + 字体渲染（**只读分析**） | 🟡 参考 |
| `Layer/Scheduler/task_manager.c` | 任务调度（**需分析实际周期**） | 🟡 参考 |
| `Layer/Control/power_control.c` | 功率控制（需检查是否阻塞主循环） | 🟡 参考 |
| `Layer/Driver/tft_driver.c` | TFT 硬件接口（软件 SPI） | 🟢 |
| `Layer/Application/state_machine.c` | 状态机 | 🟡 参考 |

---

## 🎯 V10 成功标准

1. ✅ 状态栏所有垂直线（x=59, 119, 179）在字符渲染后保持完整
2. ✅ 水平分隔线（y=210）在字符渲染后保持完整
3. ✅ 所有状态栏文本切换：无叠影 + 无闪烁
4. ✅ 光标以 2Hz 频率正常闪烁（约 500ms 周期）
5. ✅ 编译零错误零警告
6. ⏳ 用户硬件验证通过

---

## 📝 设计约束

1. **不要修改 tft_gfx.c**：这是底层驱动，V9/V10 问题应在 UI 层解决
2. **不要大重构**：只改 `UI_DrawCenteredAscii()` 和光标闪烁相关代码
3. **不要引入新依赖**：仅使用已有的 API
4. **保持向后兼容**：不改动 `UI_DrawCenteredAscii()` 的函数签名
5. **性能不退化**：SPI 传输量不显著增加

---

## 📚 相关文档（Docs 目录）

V8 系列修复报告：
- `Final_Fix_Report_V8.md` — V8.0 主报告
- `Hotfix_V8.1_Ghost_Text.md` — V8.1 叠影修复
- `Hotfix_V8.2_Status_Flicker.md` — V8.2 独立脏检查
- `Hotfix_V8.3_Border_Normal.md` — V8.3 normal 文本与边框

V9 修复报告：
- `V9_Fix_Report.md` — V9 固定宽度 Opaque 方案
- `UI_Flicker_V9_Prompt.md` — V9 任务提示词

功能文档：
- `Blink_Feature_Doc.md` — 光标闪烁 2Hz 设计文档

项目文档：
- `Project_Plan.md` — 项目进度
- `Research_Log.md` — 根因分析记录
- `Test_Results.md` — 测试结果

---

*生成时间: 2026-06-05*
*基础版本: V9*
*目标版本: V10*
*优先级: 最高（修复 V9 硬件验证失败的两个阻塞缺陷）*
