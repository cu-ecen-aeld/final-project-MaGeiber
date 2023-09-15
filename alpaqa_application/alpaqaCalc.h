#ifndef ALPAQACALC_H
#define ALPAQACALC_H

#include "SHT41.h"
#include "PMSA003I.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define C1 -42.378
#define C2 2.04901523
#define C3 10.14333127
#define C4 -0.22475541
#define C5 -0.00683783
#define C6 -0.05481717
#define C7 0.00122874
#define C8 0.00085282
#define C9 -0.00000199
#define COMPLEX_HEAT_INDEX(T, RH) (C1 + (C2 * T) + (C3 * RH) + (C4 * T * RH) + (C5 * T * T) + \
(C6 * RH *RH) + (C7 * T * T * RH) + (C8 * T * RH * RH) + (C9 * T * T * RH * RH))

#define COMPLEX_ADJUST_1_RH_LESS 13.0
#define COMPLEX_ADJUST_1_TEMP_GREATER 80
#define COMPLEX_ADJUST_1_TEMP_LESS 112
#define COMPLEX_ADJUST_2_RH_GREATER 85
#define COMPLEX_ADJUST_2_TEMP_GREATER 80
#define COMPLEX_ADJUST_2_TEMP_LESS 87
#define COMPLEX_ADJUST_2_FORMULA(T, RH) (((RH - 85) / 10) * ((87 - T) / 5))

#define SIMPLE_HEAT_INDEX(T, RH) ( 0.5 * (T + 61.0 + ((T - 68.0) * 1.2) + (RH * 0.094)))
#define SIMPLE_HEAT_FORMULA_THRESHOLD 80

#define AQI_BUFFER_SIZE (60 * 60 * 24)
#define BREAKPOINT_TABLE_SIZE 7
typedef struct
{
    uint16_t pm2_5;
    uint16_t pm10_0;
} AQI_DATA;

void initAlpaqaCalc();
float calcHeatIndex(const TEMP_HUMIDITY_DATA * data);
bool calcAQI(uint16_t * aqi);
uint16_t calcInstantAQI(const PARTICULATE_MATTER_DATA * data);
void storeAqiData(const PARTICULATE_MATTER_DATA * data);

#endif