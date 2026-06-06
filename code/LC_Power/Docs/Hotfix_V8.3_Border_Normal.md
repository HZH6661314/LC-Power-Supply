# V8.3 修复：normal文本消失和边框缺失

## 🐛 用户反馈的问题

1. **"normal"文本消失**：电源状态显示区域没有"normal"文本
2. **边框不完整**：状态栏分隔线被擦除

---

## 🔍 根本原因分析

### 问题1："normal"文本消失

**原因**：V8.2修改后，状态栏文本只在值变化时重绘，但"normal"是静态文本，永远不会变化，所以永远不会被绘制。

**代码逻辑**：
```c
// UI_Display_Init() 初始化：
s_cache.output_enabled = UI_INVALID_U8;  // 0xFF
s_cache.cvcc_cc = UI_INVALID_U8;         // 0xFF
s_cache.temperature_c = UI_INVALID_U16;  // 0xFFFF

UI_DrawStaticFrame();  // 绘制边框
UI_Display_Process();  // 第一次Process

// 在 UI_Display_Process() 中：
output_enabled = SM_Get_OutputEnabled();  // 假设是 0
cc_mode = SM_Get_CCMode();                // 假设是 0
temperature_c = SM_Get_TemperatureC();    // 假设是 25

// 独立脏检查：
if (s_cache.output_enabled != output_enabled) {  // 0xFF != 0 → TRUE
    绘制 OFF/ON
}

if (s_cache.cvcc_cc != cc_mode) {  // 0xFF != 0 → TRUE
    绘制 CV/CC
}

if (s_cache.temperature_c != temperature_c) {  // 0xFFFF != 25 → TRUE
    绘制 温度
}

// ❌ 没有代码绘制 "normal" 文本！
```

### 问题2：边框被覆盖

**原因**：`UI_DrawCenteredAscii()`中的`FillRect`覆盖了状态栏分隔线。

**布局分析**：
```
状态栏区域：y = 210-239
- 顶部分隔线：y = 210 (水平线)
- 垂直分隔线：x = 59, 119, 179

FillRect执行：
- 计算 draw_y = 210 + (30 - 14) / 2 = 210 + 8 = 218
- 但实际上某些情况下可能从 draw_y 开始向上或向下覆盖

问题：FillRect(x, draw_y, w, text_h) 可能覆盖 y=210 的线
```

---

## ✅ 应用的修复

### 修复1：初始化时显式绘制完整状态栏

**文件**：`ui_display.c`  
**位置**：`UI_Display_Init()` 函数（第127-131行）

**修改前**：
```c
UI_DrawStaticFrame();
UI_Display_Process();
```

**修改后**：
```c
UI_DrawStaticFrame();

// 初始化时绘制完整状态栏（包括静态的"normal"文本）
UI_DrawStatus(0U, 0U, 25U);

UI_Display_Process();
```

**原理**：
- 显式调用`UI_DrawStatus()`绘制所有4个文本
- 后续`UI_Display_Process()`只更新变化的文本
- "normal"作为静态文本在初始化后永不更新

### 修复2：FillRect后恢复边框线

**文件**：`ui_display.c`  
**位置**：`UI_DrawCenteredAscii()` 函数（第486-513行）

**修改后**：
```c
static void UI_DrawCenteredAscii(int16_t x, int16_t y, int16_t w, const char *text, uint16_t color, uint8_t scale)
{
    uint16_t text_w = TFTGFX_MeasureStringWidth(text, scale);
    int16_t text_h = (int16_t)(7U * scale);
    int16_t draw_x = (int16_t)(x + ((w - (int16_t)text_w) / 2));
    int16_t draw_y = (int16_t)(y + ((UI_STATUS_H - text_h) / 2));

    // 清除区域避免叠影
    TFTGFX_FillRect(x, draw_y, w, text_h, UI_COLOR_BG);

    // 绘制居中文本
    TFTGFX_DrawString(draw_x, draw_y, text, color, scale);

    // ✅ 恢复被覆盖的分隔线（状态栏分隔线在y=210）
    if (y == 210) {
        // 水平分隔线
        TFTGFX_DrawLine(x, 210, (int16_t)(x + w - 1), 210, UI_COLOR_FG);

        // 垂直分隔线（如果在边界）
        if (x == 60 || x == 120 || x == 180) {
            // 绘制左侧垂直分隔线
            TFTGFX_DrawLine((int16_t)(x - 1), 210, (int16_t)(x - 1), 239, UI_COLOR_FG);
        }
    }
}
```

