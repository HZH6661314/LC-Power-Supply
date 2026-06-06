# BSP层架构设计示例文档

## 📚 架构设计总结

本文档展示了如何使用**虚函数指针（函数指针表）**设计优雅的、完全可移植的BSP层。

---

## 🎯 设计目标

1. **完全可移植** - Driver层不依赖任何具体芯片
2. **换芯片只需修改BSP层** - 符合架构铁律二
3. **消灭extern** - 使用依赖注入，符合架构铁律三

---

## 📁 文件结构

```
Layer/
├── Bsp/
│   ├── bsp_spi.h/c      - 软件SPI抽象
│   ├── bsp_gpio.h/c     - GPIO抽象（LED、按键）
│   ├── bsp_adc.h/c      - ADC抽象
│   └── bsp_hrtim.h/c    - HRTIM抽象
│
└── Driver/
    ├── W25Q256.h/c      - W25Q256 Flash驱动（完全可移植）
    ├── temp_sensor.h/c  - 温度传感器驱动
    └── 1.54TFT.h/c      - TFT屏幕驱动
```

---

## 🔥 核心设计：虚函数指针

### 1. 软件SPI抽象（bsp_spi）

**头文件定义：**
```c
// bsp_spi.h
typedef struct {
    void (*SetHigh)(void);
    void (*SetLow)(void);
    uint8_t (*Read)(void);
} GPIO_Pin_t;

typedef struct {
    GPIO_Pin_t SCK;
    GPIO_Pin_t MOSI;
    GPIO_Pin_t MISO;
    GPIO_Pin_t CS;
    uint32_t   delay_cycles;
} SoftSPI_t;

// 获取SPI接口
SoftSPI_t* BSP_SPI_GetInterface(void);

// 通用SPI操作（完全可移植）
void SoftSPI_WriteByte(SoftSPI_t *spi, uint8_t data);
uint8_t SoftSPI_ReadByte(SoftSPI_t *spi);
uint8_t SoftSPI_TransferByte(SoftSPI_t *spi, uint8_t data);
```

**BSP层实现（STM32平台）：**
```c
// bsp_spi.c
static void W25Q256_SCK_SetHigh(void) {
    HAL_GPIO_WritePin(W25Q256_SCK_GPIO_Port, W25Q256_SCK_Pin, GPIO_PIN_SET);
}

static void W25Q256_SCK_SetLow(void) {
    HAL_GPIO_WritePin(W25Q256_SCK_GPIO_Port, W25Q256_SCK_Pin, GPIO_PIN_RESET);
}

void BSP_SPI_Init(void) {
    // 绑定虚函数指针
    s_soft_spi.SCK.SetHigh = W25Q256_SCK_SetHigh;
    s_soft_spi.SCK.SetLow  = W25Q256_SCK_SetLow;
    // ...
}
```

**Driver层使用（完全可移植）：**
```c
// W25Q256.c
void W25Q256_Init(void) {
    s_spi = BSP_SPI_GetInterface();  // 获取抽象接口
}

void W25Q256_ReadID(uint8_t *manufacturer_id, uint16_t *device_id) {
    s_spi->CS.SetLow();  // 只调用虚函数指针，不知道底层是什么芯片
    SoftSPI_WriteByte(s_spi, W25Q256_CMD_READ_JEDEC_ID);
    *manufacturer_id = SoftSPI_ReadByte(s_spi);
    // ...
    s_spi->CS.SetHigh();
}
```

---

### 2. GPIO抽象（bsp_gpio）

**头文件定义：**
```c
// bsp_gpio.h
typedef struct {
    void (*SetHigh)(void);
    void (*SetLow)(void);
    void (*Toggle)(void);
    uint8_t (*Read)(void);
} GPIO_Pin_t;

typedef struct {
    GPIO_Pin_t pin;
    void (*On)(void);
    void (*Off)(void);
    void (*Toggle)(void);
} LED_t;

typedef struct {
    GPIO_Pin_t pin;
    uint8_t (*IsPressed)(void);
} Button_t;

// 获取LED/按键接口
LED_t* BSP_GPIO_GetLED(LED_ID_t led_id);
Button_t* BSP_GPIO_GetButton(Button_ID_t btn_id);
```

