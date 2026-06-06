#ifndef TEMP_SENSOR_H
#define TEMP_SENSOR_H

#include <stdint.h>

#define VREF            3.3f        // 参考电压 (V)
#define ADC_RESOLUTION  4096.0f     // 12位ADC分辨率
#define FIXED_RESISTOR  10000.0f    // 串联固定电阻 (Ω)

/**
 * @brief 根据ADC原始值计算温度（摄氏度）
 * @param adc_value ADC转换结果（0~4095）
 * @return 温度值（℃），若超出表格范围则返回边界温度
 */
float get_temperature(uint32_t adc_value);

#endif /* TEMP_SENSOR_H */
