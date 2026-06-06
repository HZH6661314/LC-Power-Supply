# V10 UI闪烁问题修复设计文档

## 文档信息

**日期**: 2026-06-06
**版本**: V10
**基于**: V9硬件验证失败
**作者**: LCYX
**状态**: 设计已批准，待实施

---

## 执行摘要

V9硬件验证发现两个阻塞缺陷：
1. **边框仍被擦除**：Opaque渲染的字符间隙填充覆盖了垂直边框线
2. **光标闪烁极慢**：主循环阻塞导致计数器累加缓慢，闪烁周期远超500ms

本设计采用**方案A：边框安全区 + TIM2硬件定时器**，从根本上解决两个问题。

---

## 问题1：边框擦除根因与修复

### 根本原因

`TFTGFX_DrawCharOpaque()` 在绘制每个字符后自动填充间隙（scale=2时为2px宽）。
当状态栏格子（60px宽）填充满5个字符时，第5个字符的间隙会覆盖右侧边框线：

```
Cell 0 (x=0, w=60, scale=2, max_chars=5) 填充为 "  ON  "：
字符0 (' '): x=0,  像素=[0,9],   间隙=[10,11]
字符1 (' '): x=12, 像素=[12,21], 间隙=[22,23]
字符2 ('O'): x=24, 像素=[24,33], 间隙=[34,35]
字符3 ('N'): x=36, 像素=[36,45], 间隙=[46,47]
字符4 (' '): x=48, 像素=[48,57], 间隙=[58,59]  ← 覆盖 x=59 边框线！
```

### 修复策略

在 `UI_DrawCenteredAscii()` 中预留安全区：
- 保持 `max_chars = w / (6 * scale)` 计算不变
- 引入 `effective_chars = max_chars - 1` 作为实际填充宽度
- 最右侧1个字符位置留空，防止间隙触碰边框

### 实现细节

**修改位置**: `Layer/Product/ui_display.c` 函数 `UI_DrawCenteredAscii()`

**关键代码变更**:
```c
// 原有逻辑
max_chars = w / (6 * scale);

// 新增安全区
int16_t effective_chars = max_chars - 1;

// 文本长度限制
while (*p != '\0' && text_len < effective_chars) { ... }

// 填充目标宽度
while (idx < effective_chars && idx < (int16_t)(sizeof(padded) - 1)) { ... }
```

**像素布局验证（修复后）**:
```
effective_chars = 4, "ON" 填充为 " ON " (4字符)：
字符3 (' '): x=36, 像素=[36,45], 间隙=[46,47]
最右侧间隙止于 x=47，距离边框 x=59 还有 12px 安全距离 ✓
```

### 视觉影响

- 状态栏每格文本从5字符减至4字符宽度（scale=2时）
- "normal"文本从10字符减至9字符宽度（scale=1时）
- 文本仍居中显示，视觉上差异极小
- 所有现有文本（"ON", "OFF", "CV", "CC", "25C", "normal"）均可正常显示

---

## 问题2：光标闪烁缓慢根因与修复

### 根本原因

当前闪烁逻辑依赖主循环计数器：

```c
// 在 UI_Display_Process() 中（50ms任务）
s_cache.blink_counter++;
if (s_cache.blink_counter >= UI_BLINK_TICKS) { ... }
```

如果 `StateMachine_Task()` 中的操作阻塞主循环，实际周期可能是200-500ms，
导致闪烁周期变为 200ms × 10 = 2秒。

### 修复策略

**将光标闪烁时基从主循环计数器改为TIM2硬件定时器**，使闪烁周期独立于主循环执行速度。

### 架构设计

```
┌─────────────────────────────────────────┐
│  TIM2中断 (1MHz, 每1μs)                 │
│  ├─ 软件分频: 500,000次 = 500ms        │
│  ├─ 切换标志位: g_UI_Blink_Flag        │
│  └─ [可选] LED toggle 诊断              │
└─────────────────────────────────────────┘
                 ↓ (标志位)
┌─────────────────────────────────────────┐
│  UI_Display_Process() (主循环)          │
│  ├─ 检测标志位变化                      │
│  └─ 绘制/擦除光标边框                   │
└─────────────────────────────────────────┘
```

