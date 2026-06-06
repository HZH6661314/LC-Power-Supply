# CubeMX代码保护指南

## 🎯 问题说明

CubeMX重新生成代码时，会覆盖所有不在`USER CODE`区域内的代码。

---

## ✅ 解决方案：使用USER CODE区域

### CubeMX的USER CODE区域

CubeMX会保护以下区域的代码：

```c
/* USER CODE BEGIN xxx */
// 这里的代码不会被覆盖
/* USER CODE END xxx */
```

### 常见的USER CODE区域

#### 1. 文件头部
```c
/* USER CODE BEGIN Header */
// 版权信息、文件说明
/* USER CODE END Header */
```

#### 2. 包含文件
```c
/* USER CODE BEGIN Includes */
#include "my_custom_header.h"
/* USER CODE END Includes */
```

#### 3. 私有定义
```c
/* USER CODE BEGIN PD */
#define MY_CONSTANT 100
/* USER CODE END PD */
```

#### 4. 私有变量
```c
/* USER CODE BEGIN PV */
static uint32_t my_variable;
/* USER CODE END PV */
```

#### 5. 函数内部（最重要）
```c
void MX_XXX_Init(void)
{
    /* USER CODE BEGIN XXX_Init 0 */
    // 初始化前的代码
    /* USER CODE END XXX_Init 0 */
    
    // CubeMX生成的代码
    
    /* USER CODE BEGIN XXX_Init 1 */
    // 初始化中的代码
    /* USER CODE END XXX_Init 1 */
    
    // CubeMX生成的代码
    
    /* USER CODE BEGIN XXX_Init 2 */
    // 初始化后的代码
    /* USER CODE END XXX_Init 2 */
}
```

---

## 🔧 HRTIM DLL校准延时的正确位置

### 问题代码（会被覆盖）❌

```c
void MX_HRTIM1_Init(void)
{
    /* USER CODE BEGIN HRTIM1_Init 1 */
    /* USER CODE END HRTIM1_Init 1 */
    
    hhrtim1.Instance = HRTIM1;
    HAL_HRTIM_Init(&hhrtim1);
    HAL_HRTIM_DLLCalibrationStart_IT(&hhrtim1, HRTIM_CALIBRATIONRATE_14);
    
    HAL_Delay(10);  // ❌ 这里会被CubeMX覆盖！
    
    pADCTriggerCfg.Trigger = HRTIM_ADCTRIGGEREVENT24_NONE;
    HAL_HRTIM_ADCTriggerConfig(&hhrtim1, HRTIM_ADCTRIGGER_2, &pADCTriggerCfg);
}
```

### 正确代码（不会被覆盖）✅

```c
void MX_HRTIM1_Init(void)
{
    /* USER CODE BEGIN HRTIM1_Init 1 */
    // 定义延时时间
    #define HRTIM_DLL_CALIBRATION_DELAY_MS  10
    /* USER CODE END HRTIM1_Init 1 */
    
    hhrtim1.Instance = HRTIM1;
    HAL_HRTIM_Init(&hhrtim1);
    HAL_HRTIM_DLLCalibrationStart_IT(&hhrtim1, HRTIM_CALIBRATIONRATE_14);
    
    /* USER CODE BEGIN HRTIM1_Init 1.5 */
    // ✅ 这里的代码不会被覆盖
    // 等待DLL校准完成（关键！）
    HAL_Delay(HRTIM_DLL_CALIBRATION_DELAY_MS);
    /* USER CODE END HRTIM1_Init 1.5 */
    
    pADCTriggerCfg.Trigger = HRTIM_ADCTRIGGEREVENT24_NONE;
    HAL_HRTIM_ADCTriggerConfig(&hhrtim1, HRTIM_ADCTRIGGER_2, &pADCTriggerCfg);
}
```

---

## 📋 CubeMX重新生成代码后的检查清单

### 每次使用CubeMX重新生成代码后，检查：

- [ ] `HRTIM1_Init 1.5`区域的延时代码是否还在
- [ ] `bsp_adc.c`中的`HAL_ADCEx_InjectedConvCpltCallback()`是否还在
- [ ] 其他USER CODE区域的自定义代码是否还在

---

## 💡 最佳实践

### 1. 使用有意义的注释

```c
/* USER CODE BEGIN HRTIM1_Init 1.5 */
// ⚠️ 重要：DLL校准后必须延时！
// 原因：HAL_HRTIM_DLLCalibrationStart_IT()是异步的
// 如果不延时，ADC触发配置会失败
// 不要删除这段代码！
HAL_Delay(HRTIM_DLL_CALIBRATION_DELAY_MS);
/* USER CODE END HRTIM1_Init 1.5 */
```

### 2. 创建自定义的USER CODE区域

如果CubeMX没有提供合适的USER CODE区域，可以手动添加：

```c
/* USER CODE BEGIN HRTIM1_Init 1.5 */
// 自定义区域
/* USER CODE END HRTIM1_Init 1.5 */
```

