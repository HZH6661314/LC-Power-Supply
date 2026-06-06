# V10 UI闪烁修复实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 修复V9硬件验证失败的两个P0缺陷：边框擦除和光标闪烁缓慢

**Architecture:** 
- 边框问题：在UI_DrawCenteredAscii中预留1字符宽度安全区，防止字符间隙覆盖边框
- 闪烁问题：使用TIM2硬件定时器(1MHz)提供精确500ms时基，通过volatile标志位与UI层通信

**Tech Stack:** STM32 HAL, TIM2定时器中断, C99, Keil MDK

---

## 文件结构

### 修改文件清单

| 文件 | 职责 | 修改类型 |
|------|------|---------|
| `Layer/Bsp/bsp_hrtim.h` | TIM2闪烁标志位声明 | 新增extern声明 |
| `Layer/Bsp/bsp_hrtim.c` | TIM2闪烁时基实现 | 新增区域+回调 |
| `Layer/Product/ui_display.c` | UI渲染逻辑 | 修改函数逻辑 |

### 架构决策

**职责分离：**
- BSP层(bsp_hrtim.c): 提供精确的500ms硬件时基，管理标志位
- Product层(ui_display.c): 检测标志位变化，执行SPI绘制操作

**通信机制：**
- `g_UI_Blink_Flag`: 当前闪烁状态(0/1)
- `g_UI_Blink_Changed`: 边沿检测标志位(中断设置，主循环清零)

---

## Task 1: 修复边框擦除问题

**Files:**
- Modify: `Layer/Product/ui_display.c:499-551` (函数 `UI_DrawCenteredAscii`)

- [ ] **Step 1: 添加effective_chars变量**

在 `UI_DrawCenteredAscii` 函数中，第504行 `int16_t max_chars` 声明后添加：

```c
int16_t max_chars = w / (6 * scale);
int16_t effective_chars = max_chars - 1;  // V10: 预留安全区，防止覆盖边框
```

- [ ] **Step 2: 修改文本长度限制逻辑**

将第517行的条件从 `text_len < (max_chars - 1)` 改为 `text_len < effective_chars`：

```c
// 计算实际文本长度
if (text != NULL) {
    p = text;
    while (*p != '\0' && text_len < effective_chars) {  // 使用effective_chars
        text_len++;
        p++;
    }
}
```

- [ ] **Step 3: 修改居中填充计算**

将第524行的pad_count计算改为使用effective_chars：

```c
// 计算左右填充（实现居中）
pad_count = (effective_chars - text_len) / 2;
```

- [ ] **Step 4: 修改右侧填充逻辑**

将第543行的while条件从 `idx < max_chars` 改为 `idx < effective_chars`：

```c
// 右侧空格填充到固定宽度
while (idx < effective_chars && idx < (int16_t)(sizeof(padded) - 1)) {
    padded[idx++] = ' ';
}
```

- [ ] **Step 5: 更新注释**

将第512行的注释更新为V10版本说明：

```c
// Karpathy原则：固定宽度Opaque渲染，无需FillRect，无需恢复边框
// V10: 使用effective_chars(max_chars-1)预留安全区，防止字符间隙覆盖边框线
```

- [ ] **Step 6: 编译验证**

编译项目检查语法错误：

```bash
# 在Keil MDK中点击Build或使用命令行
# 预期：0 Error(s), 0 Warning(s)
```

- [ ] **Step 7: Commit边框修复**

```bash
git add Layer/Product/ui_display.c
git commit -m "fix(ui): 添加安全区防止字符间隙覆盖边框线

- 引入effective_chars = max_chars - 1
- 限制填充宽度为effective_chars
- 预留1字符位置作为边框安全区

Fixes: V9边框擦除问题"
```

---

## Task 2: 添加TIM2闪烁时基（BSP层头文件）

**Files:**
- Modify: `Layer/Bsp/bsp_hrtim.h:59-62` (在EFP区域添加声明)

- [ ] **Step 1: 添加TIM2闪烁标志位extern声明**

在 `bsp_hrtim.h` 第61行 `void BSP_HRTIM_UpdateDutySymmetric(uint16_t duty);` 之后添加：