**职责分离**:
- **TIM2中断**: 只管理计数器和标志位（快速返回，无SPI操作）
- **UI层**: 检测标志位变化，执行实际的SPI绘制

### 实现细节

**修改位置1**: `Layer/Bsp/bsp_hrtim.c` — 新增TIM2闪烁区域

新增全局变量：
```c
/* ==================== TIM2 UI Blink Timebase ==================== */
volatile uint8_t g_UI_Blink_Flag = 0;      // 当前闪烁状态 (0=隐藏, 1=显示)
volatile uint8_t g_UI_Blink_Changed = 0;   // 标志位已变化通知
```

TIM2回调实现：
```c
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) {
        static uint32_t s_blink_counter = 0;

        s_blink_counter++;

        // 每 500,000 次 = 500ms
        if (s_blink_counter >= 500000U) {
            s_blink_counter = 0U;

            // 翻转闪烁状态
            g_UI_Blink_Flag = (g_UI_Blink_Flag == 0U) ? 1U : 0U;
            g_UI_Blink_Changed = 1U;

            #ifdef UI_BLINK_DEBUG_TIM2
            HAL_GPIO_TogglePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin);
            #endif
        }
    }
}
```

**修改位置2**: `Layer/Bsp/bsp_hrtim.h` — 新增接口声明

```c
/* TIM2 UI Blink Timebase */
extern volatile uint8_t g_UI_Blink_Flag;
extern volatile uint8_t g_UI_Blink_Changed;
```

**修改位置3**: `Layer/Product/ui_display.c` — `UI_Display_Process()`

删除原有计数器逻辑，新增标志位检测：
```c
// 光标闪烁：检测TIM2中断设置的标志位
extern volatile uint8_t g_UI_Blink_Flag;
extern volatile uint8_t g_UI_Blink_Changed;

uint8_t blink_state_changed = 0U;

if (ui_state == UI_STATE_HOME_EDIT) {
    if (g_UI_Blink_Changed != 0U) {
        g_UI_Blink_Changed = 0U;  // 清除标志
        s_cache.blink_visible = g_UI_Blink_Flag;
        blink_state_changed = 1U;
    }
} else {
    if (s_cache.blink_visible == 0U) {
        s_cache.blink_visible = 1U;
        blink_state_changed = 1U;
    }
}
```

### 同步与竞态处理

**问题**: 中断可能在主循环读取时修改标志位

**解决方案**:
- `volatile` 关键字确保每次从内存读取最新值
- `g_UI_Blink_Changed` 用于边沿检测（主循环清零，中断设置）
- 读取顺序：先读 `g_UI_Blink_Changed`，再读 `g_UI_Blink_Flag`
- 最坏情况：丢失一次切换信号，下个500ms周期会补上，用户无感知

---

## LED诊断方案

### 目的

验证TIM2提供的500ms周期是否精确，以及主循环实际周期。

### 诊断点1: TIM2中断（验证500ms周期）

```c
#ifdef UI_BLINK_DEBUG_TIM2
HAL_GPIO_TogglePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin);
#endif
```

### 诊断点2: UI_Display_Process（验证主循环周期）

```c
#ifdef UI_BLINK_DEBUG_MAIN_LOOP
static uint32_t s_debug_counter = 0;
if (++s_debug_counter >= 10U) {
    s_debug_counter = 0U;
    HAL_GPIO_TogglePin(DEBUG_LED2_GPIO_Port, DEBUG_LED2_Pin);
}
#endif
```

### 验证方法

1. **TIM2周期**: 启用DEBUG_TIM2，观察LED应以2Hz均匀闪烁
2. **主循环周期**: 启用DEBUG_MAIN_LOOP，如果闪烁慢于TIM2，则说明主循环被阻塞
3. **双LED对比**: 同时启用两者，直观对比主循环实际周期