**注意：** 区域名称必须唯一，不能与现有的重复。

### 3. 将复杂逻辑移到BSP层

对于复杂的初始化逻辑，建议放在BSP层：

```c
// bsp_hrtim.c
void BSP_HRTIM_Init(void)
{
    // 这里的代码完全不受CubeMX影响
    MX_HRTIM1_Init();
    
    // 额外的初始化
    HAL_HRTIM_WaveformCounterStart(&hhrtim1, HRTIM_TIMERID_TIMER_A);
}
```

然后在main.c中调用：

```c
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_HRTIM1_Init();  // CubeMX生成
    
    /* USER CODE BEGIN 2 */
    BSP_HRTIM_Init();  // 自定义初始化
    /* USER CODE END 2 */
}
```

---

## 🔍 如何查找USER CODE区域

### 方法1：查看CubeMX生成的文件

打开任何CubeMX生成的.c文件，搜索`USER CODE`，你会看到所有可用的区域。

### 方法2：常见的USER CODE区域位置

| 位置 | 区域名称 | 用途 |
|------|---------|------|
| 文件开头 | `Header` | 版权、说明 |
| include后 | `Includes` | 自定义头文件 |
| 变量定义前 | `PV` | 私有变量 |
| 函数定义前 | `PFP` | 函数声明 |
| 函数开始 | `XXX_Init 0` | 初始化前 |
| 函数中间 | `XXX_Init 1` | 初始化中 |
| 函数结束 | `XXX_Init 2` | 初始化后 |
| main循环前 | `2` | 主循环前 |
| main循环中 | `3` | 主循环中 |

---

## ⚠️ 注意事项

### 1. 不要修改USER CODE区域外的代码

如果必须修改，记录下来，每次重新生成后手动恢复。

### 2. 使用版本控制

使用Git等版本控制工具，每次CubeMX重新生成后：

```bash
git diff
```

检查哪些代码被覆盖了。

### 3. 备份重要代码

在重新生成前，备份关键的修改：

```bash
cp Core/Src/hrtim.c Core/Src/hrtim.c.backup
```

---

## 📝 HRTIM配置总结

### 当前的正确配置

**文件：** `Core/Src/hrtim.c`

```c
void MX_HRTIM1_Init(void)
{
    /* USER CODE BEGIN HRTIM1_Init 1 */
    // 定义DLL校准延时
    #define HRTIM_DLL_CALIBRATION_DELAY_MS  10
    /* USER CODE END HRTIM1_Init 1 */
    
    hhrtim1.Instance = HRTIM1;
    hhrtim1.Init.HRTIMInterruptResquests = HRTIM_IT_NONE;
    hhrtim1.Init.SyncOptions = HRTIM_SYNCOPTION_NONE;
    
    if (HAL_HRTIM_Init(&hhrtim1) != HAL_OK)
    {
        Error_Handler();
    }
    
    if (HAL_HRTIM_DLLCalibrationStart_IT(&hhrtim1, HRTIM_CALIBRATIONRATE_14) != HAL_OK)
    {
        Error_Handler();
    }
    
    /* USER CODE BEGIN HRTIM1_Init 1.5 */
    // ⚠️ 关键：等待DLL校准完成
    // 不要删除这段代码！
    HAL_Delay(HRTIM_DLL_CALIBRATION_DELAY_MS);
    /* USER CODE END HRTIM1_Init 1.5 */
    
    // ADC触发配置
    pADCTriggerCfg.UpdateSource = HRTIM_ADCTRIGGERUPDATE_TIMER_A;
    pADCTriggerCfg.Trigger = HRTIM_ADCTRIGGEREVENT24_NONE;
    if (HAL_HRTIM_ADCTriggerConfig(&hhrtim1, HRTIM_ADCTRIGGER_2, &pADCTriggerCfg) != HAL_OK)
    {
        Error_Handler();
    }
    
    // ... 其他配置 ...
}
```

### 为什么需要延时？

1. `HAL_HRTIM_DLLCalibrationStart_IT()`启动DLL校准（异步）
2. DLL校准需要几毫秒才能完成
3. 如果立即配置ADC触发器，HRTIM状态不正确
4. `HAL_HRTIM_ADCTriggerConfig()`内部检查失败，返回错误
5. 加入延时后，DLL校准完成，配置成功

---

## 🎉 总结

### 关键点：

1. ✅ 所有自定义代码必须放在`USER CODE`区域内
2. ✅ HRTIM DLL校准后必须延时10ms
3. ✅ 使用有意义的注释说明为什么需要这段代码
4. ✅ 每次CubeMX重新生成后检查USER CODE区域

### 现在的配置：

- ✅ 延时代码在`USER CODE BEGIN HRTIM1_Init 1.5`区域
- ✅ 不会被CubeMX覆盖
- ✅ 有详细的注释说明
- ✅ HRTIM能正常初始化

---

**现在你可以放心使用CubeMX重新生成代码了！** 🎉