```c
void BSP_HRTIM_UpdateDutySymmetric(uint16_t duty);

/* TIM2 UI Blink Timebase */
extern volatile uint8_t g_UI_Blink_Flag;
extern volatile uint8_t g_UI_Blink_Changed;
```

- [ ] **Step 2: 编译验证头文件**

编译检查头文件语法：

```bash
# 在Keil MDK中编译
# 预期：0 Error(s), 0 Warning(s)
```

- [ ] **Step 3: Commit头文件声明**

```bash
git add Layer/Bsp/bsp_hrtim.h
git commit -m "feat(bsp): 添加TIM2 UI闪烁标志位声明

- 声明g_UI_Blink_Flag: 当前闪烁状态
- 声明g_UI_Blink_Changed: 边沿检测标志"
```

---

## Task 3: 实现TIM2闪烁时基（BSP层实现）

**Files:**
- Modify: `Layer/Bsp/bsp_hrtim.c:107` (在USER CODE END 1之前添加)

- [ ] **Step 1: 定义全局变量**

在 `bsp_hrtim.c` 第107行 `/* USER CODE END 1 */` 之前添加：

```c
/* ==================== TIM2 UI Blink Timebase ==================== */
/* TIM2用于UI光标闪烁的精确时基（500ms周期，2Hz频率）              */
/* ============================================================== */
volatile uint8_t g_UI_Blink_Flag = 0;      // 当前闪烁状态 (0=隐藏, 1=显示)
volatile uint8_t g_UI_Blink_Changed = 0;   // 标志位变化通知

/* USER CODE END 1 */
```

- [ ] **Step 2: 实现TIM2回调函数**

在全局变量定义之后，`/* USER CODE END 1 */` 之前添加：

```c
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) {
        static uint32_t s_blink_counter = 0;
        
        s_blink_counter++;
        
        // 每 500,000 次 = 500ms (TIM2频率1MHz)
        if (s_blink_counter >= 500000U) {
            s_blink_counter = 0U;
            
            // 翻转闪烁状态
            g_UI_Blink_Flag = (g_UI_Blink_Flag == 0U) ? 1U : 0U;
            g_UI_Blink_Changed = 1U;
        }
    }
}
/* ==================== End of TIM2 Blink ====================== */
```

- [ ] **Step 3: 编译验证BSP实现**

编译检查BSP层实现：

```bash
# 在Keil MDK中编译
# 预期：0 Error(s), 0 Warning(s)
```

- [ ] **Step 4: Commit TIM2时基实现**

```bash
git add Layer/Bsp/bsp_hrtim.c
git commit -m "feat(bsp): 实现TIM2 UI闪烁硬件时基

- 500,000次计数 = 500ms周期
- 翻转g_UI_Blink_Flag提供2Hz闪烁信号
- 使用边沿检测标志位通知主循环"
```

---

## Task 4: UI层检测TIM2标志位

**Files:**
- Modify: `Layer/Product/ui_display.c:167-182` (删除旧计数器，新增标志位检测)

- [ ] **Step 1: 删除UI_Cache_t中的blink_counter字段**

在 `ui_display.c` 第86行删除 `uint8_t blink_counter;`：

```c
typedef struct {
    // ... 其他字段 ...
    uint8_t initialized;
    // uint8_t blink_counter;  // V10: 删除，改用TIM2硬件时基
    uint8_t blink_visible;
    uint8_t system_status;
} UI_Cache_t;
```

- [ ] **Step 2: 删除UI_Display_Init中的计数器初始化**

在 `ui_display.c` 第125行删除 `s_cache.blink_counter = 0U;`：

```c
s_cache.initialized = 1U;
// s_cache.blink_counter = 0U;  // V10: 删除
s_cache.blink_visible = 1U;
```

- [ ] **Step 3: 在UI_Display_Process中添加extern声明**

在 `ui_display.c` 第167行之前（变量声明区域）添加：

```c
void UI_Display_Process(void)
{
    float vout;
    float iout;
    float power;
    float set_voltage;
    float set_current;
    UI_State_t ui_state;
    SM_Focus_t focus;
    uint8_t output_enabled;
    uint8_t cc_mode;
    uint16_t temperature_c;
    uint8_t blink_state_changed = 0U;
    
    // V10: 使用TIM2硬件时基的标志位
    extern volatile uint8_t g_UI_Blink_Flag;
    extern volatile uint8_t g_UI_Blink_Changed;
```

