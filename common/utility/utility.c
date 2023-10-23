#include <stdint.h>
#include "utility.h"

inline uint32_t Get_Freq(uint32_t systemClk, uint32_t timerPrescaler, uint32_t timerPeriod){
    return (systemClk / (timerPrescaler+1)) / (timerPeriod+1);
}

inline int32_t map(int32_t x, int32_t x_min, int32_t x_max, int32_t out_min, int32_t out_max){

	if(x <= x_min)
		return out_min;

	if(x >= x_max)
		return out_max;

	return (x - x_min) * (out_max - out_min) / (x_max - x_min) + out_min;

}