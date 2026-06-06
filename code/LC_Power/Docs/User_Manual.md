# LC 数控电源系统使用手册

## 1. 系统架构职责

本工程 UI 与按键采用 MVC 解耦结构，三层之间只通过公开接口通信。

| 模块 | 文件 | 职责 | 允许调用 | 禁止事项 |
| --- | --- | --- | --- | --- |
| 交互层 | `Layer/Product/key.h/.c` | 扫描按键、消抖、短按/长按识别、事件入队和出队、按键映射分发 | `SM_Action_Enter`、`SM_Action_Exit_Short`、`SM_Action_Exit_Long`、`SM_Action_Up`、`SM_Action_Down` | 禁止调用 UI 绘图函数，禁止维护电压/电流/功率业务参数 |
| 控制层 | `Layer/Application/state_machine.h/.c` | 维护 UI 状态、焦点、设定电压、设定电流、功率限制与防爆盾逻辑 | ADC/控制层可读取 `SM_Get_*` 查询接口，按键层可调用 `SM_Action_*` 动作接口 | 禁止绘制 UI，禁止处理底层按键电平 |
| 表现层 | `Layer/Product/ui_display.h/.c` | 绘制 240x240 UI，按 dirty flag 局部刷新变化区域 | `Get_VOUT()`、`Get_IOUT()`、`SM_Get_*()`、`TFTGFX_*()` | 禁止调用 `SM_Action_*`，禁止修改设定值或状态 |

## 2. UI 界面说明

屏幕分辨率为 240x240，所有区域采用固定坐标。

| 区域 | 坐标 | 显示内容 | 数据来源 | 刷新策略 |
| --- | --- | --- | --- | --- |
| 标题栏 | `x=0,y=0,w=240,h=32` | `LC数控电源` 占位标题 | 固定文本 | 初始化绘制 |
| 左侧实时数据 | `x=0,y=32,w=138,h=110` | 实际输出电压 VOUT、实际输出电流 IOUT | `Get_VOUT()`、`Get_IOUT()` | 数值变化时局部擦除重绘 |
| 右侧设定数据 | `x=138,y=32,w=102,h=110` | 设定电压、设定电流和进度条 | `SM_Get_TargetVoltageFinal()`、`SM_Get_CurrentLimit()` | 设定值变化时局部刷新 |
| 左下功率 | `x=0,y=142,w=138,h=68` | 实时功率 `VOUT * IOUT` | ADC 实时值计算 | 功率变化时局部刷新 |
| 右下功能 | `x=138,y=142,w=102,h=68` | 快速设置、设置 | 固定菜单项 | 初始化和焦点变化时刷新边框 |
| 底部状态栏 | `x=0,y=210,w=240,h=30` | ON/OFF、CV/CC、36C、normal | 状态机、ADC、固定文本 | 状态变化时局部刷新 |

CV/CC 显示规则：`Get_IOUT() >= SM_Get_CurrentLimit()` 时显示 `CC`，否则显示 `CV`。

## 3. 按键操作逻辑

两组按键复用同一组状态机动作。

| 物理按键 | 短按动作 | 长按动作 |
| --- | --- | --- |
| `KEY_SET` / `WT_SET` | `SM_Action_Enter` | `SM_Action_Enter` |
| `KEY_UP` / `WT_UP` | `SM_Action_Up` | `SM_Action_Up` |
| `KEY_DOWN` / `WT_DOWN` | `SM_Action_Down` | `SM_Action_Down` |
| `KEY_EXIT` | `SM_Action_Exit_Short` | `SM_Action_Exit_Long` |

状态流转表：

| 当前状态 | 操作 | 行为 | 下一个状态 |
| --- | --- | --- | --- |
| `UI_STATE_STANDBY` | Enter | 进入菜单，焦点指向设定电压 | `UI_STATE_MENU` |
| `UI_STATE_MENU` | Up/Down | 焦点在设定电压、设定电流、快速设置、设置之间循环 | `UI_STATE_MENU` |
| `UI_STATE_MENU` | Enter，焦点为设定电压/设定电流 | 进入参数调整 | `UI_STATE_ADJUST` |
| `UI_STATE_MENU` | Enter，焦点为快速设置 | 应用 `5.0V / 1.0A` 预设 | `UI_STATE_MENU` |
| `UI_STATE_MENU` | Enter，焦点为设置 | 预留设置入口，占位不跳转 | `UI_STATE_MENU` |
| `UI_STATE_ADJUST` | Up/Down | 调整当前焦点参数 | `UI_STATE_ADJUST` |
| `UI_STATE_ADJUST` | Exit 短按 | 退出调整 | `UI_STATE_MENU` |
| `UI_STATE_MENU` | Exit 短按 | 退出菜单 | `UI_STATE_STANDBY` |
| 任意 UI 状态 | Exit 长按 | 立即回待机，焦点复位 | `UI_STATE_STANDBY` |

## 4. 参数限制与防爆盾

| 参数 | 范围 | 步进 |
| --- | --- | --- |
| 设定电压 | `0.0V - 48.0V` | `0.5V` |
| 设定电流 | `0.0A - 5.0A` | `0.1A` |
| 功率限制 | `100.0W` | 固定 |

防爆盾规则由状态机执行：每次调整电压或电流后都会检查 `V * I <= 100W`。若超限，优先钳制当前正在调整的参数，保证显示设定值与闭环控制读取值一致。

## 5. 调度说明

| 周期 | 调用 | 说明 |
| --- | --- | --- |
| 初始化 | `State_Machine_Init()`、`Key_Init()` | 初始化 UI 状态、焦点、设定值和按键队列 |
| 10ms | `Key_Process()` | 扫描全部按键并派发状态机动作 |
| 50ms | `UI_Display_Process()` | dirty flag 局部刷新 UI |
| 控制中断 | `Power_Control_Process()` | 闭环控制读取 `SM_Get_TargetVoltageFinal()`、`SM_Get_CurrentLimit()`、`SM_Get_PowerLimit()` |

`UI_DrawChinese()` 当前只是矩形占位函数，真实汉字字库接入时只需要替换该函数内部绘制实现。
