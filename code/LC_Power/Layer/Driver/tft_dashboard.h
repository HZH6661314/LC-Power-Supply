#ifndef TFT_DASHBOARD_H
#define TFT_DASHBOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "tft_gfx.h"

void TFT_Dashboard_InitStatic(void);
void TFT_Dashboard_Init(void);
void TFT_Dashboard_Task(uint32_t now_ms);

#ifdef __cplusplus
}
#endif

#endif /* TFT_DASHBOARD_H */