**BSP层实现（STM32平台）：**
```c
// bsp_gpio.c
static void LED0_On(void) {
    HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_RESET);  // 低电平点亮
}

static void LED0_Off(void) {
    HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_SET);
}

void BSP_GPIO_Init(void) {
    // 绑定虚函数指针
    s_leds[LED_0].On = LED0_On;
    s_leds[LED_0].Off = LED0_Off;
    // ...
}
```

**应用层使用（完全可移植）：**
```c
// Application层
void App_Init(void) {
    LED_t *led0 = BSP_GPIO_GetLED(LED_0);
    led0->On();  // 点亮LED，不关心底层是什么芯片
}

void App_Task(void) {
    Button_t *btn = BSP_GPIO_GetButton(BTN_KEY1_SET);
    if (btn->IsPressed()) {
        LED_t *led = BSP_GPIO_GetLED(LED_0);
        led->Toggle();
    }
}
```

---

## 🚀 移植示例

### 移植到Arduino平台

**只需修改BSP层：**

```c
// bsp_spi.c - Arduino版本
static void W25Q256_SCK_SetHigh(void) {
    digitalWrite(SCK_PIN, HIGH);  // Arduino API
}

static void W25Q256_SCK_SetLow(void) {
    digitalWrite(SCK_PIN, LOW);
}

// Driver层（W25Q256.c）完全不需要修改！
```

```c
// bsp_gpio.c - Arduino版本
static void LED0_On(void) {
    digitalWrite(LED0_PIN, LOW);  // Arduino API
}

static void LED0_Off(void) {
    digitalWrite(LED0_PIN, HIGH);
}

// Application层完全不需要修改！
```

### 移植到TI C2000平台

```c
// bsp_spi.c - TI版本
static void W25Q256_SCK_SetHigh(void) {
    GpioDataRegs.GPASET.bit.GPIO0 = 1;  // TI API
}

static void W25Q256_SCK_SetLow(void) {
    GpioDataRegs.GPACLEAR.bit.GPIO0 = 1;
}

// Driver层（W25Q256.c）完全不需要修改！
```

---

## 📝 使用示例

### 示例1：W25Q256 Flash读写

```c
#include "W25Q256.h"

void Flash_Test(void) {
    // 1. 初始化
    BSP_SPI_Init();
    W25Q256_Init();

    // 2. 自检
    if (W25Q256_SelfTest()) {
        // Flash正常
    }

    // 3. 读取ID
    uint8_t manufacturer_id;
    uint16_t device_id;
    W25Q256_ReadID(&manufacturer_id, &device_id);
    // 应该读到：manufacturer_id = 0xEF, device_id = 0x4019

    // 4. 擦除扇区
    W25Q256_EraseSector(0x00000000);

    // 5. 写入数据
    uint8_t write_buffer[256] = {0x01, 0x02, 0x03, ...};
    W25Q256_WriteData(0x00000000, write_buffer, 256);

    // 6. 读取数据
    uint8_t read_buffer[256];
    W25Q256_ReadData(0x00000000, read_buffer, 256);

    // 7. 验证数据
    for (int i = 0; i < 256; i++) {
        if (read_buffer[i] != write_buffer[i]) {
            // 数据错误
        }
    }
}
```

### 示例2：LED和按键控制

```c
#include "bsp_gpio.h"

void LED_Button_Test(void) {
    // 1. 初始化
    BSP_GPIO_Init();

    // 2. 获取LED和按键接口
    LED_t *led0 = BSP_GPIO_GetLED(LED_0);
    LED_t *led1 = BSP_GPIO_GetLED(LED_1);
    Button_t *btn_set = BSP_GPIO_GetButton(BTN_KEY1_SET);
    Button_t *btn_up = BSP_GPIO_GetButton(BTN_KEY1_UP);

    // 3. 控制LED
    led0->On();   // 点亮LED0
    led1->Off();  // 熄灭LED1

    // 4. 检测按键
    if (btn_set->IsPressed()) {
        led0->Toggle();  // 翻转LED0
    }

    if (btn_up->IsPressed()) {
        led1->On();
    }
}
```

