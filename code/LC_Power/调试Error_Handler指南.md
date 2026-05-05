# 调试Error_Handler问题 - 完整指南

## 🔍 问题说明

程序一直卡在`Error_Handler()`中，说明某个HAL函数返回了错误。

---

## 🛠️ 调试方法

### 方法1：使用调试器查看调用栈（最快）

#### 步骤1：在Error_Handler设置断点

1. 打开`Core/Src/main.c`
2. 找到`Error_Handler()`函数（第174行）
3. 在第178行`__disable_irq();`设置断点

#### 步骤2：运行调试

1. 按F5启动调试
2. 程序会停在断点处

#### 步骤3：查看调用栈

在Keil中：
- 打开`Call Stack`窗口（View -> Call Stack）
- 查看是哪个函数调用了`Error_Handler()`

在VSCode中：
- 查看左侧的`CALL STACK`面板
- 找到调用`Error_Handler()`的函数

**示例调用栈：**
```
#0  Error_Handler() at main.c:178
#1  HAL_ADCEx_Calibration_Start() at stm32f3xx_hal_adc_ex.c:xxx
#2  BSP_ADC_Init() at bsp_adc.c:59
#3  main() at main.c:xxx
```

这说明是ADC校准失败了。

---

### 方法2：修改Error_Handler记录错误信息

#### 修改Error_Handler函数

编辑`Core/Src/main.c`，修改`Error_Handler()`：

```c
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  
  // 记录错误位置
  static volatile const char* error_file = __FILE__;
  static volatile int error_line = __LINE__;
  
  // 在这里设置断点，查看error_file和error_line
  __disable_irq();
  while (1)
  {
    // 可以在这里闪烁LED指示错误
  }
  /* USER CODE END Error_Handler_Debug */
}
```

但这个方法不够精确，推荐使用方法3。

---

### 方法3：在每个Error_Handler调用处设置断点（推荐）

#### 步骤1：查找所有Error_Handler调用

在VSCode中按`Ctrl+Shift+F`，搜索：`Error_Handler()`

#### 步骤2：在可疑的地方设置断点

根据你的代码，最可能出错的地方：

**1. BSP_ADC_Init() - ADC初始化**
```c
// bsp_adc.c 第59行
if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK)
{
    Error_Handler();  // ← 在这里设置断点
}

// 第70行
if (HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED) != HAL_OK)
{
    Error_Handler();  // ← 在这里设置断点
}

// 第75行
if (HAL_ADCEx_InjectedStart_IT(&hadc1) != HAL_OK)
{
    Error_Handler();  // ← 在这里设置断点
}

// 第80行
if (HAL_ADCEx_InjectedStart_IT(&hadc2) != HAL_OK)
{
    Error_Handler();  // ← 在这里设置断点
}
```

**2. MX_ADC1_Init() / MX_ADC2_Init() - ADC配置**
```c
// adc.c
if (HAL_ADC_Init(&hadc1) != HAL_OK)
{
    Error_Handler();  // ← 在这里设置断点
}
```

**3. MX_HRTIM1_Init() - HRTIM初始化**
```c
// hrtim.c
if (HAL_HRTIM_Init(&hhrtim1) != HAL_OK)
{
    Error_Handler();  // ← 在这里设置断点
}
```

#### 步骤3：逐个测试

1. 在所有可疑的`Error_Handler()`调用处设置断点
2. 运行调试
3. 看程序停在哪个断点
4. 就知道是哪个函数出错了

---

### 方法4：使用自定义Error_Handler（最详细）

#### 创建增强版Error_Handler

编辑`Core/Src/main.c`：

```c
// 在文件开头添加
typedef struct {
    const char* file;
    int line;
    const char* func;
} ErrorInfo_t;

static ErrorInfo_t g_error_info = {0};

// 修改Error_Handler
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  
  // 在这里设置断点，查看g_error_info
  __disable_irq();
  
  // 闪烁LED指示错误
  while (1)
  {
    HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);
    for(volatile int i=0; i<1000000; i++);
  }
  /* USER CODE END Error_Handler_Debug */
}

// 添加新的错误处理函数
void Error_Handler_Ex(const char* file, int line, const char* func)
{
    g_error_info.file = file;
    g_error_info.line = line;
    g_error_info.func = func;
    Error_Handler();
}
```

然后在`bsp_adc.c`中使用：

```c
if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK)
{
    Error_Handler_Ex(__FILE__, __LINE__, __FUNCTION__);
}
```

---

## 🎯 根据你的代码，最可能的错误原因

### 1. ADC校准失败（最可能）

**原因：**
- ADC时钟未使能
- ADC已经在运行中
- ADC配置错误

**解决方法：**

在`BSP_ADC_Init()`中添加检查：

```c
void BSP_ADC_Init(void)
{
    HAL_StatusTypeDef status;
    
    // 确保ADC处于停止状态
    HAL_ADC_Stop(&hadc1);
    HAL_ADC_Stop(&hadc2);
    
    // 校准ADC1
    status = HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
    if (status != HAL_OK)
    {
        // 在这里设置断点，查看status的值
        Error_Handler();
    }
    
    // 校准ADC2
    status = HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);
    if (status != HAL_OK)
    {
        Error_Handler();
    }
    
    // 启动注入通道
    status = HAL_ADCEx_InjectedStart_IT(&hadc1);
    if (status != HAL_OK)
    {
        Error_Handler();
    }
    
    status = HAL_ADCEx_InjectedStart_IT(&hadc2);
    if (status != HAL_OK)
    {
        Error_Handler();
    }
}
```

