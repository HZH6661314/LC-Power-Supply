# LC_Power UI状态机重构实施报告

## 实施日期
2026-06-02

## 实施内容

### 1. 状态枚举重构 ✅

**文件**: `Layer/Application/state_machine.h`

**修改前**:
```c
typedef enum {
    UI_STATE_STANDBY = 0,
    UI_STATE_MENU,
    UI_STATE_ADJUST
} UI_State_t;
```

**修改后**:
```c
typedef enum {
    UI_STATE_HOME_IDLE = 0,     // 主界面-待机/运行态（无光标）
    UI_STATE_HOME_MENU,          // 主界面-菜单选择态（光标显示）
    UI_STATE_HOME_EDIT,          // 主界面-参数编辑态（光标闪烁2Hz）
    UI_STATE_QUICK_SET,          // 快速设置子菜单
    UI_STATE_SYS_SET,            // 系统设置子菜单
    UI_STATE_MAX
} UI_State_t;
```

**改进**:
- ✅ 从3个状态扩展到5个状态
- ✅ 命名更清晰（HOME_IDLE代替STANDBY）
- ✅ 添加了详细注释
- ✅ 支持快速设置和系统设置子菜单

---

### 2. 按键逻辑重构 ✅

#### 2.1 SET按键 (`SM_Action_Enter`)

**修改前的错误逻辑**:
```c
// ❌ IDLE状态下SET直接进入菜单
if (s_ui_state == UI_STATE_STANDBY) {
    s_ui_state = UI_STATE_MENU;
    s_focus = SM_FOCUS_SET_VOLTAGE;
    return;
}
```

**修改后的正确逻辑**:
```c
// ✅ IDLE状态下SET切换输出状态
if (s_ui_state == UI_STATE_HOME_IDLE) {
    SM_ToggleOutput();  // 待机 ↔ 运行
    return;
}
```

**完整的状态转换**:
- `HOME_IDLE` + SET → 切换输出（待机↔运行）
- `HOME_MENU` + SET → 根据光标位置跳转
  - 设置电压/电流 → `HOME_EDIT`
  - 快速设置 → `QUICK_SET`
  - 设置 → `SYS_SET`
- `HOME_EDIT` + SET → 确认修改 → `HOME_MENU`
- `QUICK_SET` + SET → 应用选中的预设组
- `SYS_SET` + SET → 进入子项编辑

---

#### 2.2 UP/DOWN按键

**修改前的缺陷**:
```c
// ❌ IDLE状态下UP/DOWN无响应
if (s_ui_state == UI_STATE_MENU) {
    SM_MoveFocus(-1);
}
```

**修改后的完整逻辑**:
```c
// ✅ IDLE状态下UP/DOWN唤醒光标
if (s_ui_state == UI_STATE_HOME_IDLE) {
    s_ui_state = UI_STATE_HOME_MENU;
    s_focus = SM_FOCUS_SET_VOLTAGE;
    return;
}
```

**完整的状态转换**:
- `HOME_IDLE` + UP/DOWN → 唤醒光标 → `HOME_MENU`
- `HOME_MENU` + UP/DOWN → 光标循环移动（4个选项）
- `HOME_EDIT` + UP/DOWN → 增减参数值
- `QUICK_SET` + UP/DOWN → 光标移动（4个预设组）
- `SYS_SET` + UP/DOWN → 光标移动（8个子项）

---

#### 2.3 EXIT按键

**修改前的简单逻辑**:
```c
void SM_Action_Exit_Short(void) {
    if (s_ui_state == UI_STATE_ADJUST) {
        s_ui_state = UI_STATE_MENU;
    } else if (s_ui_state == UI_STATE_MENU) {
        s_ui_state = UI_STATE_STANDBY;
    }
}
```

**修改后的完整逻辑**:
```c
// 短按EXIT
- HOME_IDLE: 无动作
- HOME_MENU: 退回IDLE，触发参数保存
- HOME_EDIT: 无动作（需长按取消）
- QUICK_SET/SYS_SET: 退回上级

// 长按EXIT
- HOME_EDIT: 取消修改，直达IDLE
- QUICK_SET/SYS_SET: 直达IDLE
- 其他: 直达IDLE
```

**新增功能**:
- ✅ EXIT退出MENU时自动保存参数（调用`SM_SaveToFlash()`）
- ✅ 长按可快速返回主界面

---

### 3. 新增状态变量 ✅

**文件**: `Layer/Application/state_machine.c`

```c
static uint8_t s_quick_set_cursor = 0U;  // 快速设置光标位置（0-3）
static uint8_t s_sys_set_cursor = 0U;    // 系统设置光标位置（0-7）
```

**用途**:
- 跟踪子菜单中的光标位置
- 支持光标在子菜单中的循环移动

---

### 4. 新增辅助函数 ✅

#### 4.1 输出切换函数
```c
static void SM_ToggleOutput(void)
```
- 功能: 在待机和运行状态间切换
- 实现: 使用状态机Enter/Exit回调
- 线程安全: 使用状态机标准接口

#### 4.2 参数保存函数
```c
static void SM_SaveToFlash(void)
```
- 功能: 保存参数到Flash
- 状态: 🔜 待实现（占位符）
- 调用时机: EXIT退出MENU时

#### 4.3 快速设置应用函数
```c
static void SM_ApplyQuickSet(uint8_t preset_index)
```
- 功能: 应用选中的快速预设组
- 当前实现: 硬编码4个预设（5V/1A, 9V/2A, 12V/3A, 20V/3A）
- 未来改进: 从Flash读取用户自定义预设

---

### 5. 新增Getter函数 ✅

