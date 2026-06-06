# V8.1 紧急修复：状态栏叠影问题

## 🐛 用户反馈问题

**现象**: "OFF" ↔ "ON" 状态切换有叠影  
**严重性**: 高（影响用户体验）  
**发现时间**: 2026-06-03（V8硬件测试后）

---

## 🔍 根本原因分析

### 问题代码
```c
static void UI_DrawCenteredAscii(int16_t x, int16_t y, int16_t w, const char *text, uint16_t color, uint8_t scale)
{
    uint16_t text_w = TFTGFX_MeasureStringWidth(text, scale);
    int16_t draw_x = (int16_t)(x + ((w - (int16_t)text_w) / 2));
    int16_t draw_y = (int16_t)(y + ((UI_STATUS_H - (int16_t)(7U * scale)) / 2));

    // ❌ 问题：不同长度文本居中位置不同
    TFTGFX_DrawStringOpaque(draw_x, draw_y, text, color, UI_COLOR_BG, scale);
}
```

### 为什么产生叠影

**文本宽度不同**:
```
"OFF" → 3字符 × 6px × 2倍缩放 = 36px宽
"ON"  → 2字符 × 6px × 2倍缩放 = 24px宽
```

**居中位置计算**（容器宽度60px）:
```
"OFF" 绘制位置: x + (60-36)/2 = x + 12
"ON"  绘制位置: x + (60-24)/2 = x + 18
```

**叠影形成过程**:
```
步骤1: 绘制 "OFF" 在 x=12 位置
       [    OFF    ]
       ^12  ^48
       
步骤2: 切换为 "ON"，绘制在 x=18 位置
       [    ON     ]
            ^18 ^42
       
结果:  "OFF" 的 "F" 左边缘（x=12-18）未被覆盖
       [  F  ON    ]
          ^叠影
```

### 图解说明

```
容器区域 [0------------------------60]

T0: 绘制 "OFF"
    [          OFF          ]
    ^12      ^24  ^36     ^48

T1: 切换为 "ON" (DrawStringOpaque只覆盖自己的24px)
    [          OFF          ]
    ^12      ^18   ^30 ^42  ^48
             [  ON  ]
             ↑ 只覆盖这个区域
             
实际显示:
    [     F    ON     ]
          ↑叠影残留
```

---

## ✅ 应用的修复

### 修复后的代码
```c
static void UI_DrawCenteredAscii(int16_t x, int16_t y, int16_t w, const char *text, uint16_t color, uint8_t scale)
{
    uint16_t text_w = TFTGFX_MeasureStringWidth(text, scale);
    int16_t text_h = (int16_t)(7U * scale);
    int16_t draw_x = (int16_t)(x + ((w - (int16_t)text_w) / 2));
    int16_t draw_y = (int16_t)(y + ((UI_STATUS_H - text_h) / 2));

    // ✅ 修复：先清除整个容器区域
    TFTGFX_FillRect(x, draw_y, w, text_h, UI_COLOR_BG);

    // 再绘制居中文本
    TFTGFX_DrawString(draw_x, draw_y, text, color, scale);
}
```

### 修复原理

**新的渲染流程**:
```
容器区域 [0------------------------60]

步骤1: FillRect清除整个区域（60px宽）
       [                            ]
       ← 全部填充背景色

步骤2: 绘制居中文本
       [           ON              ]
                   ↑ 任何位置都不会有残留
```

### 为什么这样修复

1. **完全覆盖**: FillRect清除整个容器宽度，确保旧内容100%清除
2. **无残留**: 无论新文本宽度多少，都不会有叠影
3. **闪烁可控**: FillRect区域很小（60px × 14px = 840像素 @ 2倍缩放），闪烁时间约0.6ms，肉眼不可察觉

---

## 📊 性能影响分析

### 绘制时间估算（SPI ~2MHz）

**修复前**（仅DrawStringOpaque）:
```
"ON": 2字符 × (5×7像素 × 4倍缩放) = 2 × 140像素 × 2字节 = 560字节
时间: 560字节 / 250KB/s ≈ 2.2ms
```

