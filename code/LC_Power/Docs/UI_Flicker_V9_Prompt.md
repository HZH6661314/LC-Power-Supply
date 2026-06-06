# 数控电源 UI 闪烁问题 - V9 阶段提示词

## 📅 任务日期
2026-06-03

---

## 📊 上一阶段（V8.0→V8.3）进度报告

### ✅ 已完成

| 版本 | 修复内容 | 修改函数 |
|------|----------|----------|
| V8.0 | 消除 FillRect 预填充 | `UI_DrawPower()`, `UI_DrawFunctions()` |
| V8.1 | 修复居中文本叠影 | `UI_DrawCenteredAscii()` — FillRect + 恢复边框 |
| V8.2 | 独立脏检查消除群组闪烁 | `UI_Display_Process()` — 拆分状态栏更新 |
| V8.3 | 修复 normal 消失 + 边框恢复 | `UI_Display_Init()`, `UI_DrawCenteredAscii()` |

### ❌ 遗留缺陷

1. **边框恢复不完整**：`UI_DrawCenteredAscii()` 中 FillRect 覆盖右侧垂直线（x=59, 119, 179），仅恢复左侧线
2. **渲染策略脆弱**：FillRect + DrawString + 手动恢复边框 的三元组操作容易出错

### ⏳ 未涉及（来自 提示词.md 全局任务）

- 开机 UI 加载速度优化
- 快速设置子菜单 (QUICK_SET)
- 系统设置子菜单 (SYS_SET)
- 编辑态数值恢复（长按取消）
- W25Q256 Flash 存储
- 汉字渲染

---

# Agent 工具与规范要求 (极度重要)
为了保证工程质量，请你严格开启并遵循以下模式：
1. 开启 `using-superpowers` 模式以深度理解我的需求。
2. 开启 `planning-with-files` 和 `writing-plans`：在根目录生成并维护 `Project_Plan.md`（跟踪阶段和进度）、`Research_Log.md`（存储研究和发现）、`Test_Results.md`（会话日志和测试结果）。
3. 严格遵循 `karpathy-guidelines` 代码规范，保证代码的极致优雅、高内聚低耦合。

然后根据提示词 "Docs/UI_Flicker_V9_Prompt.md" 来工作。

---

# 数控电源 UI 闪烁问题 - V9 阶段提示词

## 🎯 当前任务目标

**彻底解决 V8.3 遗留的 2 个问题，并完成闪烁任务的最终验收**

---

## 🔴 高优先级：修复 V8.3 遗留缺陷

### 缺陷 1：状态栏右侧垂直线被 FillRect 擦除未恢复

**文件**：`Layer/Product/ui_display.c`  
**函数**：`UI_DrawCenteredAscii()`（第 486-513 行）

**根本原因**：
```
FillRect 覆盖范围：x=[x, x+w-1]，所以每条垂直线被两个相邻格子各覆盖一次：
- 线 x=59: 被 Cell 0 (x=0,w=60) FillRect 的右边界覆盖
- 线 x=119: 被 Cell 1 (x=60,w=60) FillRect 的右边界覆盖
- 线 x=179: 被 Cell 2 (x=120,w=60) FillRect 的右边界覆盖

当前代码只恢复了"当前格子的左边界线"（x=60/120/180），
但 FillRect 覆盖的是"当前格子的右边界线"（x=59/119/179）。
```

**修复方向**：
- **方案 A（推荐）**：废弃 FillRect+恢复边框 策略，改用**固定宽度 Opaque 渲染**
  ```c
  // 将文本填充为固定宽度（左对齐+右空格）
  char padded[12];
  int max_chars = w / (6 * scale);
  snprintf(padded, sizeof(padded), "%-*s", max_chars, text);
  // Opaque 模式在固定位置左边对齐绘制 → 完整覆盖无残留
  TFTGFX_DrawStringOpaque(x, draw_y, padded, fg, bg, scale);
  ```
  优势：无需 FillRect、无需手动恢复边框、真正的"直接覆盖"

- **方案 B**：补全边框恢复逻辑（治标）
  ```c
  // 恢复右侧垂直线（当前格子的右边界 = 下一格子的左边界）
  if (x == 0 || x == 60 || x == 120) {
      TFTGFX_DrawLine(x + w - 1, 210, x + w - 1, 239, FG);
  }
  ```