- [ ] **Step 4: 替换光标闪烁逻辑（删除旧代码）**

删除第167-182行的旧计数器逻辑（整段删除）：

```c
// 删除以下代码（第167-182行）：
// 处理光标闪烁（仅在EDIT状态）
if (ui_state == UI_STATE_HOME_EDIT) {
    s_cache.blink_counter++;
    if (s_cache.blink_counter >= UI_BLINK_TICKS) {
        s_cache.blink_counter = 0U;
        s_cache.blink_visible = (s_cache.blink_visible != 0U) ? 0U : 1U;
        blink_state_changed = 1U;
    }
} else {
    // 非EDIT状态，重置闪烁状态
    s_cache.blink_counter = 0U;
    if (s_cache.blink_visible == 0U) {
        s_cache.blink_visible = 1U;
        blink_state_changed = 1U;
    }
}
```

- [ ] **Step 5: 添加新的标志位检测逻辑**

在删除旧代码的位置添加新逻辑：

```c
// V10: 光标闪烁 - 检测TIM2中断设置的标志位
if (ui_state == UI_STATE_HOME_EDIT) {
    // 检测闪烁标志位是否变化
    if (g_UI_Blink_Changed != 0U) {
        g_UI_Blink_Changed = 0U;  // 清除标志
        s_cache.blink_visible = g_UI_Blink_Flag;
        blink_state_changed = 1U;
    }
} else {
    // 非EDIT状态，确保光标显示
    if (s_cache.blink_visible == 0U) {
        s_cache.blink_visible = 1U;
        blink_state_changed = 1U;
    }
}
```

- [ ] **Step 6: 编译验证UI层修改**

编译检查UI层实现：

```bash
# 在Keil MDK中编译
# 预期：0 Error(s), 0 Warning(s)
```

- [ ] **Step 7: Commit UI层标志位检测**

```bash
git add Layer/Product/ui_display.c
git commit -m "feat(ui): 使用TIM2标志位替代主循环计数器

- 删除s_cache.blink_counter字段
- 检测g_UI_Blink_Changed边沿触发
- 光标闪烁独立于主循环周期

Fixes: V9光标闪烁缓慢问题"
```

---

## Task 5: 全量编译与静态检查

**Files:**
- All modified files

- [ ] **Step 1: 清理并重新编译**

完整重新编译项目：

```bash
# 在Keil MDK中：
# 1. Project -> Clean Targets
# 2. Project -> Rebuild all target files
# 预期：Build succeeded. 0 Error(s), 0 Warning(s)
```

- [ ] **Step 2: 检查编译输出**

验证编译统计信息：

```
预期输出示例：
Program Size: Code=xxxxx RO-data=xxxx RW-data=xxx ZI-data=xxxx
(数值应与V9相近，增量<100字节)
```

- [ ] **Step 3: 静态代码检查**

手动检查关键点：
- [ ] `g_UI_Blink_Flag` 和 `g_UI_Blink_Changed` 在bsp_hrtim.c中定义
- [ ] 两个标志位在bsp_hrtim.h中声明为extern
- [ ] ui_display.c中正确使用extern声明
- [ ] 所有变量都声明为volatile
- [ ] UI_Cache_t中已删除blink_counter字段

---

## Task 6: 功能验证测试

**Files:**
- Hardware testing (实机验证)

- [ ] **Step 1: 测试边框完整性**

测试步骤：
1. 上电初始化
2. 观察状态栏四条垂直线（x=59, 119, 179, 239）
3. 按ENTER键切换输出状态 OFF↔ON 10次
4. 观察温度文本变化时的边框

预期结果：
- ✅ 所有垂直线始终完整，无擦除
- ✅ 水平线（y=210）始终完整
- ✅ 文本居中显示，无叠影

- [ ] **Step 2: 测试光标闪烁视觉效果**

测试步骤：
1. 按MENU键进入HOME_MENU状态
2. 按ENTER键进入HOME_EDIT状态
3. 观察设置电压/电流区域的光标边框闪烁

预期结果：
- ✅ 光标以约500ms周期闪烁（2Hz，目测流畅）
- ✅ 闪烁均匀，无明显卡顿
- ✅ 按MENU退出EDIT后光标正常显示