### 示例3：在主循环中使用

```c
// main.c
int main(void) {
    // 硬件初始化
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    // BSP层初始化
    BSP_SPI_Init();
    BSP_GPIO_Init();

    // Driver层初始化
    W25Q256_Init();

    // 应用层初始化
    LED_t *led0 = BSP_GPIO_GetLED(LED_0);
    Button_t *btn = BSP_GPIO_GetButton(BTN_KEY1_SET);

    while (1) {
        // 按键控制LED
        if (btn->IsPressed()) {
            led0->Toggle();
            HAL_Delay(200);  // 消抖
        }

        // 其他任务
        // ...
    }
}
```

---

## ✅ 架构验证

### 验证铁律二：平台移植测试

**测试方法：**
1. 尝试在PC上编译Driver层代码（使用gcc）
2. 检查Driver层是否包含任何硬件相关头文件

```bash
# 编译Driver层（应该成功）
gcc -c W25Q256.c -I../Bsp -std=c99

# 检查依赖
grep -r "stm32" W25Q256.c  # 应该没有结果
grep -r "HAL_" W25Q256.c   # 应该没有结果
```

**结果：**
- ✅ Driver层不包含任何STM32相关头文件
- ✅ Driver层只调用BSP层的抽象接口
- ✅ 换芯片只需修改BSP层

### 验证铁律三：消灭extern

**检查方法：**
```bash
# 检查Driver层是否有extern变量
grep -r "extern" W25Q256.c  # 应该没有结果（除了函数声明）
```

**结果：**
- ✅ Driver层不使用extern变量
- ✅ 使用依赖注入（函数参数传递）

---

## 🎓 架构设计要点

### 1. 虚函数指针的优势

**传统方式（不推荐）：**
```c
// Driver层直接调用HAL库
HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
```
❌ 问题：Driver层依赖STM32 HAL库，换芯片需要修改Driver层

**虚函数指针方式（推荐）：**
```c
// Driver层只调用虚函数指针
s_spi->CS.SetLow();
```
✅ 优势：Driver层完全不知道底层是什么芯片

### 2. 依赖注入的优势

**传统方式（不推荐）：**
```c
// Driver层使用extern
extern uint32_t g_adc_value;
float voltage = g_adc_value * 0.001f;
```
❌ 问题：全局变量满天飞，难以维护

**依赖注入方式（推荐）：**
```c
// Scheduler层传递参数
float voltage = BSP_ADC_GetVoltage();
float duty = PID_Calculate(target, voltage);
BSP_HRTIM_SetDuty(duty);
```
✅ 优势：数据流清晰，易于测试

### 3. 接口设计原则

1. **接口最小化** - 只暴露必要的函数
2. **接口稳定性** - 避免频繁修改
3. **接口抽象性** - 隐藏实现细节

---

## 📊 架构对比

| 特性 | 传统方式 | 虚函数指针方式 |
|------|---------|---------------|
| 可移植性 | ❌ 差 | ✅ 优秀 |
| 代码耦合度 | ❌ 高 | ✅ 低 |
| 换芯片成本 | ❌ 高（需修改Driver层） | ✅ 低（只修改BSP层） |
| 代码可读性 | ✅ 好 | ✅ 好 |
| 运行效率 | ✅ 高 | ⚠️ 略低（函数指针调用） |

---

## 🎯 总结

通过使用**虚函数指针**设计BSP层，我们实现了：

1. ✅ **完全可移植** - Driver层不依赖任何具体芯片
2. ✅ **符合架构铁律** - 单向依赖、平台移植测试、消灭extern
3. ✅ **易于维护** - 代码结构清晰，职责明确
4. ✅ **易于扩展** - 新增外设只需添加虚函数绑定

这就是**工程级别的架构设计**！