**原理**：
- 检测是否在状态栏（y == 210）
- FillRect后重新绘制被覆盖的水平线和垂直线
- 确保边框始终完整

---

## 📊 修复效果

### 状态栏布局
```
┌────────────────────────────────────────┐
│  0      60     120     180       240  │
├────────┼───────┼───────┼───────────────┤  ← y=210 水平线
│  OFF   │  CV   │  25C  │    normal    │
│   ON   │  CC   │       │              │
└────────┴───────┴───────┴───────────────┘
         ↑       ↑       ↑
      x=59    x=119   x=179
      垂直分隔线
```

### 修复后的行为

1. **初始化阶段**：
   - `UI_DrawStaticFrame()` → 绘制所有边框
   - `UI_DrawStatus(0, 0, 25)` → 绘制4个文本（包括"normal"）
   - `UI_Display_Process()` → 第一次脏检查（所有值都不同，会重绘，但"normal"已在）

2. **运行阶段**：
   - 仅变化的文本重绘
   - FillRect后自动恢复边框
   - "normal"永不重绘

---

## 🧪 测试验证

### 测试1：初始化检查
**步骤**：
1. 上电或复位
2. 观察状态栏

**预期**：
- ✅ 4个区域都有文本：OFF/ON、CV/CC、温度、normal
- ✅ 所有分隔线完整清晰

### 测试2：动态更新检查
**步骤**：
1. 快速切换输出10次
2. 观察状态栏边框

**预期**：
- ✅ OFF/ON切换时，边框自动恢复
- ✅ 分隔线始终完整

### 测试3：所有格子检查
**步骤**：
1. 切换输出（OFF↔ON）
2. 改变负载（CV↔CC）
3. 等待温度变化
4. 观察"normal"区域

**预期**：
- ✅ 每个格子独立更新
- ✅ "normal"始终显示
- ✅ 边框始终完整

---

## 📝 技术细节

### 为什么需要显式调用UI_DrawStatus()

**V8.2的设计缺陷**：
```c
// 独立脏检查设计：
if (变化) { 重绘; }

// 对于静态内容：
"normal"永远不变 → 永远不会进入if分支 → 永远不会绘制
```

**修复策略**：
- 初始化时全绘一次（调用`UI_DrawStatus()`）
- 运行时按需更新（独立脏检查）
- 静态内容"normal"在初始化后永不需要更新

### 边框恢复的精确性

**状态栏分隔线位置**：
```
水平线：y = 210（整行）
垂直线：
- x = 59  (第1和第2格之间)
- x = 119 (第2和第3格之间)
- x = 179 (第3和第4格之间)
```

**恢复逻辑**：
```c
if (y == 210) {  // 确认在状态栏
    // 恢复水平线（当前格子的部分）
    DrawLine(x, 210, x+w-1, 210);
    
    // 恢复左侧垂直线（如果当前格子在分隔线右侧）
    if (x == 60 || x == 120 || x == 180) {
        DrawLine(x-1, 210, x-1, 239);
    }
}
```

---

## 🔄 与之前修复的关系

| 版本 | 修复内容 | 引入问题 | 本次解决 |
|------|----------|----------|----------|
| V8.1 | 叠影（FillRect清除） | 引入边框覆盖风险 | ✅ 边框恢复 |
| V8.2 | 闪烁（独立脏检查） | "normal"消失 | ✅ 初始化全绘 |
| V8.3 | **两个新问题** | - | ✅ 完整修复 |

---

## ✅ 修复总结

**修改文件**: `Layer/Product/ui_display.c`  
**修改位置**: 
1. `UI_Display_Init()` - 添加`UI_DrawStatus()`调用
2. `UI_DrawCenteredAscii()` - 添加边框恢复逻辑

**修改行数**: 约15行

**解决问题**:
1. ✅ "normal"文本现在会显示
2. ✅ 状态栏分隔线始终完整
3. ✅ 保持V8.2的无闪烁特性

---

*修复时间: 2026-06-03*  
*版本: V8.3*  
*状态: 已应用，待用户验证*  
*预期: normal文本显示 + 边框完整 + 无闪烁*