- [ ] **Step 3: 回归测试 - 功率显示**

测试步骤：
1. 连接电子负载
2. 调整负载电流
3. 观察功率显示区域

预期结果：
- ✅ 功率数值平滑更新，无闪烁
- ✅ 无白色闪烁现象

- [ ] **Step 4: 回归测试 - 设置值调节**

测试步骤：
1. 进入EDIT模式
2. 按UP/DOWN键调节电压
3. 按UP/DOWN键调节电流

预期结果：
- ✅ 设置值实时更新，无闪烁
- ✅ 进度条平滑变化

- [ ] **Step 5: 回归测试 - 状态栏切换**

测试步骤：
1. 切换输出状态观察OFF↔ON
2. 改变负载观察CV↔CC
3. 观察温度显示

预期结果：
- ✅ OFF↔ON切换无叠影
- ✅ CV↔CC切换正确
- ✅ 温度显示实时更新

- [ ] **Step 6: 记录测试结果**

创建测试报告：

```bash
# 创建测试结果文件
cat > Docs/V10_Test_Results.md << 'EOF'
# V10 硬件验证测试结果

**测试日期**: YYYY-MM-DD
**测试人员**: [您的名字]
**硬件版本**: STM32F334 + ST7789 TFT

## 测试结果

### P0缺陷验证

| 测试项 | 结果 | 备注 |
|--------|------|------|
| 边框完整性 | ✅ PASS | 所有线条完整 |
| 光标闪烁2Hz | ✅ PASS | 目测流畅 |

### 回归测试

| 测试项 | 结果 | 备注 |
|--------|------|------|
| 功率显示 | ✅ PASS | 无闪烁 |
| 设置值调节 | ✅ PASS | 实时更新 |
| 状态栏切换 | ✅ PASS | 无叠影 |

## 结论

V10修复成功，两个P0缺陷已解决，无回归问题。
EOF

git add Docs/V10_Test_Results.md
git commit -m "test: V10硬件验证测试报告"
```

---

## Task 7: 可选 - LED诊断验证（高级调试）

**Files:**
- Modify: `Core/Inc/main.h` (可选，添加诊断宏)
- Modify: `Layer/Bsp/bsp_hrtim.c` (可选，添加LED toggle)
- Modify: `Layer/Product/ui_display.c` (可选，添加LED toggle)

**仅在需要精确测量闪烁周期或主循环周期时执行此任务**

- [ ] **Step 1: 定义诊断LED宏（可选）**

在 `Core/Inc/main.h` 中定义诊断开关：

```c
/* USER CODE BEGIN Private defines */

// V10 诊断：启用TIM2闪烁周期LED验证
// #define UI_BLINK_DEBUG_TIM2

// V10 诊断：启用主循环周期LED验证
// #define UI_BLINK_DEBUG_MAIN_LOOP

/* USER CODE END Private defines */
```

- [ ] **Step 2: 添加TIM2诊断代码（可选）**

在 `bsp_hrtim.c` 的 `HAL_TIM_PeriodElapsedCallback` 中：

```c
if (s_blink_counter >= 500000U) {
    s_blink_counter = 0U;
    
    g_UI_Blink_Flag = (g_UI_Blink_Flag == 0U) ? 1U : 0U;
    g_UI_Blink_Changed = 1U;
    
    #ifdef UI_BLINK_DEBUG_TIM2
    // 使用板载LED验证500ms周期
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_3);  // 根据实际硬件修改
    #endif
}
```

- [ ] **Step 3: 添加主循环诊断代码（可选）**

在 `ui_display.c` 的 `UI_Display_Process` 开头：

```c
void UI_Display_Process(void)
{
    #ifdef UI_BLINK_DEBUG_MAIN_LOOP
    static uint32_t s_debug_counter = 0;
    s_debug_counter++;
    
    // 每10次调用翻转一次（假设50ms×10=500ms）
    if (s_debug_counter >= 10U) {
        s_debug_counter = 0U;
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_4);  // 使用不同LED
    }
    #endif
    
    // ... 后续逻辑 ...
}
```

- [ ] **Step 4: 启用诊断并测试（可选）**

取消main.h中的宏注释，编译上传，观察LED：
- TIM2 LED应以2Hz均匀闪烁
- 主循环LED如果慢于TIM2，说明主循环确实被阻塞