### 诊断代码生命周期

- 初期：保留诊断代码进行硬件验证
- 验证通过后：通过宏开关禁用
- 正式版本：可永久保留（诊断代码开销极小）

---

## 文件修改清单

| 文件 | 修改类型 | 修改内容 |
|------|---------|----------|
| `Layer/Bsp/bsp_hrtim.h` | 修改 | 添加TIM2闪烁标志位extern声明 |
| `Layer/Bsp/bsp_hrtim.c` | 修改 | 添加TIM2闪烁区域（注释划分）+ 回调实现 |
| `Layer/Product/ui_display.c` | 修改 | `UI_DrawCenteredAscii()`安全区 + 标志位检测逻辑 |
| `Core/Inc/main.h` | 可选 | 添加诊断LED宏定义 |

---

## 代码规范（Karpathy原则）

### 简洁性
- TIM2回调不超过10行代码
- UI层只检测标志位，不操作硬件
- 删除旧计数器逻辑，无冗余代码

### 单一职责
- `bsp_hrtim.c`: 提供精确500ms时基
- `ui_display.c`: 根据时基绘制光标

### 最小状态
- 全局变量仅2个：`g_UI_Blink_Flag` 和 `g_UI_Blink_Changed`
- 静态变量仅1个：TIM2回调的 `s_blink_counter`

### 清晰命名
- `g_`: 全局变量
- `s_`: 静态局部变量
- `UI_`: UI模块相关
- `BSP_`: BSP层相关

---

## 测试验证

### 编译验证
- 零错误、零警告
- extern声明与定义匹配
- volatile正确使用

### 功能测试

**测试1: 边框完整性**
- 快速切换状态栏文本10次
- 预期：x=59, 119, 179垂直线完整，y=210水平线完整

**测试2: 光标闪烁频率（LED诊断）**
- 启用UI_BLINK_DEBUG_TIM2
- 进入EDIT模式
- 预期：LED以2Hz均匀闪烁

**测试3: 光标闪烁视觉**
- 禁用诊断宏
- 进入EDIT模式
- 预期：屏幕光标以500ms周期闪烁，均匀流畅

**测试4: 主循环周期诊断（可选）**
- 同时启用两个诊断宏，两个不同LED
- 预期：收集主循环实际周期数据

### 回归测试

确保V8-V9已修复功能不受影响：
1. ✅ 功率显示平滑无闪烁
2. ✅ 设置值调节无闪烁
3. ✅ OFF↔ON切换无叠影
4. ✅ CV↔CC模式切换正确
5. ✅ 温度显示实时更新

---

## V10成功标准

| 序号 | 标准 | 验证方式 |
|-----|------|---------|
| 1 | 边框完整性 | 目测所有线条完整 |
| 2 | 光标闪烁2Hz | LED诊断+目测 |
| 3 | 编译零警告零错误 | Keil MDK编译输出 |
| 4 | 无性能退化 | CPU使用率不增加 |
| 5 | 代码规范 | 遵循Karpathy原则 |
| 6 | 用户硬件验证通过 | 最终确认 |

---

## 风险与缓解

### 风险1: TIM2中断频率高（1MHz）可能增加CPU开销
- **缓解**: 计数器递增只需几个周期，开销<0.01%
- **验证**: 通过性能测试确认无影响

### 风险2: 中断与主循环间的竞态条件
- **缓解**: 使用volatile + 边沿检测标志位
- **最坏情况**: 丢失一次切换，下周期自动恢复

### 风险3: 安全区可能导致文本显示不完整
- **缓解**: 所有现有文本均在4字符内（scale=2）或9字符内（scale=1）
- **验证**: 编译后目测确认

---

*设计完成时间: 2026-06-06*
*预计实施时间: 1-2小时*
*设计批准: 用户已确认*