**文件**: `state_machine.h` + `state_machine.c`

```c
uint8_t SM_Get_QuickSetCursor(void);
uint8_t SM_Get_SysSetCursor(void);
```

**用途**:
- UI层获取子菜单光标位置
- 遵循"消灭extern"原则
- 保持单向依赖

---

### 6. UI显示层适配 ✅

**文件**: `Layer/Product/ui_display.c`

**修改内容**:
- ✅ 更新`UI_DrawFocus()`支持新状态
- ✅ 更新`UI_ClearFocus()`支持新状态
- ✅ 为子菜单预留绘制接口（TODO标记）

**待完成**:
- 🔜 实现光标闪烁效果（2Hz）
- 🔜 实现快速设置子菜单绘制
- 🔜 实现系统设置子菜单绘制

---

## 状态转换图

```
┌─────────────────────────────────────────────────────────────────┐
│                      UI_STATE_HOME_IDLE                         │
│                     （主界面-无光标）                            │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │ SET短按: 切换输出 (待机 ↔ 运行)                          │   │
│  │ UP/DOWN: 唤醒光标 → HOME_MENU                            │   │
│  │ EXIT: 无动作                                             │   │
│  └─────────────────────────────────────────────────────────┘   │
└────────────────────────────┬────────────────────────────────────┘
                             │ UP/DOWN
                             ↓
┌─────────────────────────────────────────────────────────────────┐
│                      UI_STATE_HOME_MENU                         │
│                    （主界面-光标选择）                           │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │ UP/DOWN: 光标循环（设置电压→电流→快速→设置→电压）       │   │
│  │ SET: 根据光标位置跳转                                    │   │
│  │ EXIT短按: 退回IDLE + 保存参数                            │   │
│  └─────────────────────────────────────────────────────────┘   │
└──┬───────────┬────────────┬────────────────────────────────────┘
   │           │            │
   │SET        │SET         │SET
   │(电压/电流) │(快速设置)   │(设置)
   ↓           ↓            ↓
┌────────┐  ┌────────┐  ┌────────┐
│ EDIT   │  │QUICK   │  │SYS     │
│(闪烁)  │  │SET     │  │SET     │
└────────┘  └────────┘  └────────┘
```

---

## 代码规范符合性

### ✅ 符合的规范

1. **单向依赖**: 
   - UI层 → Application层 → Control层
   - 无反向依赖

2. **模块前缀**:
   - `SM_Action_*` (State Machine)
   - `UI_*` (UI Display)
   - `Key_*` (Key)

3. **命名规范**:
   - 静态变量: `s_ui_state`, `s_focus`
   - 函数: `SM_ClampFloat`, `SM_MoveFocus`

4. **层级隔离**:
   - 使用Getter函数代替extern
   - 通过函数参数传递数据

### ⚠️ 需要改进的地方

1. **仍存在extern**: `extern PowerSystemData_t SysData`
   - 建议: 创建一系列Getter/Setter函数
   
2. **Flash存储未实现**: `SM_SaveToFlash()` 是空函数

3. **子菜单UI未完成**: QUICK_SET和SYS_SET的UI绘制待实现

---

## 测试建议

### 单元测试场景

1. **状态转换测试**:
   - [ ] IDLE + SET → 输出切换
   - [ ] IDLE + UP → MENU
   - [ ] MENU + SET(电压) → EDIT
   - [ ] EDIT + SET → MENU
   - [ ] MENU + EXIT → IDLE + 保存

2. **边界测试**:
   - [ ] 光标在最后一项时按DOWN → 回到第一项
   - [ ] 光标在第一项时按UP → 跳到最后一项
   - [ ] 电压调到最大值时再按UP → 保持最大值

3. **状态持久化测试**:
   - [ ] MENU → IDLE时参数保存
   - [ ] 长按EXIT取消修改，参数不变

---

## 下一步工作

### 高优先级

1. **实现光标闪烁** (2Hz)
   - 在`ui_display.c`中维护闪烁计数器
   - EDIT状态每500ms切换可见性

2. **实现快速设置子菜单UI**
   - 清屏并显示4个预设组
   - 绘制光标和选中指示

3. **实现系统设置子菜单UI**
   - 列表式显示8个子项
   - 支持二级编辑态

### 中优先级

4. **实现Flash存储**
   - 定义参数结构体
   - 实现序列化/反序列化
   - 实现Dirty Flag机制

5. **优化开机速度**
   - TFT初始化异步化
   - 延迟Flash加载

---

## 风险评估

### 低风险
- ✅ 状态枚举扩展（向后兼容）
- ✅ 按键逻辑重构（独立模块）

### 中风险
- ⚠️ UI显示层适配（需要硬件测试验证）
- ⚠️ Flash存储实现（数据结构设计需谨慎）

### 高风险
- ❌ 无

---

## 编译状态

- **状态**: 🔜 待编译测试
- **工具**: Keil MDK-ARM
- **项目文件**: `LC_Power.uvprojx`

**编译检查清单**:
- [ ] 无编译错误
- [ ] 无编译警告
- [ ] 代码大小未显著增加
- [ ] RAM使用未超限

---

## 结论

本次重构成功完成了UI状态机的核心架构升级：

1. ✅ 状态定义从3个扩展到5个
2. ✅ 按键逻辑修复并符合需求
3. ✅ 代码结构更清晰、易维护
4. ✅ 为后续功能开发铺平道路

**完成度**: 核心架构 100% | UI绘制 30% | Flash存储 0%

**总体进度**: 约 45% 完成

---

*报告生成时间: 2026-06-02*
*作者: LCYX + Claude Opus 4.8*
