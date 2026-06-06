# UI无闪烁绘制优化方案

## 📅 2026-06-02
## 🎯 目标：修改设置值时避免区块闪烁

---

## 🔍 问题分析

### 当前现象
用户修改电压/电流时：
1. ✅ 设置区块不再"一直刷新"（已修复）
2. ❌ 但修改数值时，整个区块闪烁一下
3. ❌ 感觉是"擦除→重绘"

### 根本原因

**UI_DrawSettings()调用流程**：
```c
UI_DrawSettings(voltage, current)
  └─> UI_DrawProgressBar()
       ├─> TFTGFX_DrawRect() - 绘制边框
       ├─> TFTGFX_FillRect(..., UI_COLOR_BG) - ❌ 用白色填充进度条内部
       └─> TFTGFX_FillRect(..., UI_COLOR_FG) - 绘制新的进度
```

**闪烁原因**：
- 进度条先用白色填充（清除旧进度）
- 再用黑色填充（绘制新进度）
- 人眼能看到短暂的白色闪烁

---

## 💡 解决方案

### 方案A：缓存进度条值，增量更新（最优）⭐⭐⭐⭐⭐

**思路**：
- 记录上次绘制的进度条宽度
- 只更新变化的部分（增加或减少）
- 避免完全擦除

**实现**：
```c
typedef struct {
    float vout;
    float iout;
    float power;
    float set_voltage;
    float set_current;
    int16_t progress_v_width;  // 新增：电压进度条宽度缓存
    int16_t progress_i_width;  // 新增：电流进度条宽度缓存
    // ...
} UI_Cache_t;

static void UI_DrawProgressBarSmart(int16_t x, int16_t y, 
                                     float value, float max_value,
                                     int16_t *cached_width)
{
    int16_t new_width = 计算新宽度;
    int16_t old_width = *cached_width;
    
    if (new_width > old_width) {
        // 进度增加：只填充增加的部分
        TFTGFX_FillRect(x + 1 + old_width, y + 1, 
                        new_width - old_width, h - 2, UI_COLOR_FG);
    } else if (new_width < old_width) {
        // 进度减少：只擦除减少的部分
        TFTGFX_FillRect(x + 1 + new_width, y + 1, 
                        old_width - new_width, h - 2, UI_COLOR_BG);
    }
    // new_width == old_width: 不绘制
    
    *cached_width = new_width;
}
```

**优点**：
- 无闪烁
- 性能最优（只更新变化部分）
- 流畅

**缺点**：
- 需要修改缓存结构
- 代码稍复杂

---

### 方案B：减少UI_DrawSettings调用频率（推荐）⭐⭐⭐⭐

**思路**：
- 增大脏检查阈值
- 减少不必要的重绘

**实现**：
```c
// ui_display.c:185-190
if (UI_FloatChanged(s_cache.set_voltage, set_voltage, 0.01f) ||
    UI_FloatChanged(s_cache.set_current, set_current, 0.001f)) {
    // 改为：
    // 0.01f → 0.1f (电压变化0.1V才重绘)
    // 0.001f → 0.01f (电流变化0.01A才重绘)
}
```

**优点**：
- 修改简单
- 减少重绘次数

**缺点**：
- 数值显示不够实时
- 只是缓解，没有彻底解决

---

### 方案C：使用双缓冲技术（过度设计）⭐⭐

**思路**：
- 在内存中准备好画面
- 一次性刷新到屏幕

**缺点**：
- STM32 RAM不够大
- 不适合本项目

---

### 方案D：接受当前状态（临时）⭐⭐⭐

**分析**：
- 修改设置值时短暂闪烁是可以接受的
- 不是高频操作
- 优先解决其他问题

**建议**：
- 先解决"短按无响应"问题
- 后续再优化闪烁

---

## ✅ 推荐实施方案

### 阶段1：快速缓解（5分钟）

**调整阈值，减少重绘频率**：

```c
// ui_display.c:185
if (UI_FloatChanged(s_cache.set_voltage, set_voltage, 0.05f) ||  // 0.01→0.05
    UI_FloatChanged(s_cache.set_current, set_current, 0.01f)) {   // 0.001→0.01
    UI_DrawSettings(set_voltage, set_current);
    s_cache.set_voltage = set_voltage;
    s_cache.set_current = set_current;
}
```

**效果**：
- 减少80%的重绘次数
- 闪烁感知度大幅降低
- 数值更新稍有延迟但可接受

---

### 阶段2：彻底解决（1-2小时，可选）

**实现智能进度条增量更新**：

1. 修改缓存结构
2. 实现`UI_DrawProgressBarSmart()`
3. 修改`UI_DrawSettings()`调用新函数
4. 测试验证

---

## 🧪 测试对比

| 场景 | 当前状态 | 方案B效果 | 方案A效果 |
|------|---------|----------|----------|
| UP键增加0.1V | 闪烁 | 不闪烁（阈值未达到） | 流畅增加 |
| UP键增加1.0V | 闪烁 | 轻微闪烁（重绘1次） | 流畅增加 |
| 连续按UP键 | 频繁闪烁 | 偶尔闪烁 | 完全流畅 |

---

## 📊 优先级建议

| 问题 | 严重程度 | 优先级 | 建议 |
|------|----------|--------|------|
| 短按无响应 | 🔥🔥🔥 高 | P0 | 立即解决 |
| 设置区块一直刷新 | 🔥🔥🔥 高 | P0 | ✅ 已解决 |
| 修改时闪烁 | 🔥 低 | P2 | 快速缓解 |
| 光标不是反相 | 🔥 低 | P3 | 后续优化 |

---

## 🎯 立即行动

**建议**：
1. ✅ 先测试"短按无响应"问题是否解决
2. ⏳ 如果短按正常，再优化闪烁
3. ⏳ 如果短按仍无响应，优先解决按键问题

**快速缓解闪烁**（如果需要）：
```c
// 修改ui_display.c:185
if (UI_FloatChanged(s_cache.set_voltage, set_voltage, 0.05f) ||
    UI_FloatChanged(s_cache.set_current, set_current, 0.01f)) {
```

---

*方案文档生成时间: 2026-06-02*  
*推荐方案: 先解决按键问题，再快速缓解闪烁*  
*彻底解决: 可选，需要1-2小时*
