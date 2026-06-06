# 屏幕点亮与 UI 任务完成指标

> 目标：解决当前 TFT 屏幕无任何画面的问题，并确认 MVC UI 能稳定显示。  
> 规则：以下所有检查项完成后，才算“屏幕/UI 任务完成”。其中“用户验收”部分由你在真实硬件上打勾。

## 1. 编译与链接

- [x] Keil 工程 Rebuild 无 `Error`。
- [x] Keil 工程无 `TFTGFX_* Undefined symbol` 链接错误。
- [x] `ui_display.c`、`tft_gfx.c`、`tft_driver.c` 均被 Keil 工程实际编译。
- [x] `task_manager.c`、`ui_display.c`、`state_machine.c`、`key.c` 修改后无新增编译警告。

## 2. TFT 初始化链路

- [x] `SysCore_Init()` 中完成 BSP、状态机、按键、TFT 非阻塞初始化启动。
- [x] `SysCore_Run()` 持续调用 `TFT_InitProcess()`，直到 TFT 初始化完成。
- [x] `TFT_InitProcess()` 返回完成后，调用 `TFT_SetBacklight(100U)`。
- [x] `TFT_InitProcess()` 返回完成后，只调用一次 `UI_Display_Init()`。
- [x] `s_UI_Initialized != 0U` 后，50ms 调度周期调用 `UI_Display_Process()`。
- [x] 若 TFT 初始化未完成，系统不会提前调用 UI 绘图函数。

## 3. 首屏可见性

- [x] `UI_Display_Init()` 中有明确的首屏背景填充动作。
- [x] `UI_Display_Init()` 中绘制 240x240 外框与分区线。
- [x] 首屏至少显示标题栏、实时电压/电流区域、设定值区域、功率区域、功能区、底部状态栏。
- [x] 即使 ADC 当前值为 0，屏幕也能看到线框、文本或占位框。
- [x] `UI_DrawChinese()` 占位函数在无汉字库时仍能画出可见矩形框。

## 4. 局部刷新策略

- [ ] `UI_Display_Process()` 不进行全屏刷屏。
- [ ] 实时 VOUT/IOUT 变化时，只刷新左侧实时数据区域。
- [ ] 设定电压/电流变化时，只刷新右侧设定区域和进度条。
- [ ] 实时功率变化时，只刷新左下功率区域。
- [ ] CV/CC 或 ON/OFF 状态变化时，只刷新底部状态栏。
- [ ] MENU/ADJUST 焦点变化时，只重绘相关焦点区域边框，不清空整个屏幕。

## 5. MVC 边界

- [x] `key.c` 不包含 `ui_display.h`。
- [x] `key.c` 不调用任何 `UI_*` 函数。
- [x] `key.c` 不调用任何 `TFTGFX_*` 或 `TFT_*` 绘图函数。
- [x] `key.c` 的 `KeyMap_t.action` 只指向 `SM_Action_*`。
- [x] `ui_display.c` 不调用任何 `SM_Action_*`。
- [x] `ui_display.c` 只通过 `SM_Get_*` 查询状态机数据。
- [x] `power_control.c` 不再包含 `key.h`。
- [x] `power_control.c` 只通过 `SM_Get_TargetVoltageFinal()`、`SM_Get_CurrentLimit()`、`SM_Get_PowerLimit()` 读取设定值。

## 6. 用户硬件验收

请烧录后在真实硬件上逐项打勾。

- [x] 上电后背光点亮。
- [x] 上电后 2 秒内屏幕不再是纯黑或纯白空屏。
- [x] 屏幕上能看到 UI 外框。
- [ ] 屏幕上能看到标题栏占位内容。
- [ ] 屏幕上能看到 VOUT/IOUT 数字区域。
- [ ] 屏幕上能看到设定电压/设定电流数字区域。
- [ ] 屏幕上能看到两个进度条边框。
- [ ] 屏幕上能看到左下功率区域。
- [ ] 屏幕上能看到右下“快速设置/设置”占位区域。
- [ ] 屏幕底部能看到 `ON/OFF`、`CV/CC`、`36C`、`normal` 四段状态栏。
- [ ] 按 SET 后，焦点框出现在设定电压区域。
- [ ] 按 UP/DOWN 后，焦点能在四个菜单项之间移动。
- [ ] 在设定电压或设定电流上按 SET 后，进入 ADJUST 状态，焦点边框加粗。
- [ ] ADJUST 状态下按 UP/DOWN，设定值发生变化。
- [ ] EXIT 短按能从 ADJUST 回 MENU，再从 MENU 回 STANDBY。
- [ ] EXIT 长按能从任意 UI 状态直接回 STANDBY。

## 7. 防爆盾验收

- [ ] 设定电压范围不会超过 `0.0V - 48.0V`。
- [ ] 设定电流范围不会超过 `0.0A - 5.0A`。
- [ ] 调整电压或电流后，`设定电压 * 设定电流 <= 100W`。
- [ ] 当尝试调到超过 100W 的组合时，当前正在调整的参数会被自动钳制。
- [ ] UI 显示的设定值与闭环控制读取的设定值一致。

## 8. 任务完成判定

- [ ] 编译与链接检查全部完成。
- [ ] TFT 初始化链路检查全部完成。
- [ ] 首屏可见性检查全部完成。
- [ ] 局部刷新策略检查全部完成。
- [ ] MVC 边界检查全部完成。
- [ ] 用户硬件验收全部完成。
- [ ] 防爆盾验收全部完成。

所有项目打勾后，本轮“屏幕点亮与 MVC UI 改造”才算完成。
