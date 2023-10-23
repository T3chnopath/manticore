#ifndef __UTILITY_H
#define __UTILITY_H

#include <stdint.h>
#include "utility.h"

inline uint32_t Get_Freq(uint32_t systemClk, uint32_t timerPrescaler, uint32_t timerPeriod);
inline int32_t map(int32_t x, int32_t x_min, int32_t x_max, int32_t out_min, int32_t out_max);

#endif