---

### 2. ADC中断未使能

**检查：**

在`adc.c`的`HAL_ADC_MspInit()`中，确认中断已使能：

```c
void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{
    if(adcHandle->Instance==ADC1)
    {
        // 使能ADC时钟
        __HAL_RCC_ADC12_CLK_ENABLE();
        
        // 配置GPIO
        // ...
        
        // ✅ 确认这两行存在
        HAL_NVIC_SetPriority(ADC1_2_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(ADC1_2_IRQn);
    }
}
```

---

### 3. HRTIM初始化失败

**可能原因：**
- HRTIM时钟未使能
- HRTIM配置参数错误

**检查：**

在`hrtim.c`中，确认HRTIM时钟已使能。

---

## 📋 快速诊断步骤

### 步骤1：使用调试器

1. 在`Error_Handler()`第178行设置断点
2. 运行调试（F5）
3. 程序停在断点时，查看`Call Stack`
4. 找到调用`Error_Handler()`的函数

### 步骤2：定位具体错误

根据调用栈，在对应的函数中设置断点：

- 如果是`BSP_ADC_Init()` → 检查ADC配置
- 如果是`MX_HRTIM1_Init()` → 检查HRTIM配置
- 如果是`HAL_Init()` → 检查系统时钟配置

### 步骤3：查看HAL返回值

在出错的函数中，查看HAL函数的返回值：

```c
HAL_StatusTypeDef status = HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
// 在这里设置断点，查看status的值
if (status != HAL_OK)
{
    // status可能是：
    // HAL_ERROR = 0x01
    // HAL_BUSY = 0x02
    // HAL_TIMEOUT = 0x03
    Error_Handler();
}
```

---

## 🔧 常见错误和解决方法

### 错误1：ADC_ERROR_INTERNAL

**原因：** ADC内部错误，通常是配置问题

**解决：**
```c
// 检查ADC是否已经初始化
if (hadc1.State == HAL_ADC_STATE_RESET)
{
    // ADC未初始化
}
```

---

### 错误2：HAL_BUSY

**原因：** ADC正在运行中，无法校准

**解决：**
```c
// 先停止ADC
HAL_ADC_Stop(&hadc1);
HAL_ADC_Stop_IT(&hadc1);
HAL_ADCEx_InjectedStop(&hadc1);
HAL_ADCEx_InjectedStop_IT(&hadc1);

// 然后再校准
HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
```

---

### 错误3：HAL_TIMEOUT

**原因：** ADC操作超时

**解决：**
```c
// 检查ADC时钟是否使能
if (__HAL_RCC_ADC12_IS_CLK_ENABLED() == 0)
{
    // ADC时钟未使能
    __HAL_RCC_ADC12_CLK_ENABLE();
}
```

---

## 💡 调试技巧

### 技巧1：逐步注释代码

如果不确定哪里出错，逐步注释代码：

```c
void BSP_ADC_Init(void)
{
    // 先注释掉所有代码
    /*
    HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
    HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);
    HAL_ADCEx_InjectedStart_IT(&hadc1);
    HAL_ADCEx_InjectedStart_IT(&hadc2);
    */
    
    // 然后逐行取消注释，找到出错的那一行
    HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
    // 如果这里就出错了，说明是ADC1校准的问题
}
```

---

### 技巧2：使用LED指示错误位置

```c
void BSP_ADC_Init(void)
{
    HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_SET);  // LED0亮
    
    if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK)
    {
        // 如果LED0亮了但LED1没亮，说明是这里出错
        Error_Handler();
    }
    
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);  // LED1亮
    
    if (HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED) != HAL_OK)
    {
        // 如果LED1亮了但LED2没亮，说明是这里出错
        Error_Handler();
    }
    
    HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);  // LED2亮
}
```

---

### 技巧3：使用串口打印调试信息

```c
void BSP_ADC_Init(void)
{
    printf("开始ADC初始化...\n");
    
    printf("校准ADC1...\n");
    if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK)
    {
        printf("ADC1校准失败！\n");
        Error_Handler();
    }
    printf("ADC1校准成功\n");
    
    printf("校准ADC2...\n");
    if (HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED) != HAL_OK)
    {
        printf("ADC2校准失败！\n");
        Error_Handler();
    }
    printf("ADC2校准成功\n");
}
```

---

## 🎯 现在就开始调试！

### 第1步：设置断点

在`Error_Handler()`第178行设置断点

### 第2步：运行调试

按F5启动调试

### 第3步：查看调用栈

程序停在断点时，查看`Call Stack`窗口

### 第4步：告诉我结果

告诉我：
1. 调用栈显示是哪个函数调用了`Error_Handler()`？
2. 是在初始化哪个外设时出错的？

---

**现在去调试，然后告诉我调用栈的内容！** 🔍