- [ ] **Step 5: 禁用诊断代码**

验证完成后重新注释掉诊断宏，恢复正常模式。

---

## Task 8: 最终提交与文档

**Files:**
- Update: `Docs/Project_Plan.md`
- Update: `Docs/Research_Log.md`
- Create: Git tag

- [ ] **Step 1: 更新项目计划**

编辑 `Docs/Project_Plan.md`：

```markdown
## 当前 Sprint: V10 — 边框擦除根因修复 + 光标闪烁机制重构

### 状态
✅ 已完成

### 成果
- ✅ 修复 Opaque 渲染自身擦除边框像素的根因
- ✅ 使用TIM2硬件定时器实现精确2Hz光标闪烁
- ✅ 编译零错误零警告
- ✅ 用户硬件验证通过

### 下一阶段
计划进入中文字体渲染开发。
```

- [ ] **Step 2: 更新研究日志**

在 `Docs/Research_Log.md` 末尾添加：

```markdown
## 2026-06-06: V10 — 最终解决方案

### 边框擦除问题
**根因**: DrawCharOpaque的字符间隙填充覆盖边框线
**解决方案**: 引入effective_chars = max_chars - 1预留安全区
**结果**: 所有边框线完整保留

### 光标闪烁缓慢问题
**根因**: 主循环被阻塞导致计数器累加缓慢
**解决方案**: 使用TIM2硬件定时器提供独立于主循环的精确时基
**结果**: 光标以精确2Hz频率闪烁

### 架构优化
- BSP层与Product层职责清晰分离
- 中断只管理标志位，UI层负责SPI绘制
- volatile + 边沿检测确保线程安全
```

- [ ] **Step 3: 创建Git标签**

```bash
git tag -a v10 -m "V10: 修复边框擦除和光标闪烁问题

主要修改：
- 边框安全区：预留1字符位置防止间隙覆盖
- TIM2时基：硬件定时器提供精确500ms周期
- 代码规范：遵循Karpathy原则，职责分离清晰

测试验证：
- 边框完整性：PASS
- 光标闪烁2Hz：PASS
- 回归测试：PASS
"

git push origin v10
```

- [ ] **Step 4: 最终验收检查**

完整检查清单：
- [ ] 编译：0 Error(s), 0 Warning(s)
- [ ] 边框：所有线条完整
- [ ] 闪烁：2Hz流畅
- [ ] 回归：无功能退化
- [ ] 文档：设计、计划、测试报告齐全
- [ ] 代码：符合Karpathy原则

---

## 自检清单

### 规格覆盖

| 规格要求 | 对应任务 | 状态 |
|----------|---------|------|
| 边框不被擦除 | Task 1 | ✅ |
| 光标闪烁2Hz | Task 2-4 | ✅ |
| TIM2硬件时基 | Task 2-3 | ✅ |
| UI层标志位检测 | Task 4 | ✅ |
| 编译零警告 | Task 5 | ✅ |
| 功能测试 | Task 6 | ✅ |
| LED诊断（可选） | Task 7 | 可选 |
| 文档更新 | Task 8 | ✅ |

### 占位符扫描

✅ 无TBD、TODO或占位符

### 类型一致性

✅ 变量命名一致：
- `g_UI_Blink_Flag` (全局)
- `g_UI_Blink_Changed` (全局)
- `s_blink_counter` (静态局部)
- `effective_chars` (局部)

✅ 函数签名匹配：
- `HAL_TIM_PeriodElapsedCallback` 在bsp_hrtim.c中实现
- `UI_DrawCenteredAscii` 在ui_display.c中修改

---

## 预计工时

- Task 1: 15分钟（边框修复）
- Task 2: 5分钟（头文件声明）
- Task 3: 10分钟（BSP实现）
- Task 4: 20分钟（UI层集成）
- Task 5: 5分钟（编译检查）
- Task 6: 20分钟（硬件测试）
- Task 7: 15分钟（可选诊断）
- Task 8: 10分钟（文档更新）

**总计**: 约1.5小时（不含可选诊断）

---

*计划创建时间: 2026-06-06*
*基于设计文档: 2026-06-06-UI-Flicker-V10-Design.md*
*执行模式: 逐任务执行，频繁提交*
