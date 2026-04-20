#include "temp_sensor.h"
#include <math.h>

// 温度表 (℃)
static const float temp_table[] = {
    -40, -39, -38, -37, -36, -35, -34, -33, -32, -31,
    -30, -29, -28, -27, -26, -25, -24, -23, -22, -21,
    -20, -19, -18, -17, -16, -15, -14, -13, -12, -11,
    -10, -9,  -8,  -7,  -6,  -5,  -4,  -3,  -2,  -1,
    0,   1,   2,   3,   4,   5,   6,   7,   8,   9,
    10,  11,  12,  13,  14,  15,  16,  17,  18,  19,
    20,  21,  22,  23,  24,  25,  26,  27,  28,  29,
    30,  31,  32,  33,  34,  35,  36,  37,  38,  39,
    40,  41,  42,  43,  44,  45,  46,  47,  48,  49,
    50,  51,  52,  53,  54,  55,  56,  57,  58,  59,
    60,  61,  62,  63,  64,  65,  66,  67,  68,  69,
    70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
    80,  81,  82,  83,  84,  85,  86,  87,  88,  89,
    90,  91,  92,  93,  94,  95,  96,  97,  98,  99,
    100, 101, 102, 103, 104, 105, 106, 107, 108, 109,
    110, 111, 112, 113, 114, 115, 116, 117, 118, 119,
    120, 121, 122, 123, 124, 125
};

// 对应温度下的典型电阻值 (Ω) —— 取自 Rnor 列，已转换为欧姆
static const float res_table[] = {
    343632.6f, 321280.9f, 300533.9f, 281266.0f, 263362.4f,
    246717.7f, 231235.5f, 216827.3f, 203411.8f, 190914.4f,
    179266.6f, 168405.3f, 158272.6f, 148815.1f, 139983.7f,
    131733.2f, 124021.6f, 116810.7f, 110064.8f, 103751.2f,
    97839.6f,  92302.0f,  87112.4f,  82247.1f,  77683.7f,
    73401.8f,  69382.3f,  65607.7f,  62061.6f,  58728.8f,
    55595.3f,  52648.0f,  49874.7f,  47264.3f,  44806.2f,
    42490.6f,  40308.6f,  38251.6f,  36311.7f,  34481.7f,
    32754.7f,  31124.3f,  29584.7f,  28130.1f,  26755.6f,
    25456.2f,  24227.4f,  23065.0f,  21965.0f,  20923.9f,
    19938.0f,  19004.1f,  18119.3f,  17280.7f,  16485.7f,
    15731.7f,  15016.4f,  14337.6f,  13693.3f,  13081.6f,
    12500.5f,  11948.5f,  11423.9f,  10925.2f,  10451.0f,
    10000.0f,  9570.9f,   9162.6f,   8773.8f,   8403.7f,
    8051.2f,   7715.4f,   7395.3f,   7090.3f,   6799.5f,
    6522.1f,   6257.6f,   6005.1f,   5764.2f,   5534.2f,
    5314.6f,   5104.9f,   4904.5f,   4713.0f,   4530.0f,
    4355.1f,   4187.8f,   4027.8f,   3874.8f,   3728.3f,
    3588.2f,   3454.0f,   3325.5f,   3202.5f,   3084.6f,
    2971.7f,   2863.5f,   2759.7f,   2660.3f,   2564.9f,
    2473.4f,   2385.6f,   2301.4f,   2220.6f,   2143.1f,
    2068.6f,   1997.0f,   1928.3f,   1862.3f,   1798.9f,
    1738.0f,   1679.4f,   1623.1f,   1568.9f,   1516.8f,
    1466.7f,   1418.5f,   1372.2f,   1327.5f,   1284.5f,
    1243.1f,   1203.3f,   1164.9f,   1127.9f,   1092.3f,
    1058.0f,   1024.9f,   993.0f,    962.3f,    932.6f,
    904.0f,    876.4f,    849.8f,    824.1f,    799.4f,
    775.4f,    752.3f,    730.0f,    708.5f,    687.7f,
    667.6f,    648.2f,    629.5f,    611.3f,    593.8f,
    576.9f,    560.5f,    544.7f,    529.3f,    514.5f,
    500.2f,    486.3f,    472.9f,    459.9f,    447.4f,
    435.2f,    423.4f,    412.0f,    400.9f,    390.2f,
    379.9f,    369.8f,    360.1f,    350.6f,    341.5f,
    332.6f
};

#define TABLE_SIZE  (sizeof(temp_table) / sizeof(temp_table[0]))

/**
 * @brief 计算热敏电阻阻值 (Ω)
 * @param adc_val ADC原始值 (0~4095)
 * @return 热敏电阻阻值，若电压异常返回 -1
 */
static float calc_resistance(uint32_t adc_val) {
    // 防止极端电压导致除零
    if (adc_val >= ADC_RESOLUTION - 1) {
        return -1.0f;   // 电压接近 VREF，阻值极大，温度极低
    }
    if (adc_val <= 1) {
        return -1.0f;   // 电压接近 0，阻值极小，温度极高
    }

    float voltage = (float)adc_val / ADC_RESOLUTION * VREF;
    // 分压公式：热敏电阻电压 = Vref * Rt / (Rfix + Rt)
    // 推导出 Rt = Rfix * V_ADC / (Vref - V_ADC)
    float rt = FIXED_RESISTOR * voltage / (VREF - voltage);
    return rt;
}

/**
 * @brief 优化查表：使用对数线性插值
 * @param rt 实测电阻值 (Ω)
 * @return 温度 (℃)
 */
static float lookup_temperature(float rt) {
    // 边界检查
    if (rt >= res_table[0]) {
        return temp_table[0];   // 低于 -40℃
    }
    if (rt <= res_table[TABLE_SIZE - 1]) {
        return temp_table[TABLE_SIZE - 1]; // 高于 125℃
    }

    // 找到 rt 所在的区间
    for (uint8_t i = 1; i < TABLE_SIZE; i++) {
        if (rt >= res_table[i]) {
            // 区间 [i, i-1]
            float t_low  = temp_table[i];
            float t_high = temp_table[i-1];
            float r_low  = res_table[i];
            float r_high = res_table[i-1];

            // 对数线性插值：ln(R) 与温度近似线性
            float ln_r_low  = logf(r_low);
            float ln_r_high = logf(r_high);
            float ln_rt     = logf(rt);

            float t = t_low + (t_high - t_low) * (ln_rt - ln_r_low) / (ln_r_high - ln_r_low);
            return t;
        }
    }

    return 0.0f; // 不应该执行到这里
}

/**
 * @brief 对外接口：根据ADC值获取温度
 */
float get_temperature(uint32_t adc_value) {
    float rt = calc_resistance(adc_value);
    if (rt < 0) {
        // 电压异常，返回边界温度
        return (adc_value >= ADC_RESOLUTION - 1) ? temp_table[0] : temp_table[TABLE_SIZE - 1];
    }
    return lookup_temperature(rt);
}