### 缺陷 2："normal" 文本是静态文本，改变 UI 状态后可能不更新

**文件**：`Layer/Product/ui_display.c`  
**函数**：`UI_Display_Process()`（第 135-246 行）

**问题**：如果将来 UI 状态变化导致"normal"需要变为其他内容（如"OTP"、"OCP"），当前代码不会更新它。

**修复方向**：为"normal"添加脏检查（从状态机获取 system_status 值）

---

## 🟡 中优先级：验证 V8.0-V8.3 所有修复点

### 需要逐项硬件验证

1. **功率显示**（V8.0）：数值变化时是否平滑？
2. **设置值显示**（V8.0）：调节电压/电流时是否闪烁？
3. **居中文本叠影**（V8.1）：OFF↔ON 切换文字是否重叠？
4. **独立更新**（V8.2）：快速切换时，未变化的格子是否保持静止？
5. **初始化渲染**（V8.3）：上电后所有 4 个状态栏文本是否显示？

---

## 🟢 低优先级：整体渲染策略优化

### 优化方向：Opaque-First 策略

**原则**：能用 Opaque 就不用 FillRect

| 场景 | 旧方案 | 新方案 |
|------|--------|--------|
| 右对齐数值 | ✅ Opaque（不变） | ✅ 保持 |
| 固定位置文本 | ✅ Opaque（不变） | ✅ 保持 |
| **居中变宽文本** | ⚠️ FillRect + DrawString + 恢复边框 | ✅ 固定宽度 Opaque |
| 进度条 | FillRect 内部（合理） | ✅ 保持 |

### 可选的架构改进

将 `UI_DrawCenteredAscii` 拆分为两个功能：
1. 状态栏专用版本：知道确切布局，直接 Opaque
2. 通用居中版本：用于未来其他场景

---

## 📂 关键文件清单

| 文件 | 用途 | 状态 |
|------|------|------|
| `Layer/Product/ui_display.c` | UI 绘制（**主要修改目标**） | 🔴 待修复 |
| `Layer/Product/ui_display.h` | UI 接口 | 🟢 |
| `Layer/Driver/tft_gfx.c` | 图形原语 API | 🟢 |
| `Layer/Driver/tft_gfx.h` | 图形原语头文件 | 🟢 |
| `Layer/Scheduler/task_manager.c` | 任务调度（UI 50ms） | 🟢 |
| `Layer/Application/state_machine.c` | 状态机 | 🟡 |
| `Docs/数控电源代码规范.md` | 编码规范 | 🟢 |
| `Docs/Task_Handover_UI_Flicker.md` | 原始任务描述 | 🟢 |

---

## 🎯 V9 成功标准

1. ✅ 状态栏右侧垂直线（x=59, 119, 179）在任意格子更新后保持完整
2. ✅ 水平分隔线（y=210）在任意格子更新后保持完整
3. ✅ 所有状态栏文本切换：无叠影 + 无闪烁
4. ✅ 方案 A 实现或方案 B 实现均可（推荐 A）
5. ✅ 编译零错误零警告
6. ⏳ 用户验证通过（硬件实测）

---

## 📝 设计约束

1. **不要大重构**：只改 `UI_DrawCenteredAscii()` 或状态栏相关代码
2. **不要引入新依赖**：仅使用已有的 `tft_gfx.c` API
3. **保持性能**：单次更新的 SPI 传输量不能显著增加
4. **向后兼容**：不改动 `UI_DrawCenteredAscii()` 的函数签名（它也被其他地方调用）

---

## 📚 相关文档（Docs 目录）

V8 系列修复报告：
- `Final_Fix_Report_V8.md` — V8.0 主报告
- `Hotfix_V8.1_Ghost_Text.md` — V8.1 叠影修复
- `Hotfix_V8.2_Status_Flicker.md` — V8.2 独立脏检查
- `Hotfix_V8.3_Border_Normal.md` — V8.3 normal 文本与边框

历史文档：
- `Task_Handover_UI_Flicker.md` — 原始任务
- `UI_Flicker_Fix_Plan.md` — 早期方案
- `提示词.md` — 全局项目需求
- `数控电源代码规范.md` — 编码规范

---

*生成时间: 2026-06-03*  
*基础版本: V8.3*  
*目标版本: V9*  
*优先级: 最高（修复阻塞验收的缺陷）*