**修复后**（FillRect + DrawString）:
```
FillRect: 60px × 14px = 840像素 × 2字节 = 1680字节 → 6.7ms
DrawString: 2字符 × 140像素 × 2字节 = 560字节 → 2.2ms
总计: 8.9ms
```

**额外开销**: 约6.7ms（FillRect部分）

### 闪烁风险评估

**理论闪烁时间**: 6.7ms  
**人眼闪烁阈值**: 约16ms（60Hz）  
**结论**: **不会产生可察觉的闪烁**

原因：
1. 区域很小（仅60×14像素）
2. FillRect后立即DrawString，间隙极小
3. 6.7ms远低于人眼闪烁感知阈值（16ms）

---

## 🎯 其他受影响的文本

### 需要相同修复的场景

检查其他使用`UI_DrawCenteredAscii()`的地方：

1. **"CV" ↔ "CC"** - ✅ 宽度相同（2字符），无叠影风险
   ```c
   UI_DrawCenteredAscii(60, 210, 60, (cc_mode != 0U) ? "CC" : "CV", UI_COLOR_FG, 2U);
   ```

2. **温度显示** - ⚠️ 宽度可变，可能有叠影
   ```c
   snprintf(temp_text, sizeof(temp_text), "%uC", temperature_c);
   // "25C" (3字符) vs "100C" (4字符)
   UI_DrawCenteredAscii(120, 210, 60, temp_text, UI_COLOR_FG, 2U);
   ```
   **评估**: 温度变化缓慢，且通常在相邻值（如25→26），叠影不明显，但理论上存在

3. **"normal"** - ✅ 静态文本，无切换，无风险
   ```c
   UI_DrawCenteredAscii(180, 210, 60, "normal", UI_COLOR_FG, 1U);
   ```

### 结论
当前修复已解决所有调用`UI_DrawCenteredAscii()`的叠影问题，包括温度显示。

---

## 🔄 修复前后对比

### 修复前
```
状态切换: "OFF" → "ON"
结果: [  F  ON  ]  ← 叠影
```

### 修复后
```
状态切换: "OFF" → "ON"
结果: [    ON   ]  ← 清晰
```

---

## 🧪 测试验证

### 测试步骤
1. 快速按SET键切换输出10次
2. 观察状态栏第一格显示

**预期结果**:
- ✅ "OFF" 和 "ON" 切换清晰
- ✅ 无叠影残留
- ✅ 无可察觉闪烁（6.7ms << 16ms阈值）

**注意事项**:
- 如果用户仍然看到"闪烁"，请区分：
  - **叠影**（文字重叠）- 本次修复已解决
  - **白色闪烁**（背景色闪现）- 需要进一步分析

---

## 📝 经验教训

### 1. Opaque渲染的局限性
**教训**: `DrawStringOpaque`仅覆盖文本自身区域，对于：
- ✅ **固定位置绘制**（如数值右对齐）- 完美工作
- ❌ **居中对齐**（位置随宽度变化）- 会产生叠影

### 2. 居中文本的正确模式
对于居中显示的可变宽度文本：
```c
// ✅ 正确模式
FillRect(container_x, y, container_width, text_height, BG_COLOR);
DrawString(centered_x, y, text, FG_COLOR, scale);
```

### 3. 性能与正确性的权衡
- 小区域FillRect（<1000像素）开销可接受（<7ms）
- 大区域FillRect（>10000像素）需要避免
- 本次修复在可接受范围内

---

## ✅ 修复总结

**修改文件**: `Layer/Product/ui_display.c`  
**修改函数**: `UI_DrawCenteredAscii()`  
**修改行数**: 约10行  
**影响范围**: 状态栏所有居中文本（4处调用）  
**性能影响**: 每次刷新增加约6.7ms（可接受）  
**闪烁风险**: 极低（6.7ms远低于16ms阈值）  

---

*修复时间: 2026-06-03*  
*版本: V8.1*  
*状态: 已应用，待用户验证*
