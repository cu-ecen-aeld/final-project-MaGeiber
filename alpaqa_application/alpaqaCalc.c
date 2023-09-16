#include "alpaqaCalc.h"

typedef struct
{
    uint16_t breakpointMin;
    uint16_t breakpointMax;
} BREAKPOINTS_PAIR;

typedef struct
{
    BREAKPOINTS_PAIR breakpointPm2_5;
    BREAKPOINTS_PAIR breakpoints_pm10_0;
    BREAKPOINTS_PAIR breakpoints_aqi;
} BREAKPOINTS;
// Note: I rounded PM2.5 values, since the sensor precision is 1 ug/m^3
const BREAKPOINTS breakpoints_table[BREAKPOINT_TABLE_SIZE] =
{
    {{0, 12}, {0, 54}, {0, 50}},
    {{13, 35}, {55, 154}, {51, 100}},
    {{36, 55}, {155, 254}, {101, 150}},
    {{56, 150}, {255, 354}, {151, 200}},
    {{151, 250}, {355, 424}, {201, 300}},
    {{251, 350}, {425, 504}, {301, 400}},
    {{351, 500}, {505, 604}, {401, 500}}
};

static AQI_DATA aqiBuffer[AQI_BUFFER_SIZE];
static uint32_t bufferIdx;
static uint32_t runningSumPm2_5;
static uint32_t runningSumPm10_0;
static bool bufferFull;

static uint16_t calculateAqiIndex(uint16_t pm2_5, uint16_t pm10_0);
static uint16_t calculatePollutantIndex(uint16_t pollutantConcentration, uint16_t breakpointHigh, uint16_t breakpointLow, uint16_t aqiHigh, uint16_t aqiLow);

void initAlpaqaCalc()
{
    memset(aqiBuffer, 0, sizeof(aqiBuffer));
    bufferIdx = 0;
    bufferFull = false;
    runningSumPm2_5 = 0;
    runningSumPm10_0 = 0;
}

// This uses the heat index equations given by NOAA, using a simple equation first
// and using the more advanced calculations if needed
float calcHeatIndex(const TEMP_HUMIDITY_DATA * data)
{
    float heatIndex;
    float adjustment;

    // Simple calculation
    heatIndex = SIMPLE_HEAT_INDEX(data->temperatureF, data->humidity);

    if(((heatIndex + data->temperatureF) / 2) >= SIMPLE_HEAT_FORMULA_THRESHOLD)
    {
        heatIndex = COMPLEX_HEAT_INDEX(data->temperatureF, data->humidity);

        // Special adjustments
        if(data->humidity < COMPLEX_ADJUST_1_RH_LESS &&
        data->temperatureF > 80 &&
        data->temperatureF < 112)
        {
            adjustment = (((13 - data->humidity) / 4) * sqrt((17-abs(data->temperatureF - 95)) / 17));
            heatIndex -= adjustment;
        }

        if(data->humidity > COMPLEX_ADJUST_2_RH_GREATER &&
        data->temperatureF > COMPLEX_ADJUST_2_TEMP_GREATER &&
        data->temperatureF < COMPLEX_ADJUST_2_TEMP_LESS)
        {
            heatIndex += COMPLEX_ADJUST_2_FORMULA(data->temperatureF, data->humidity);
        }
    }

    return heatIndex;
}

// This calculates the AQI using the forumulas and guidance in the U.S. EPA Technical Assistance Document for AQI.
// The function uses the PM2.5 and PM10 data as that is what the connected PMSA003I sensor has that is applicable.
// Returns: Boolean indicating if enough historical data exists for accurate 24 hour guidance. Without 24 hours of
// data, the calculation will be a best effort averaged estimate with the data that has been collected so far.
// It may be useful to expand this to use the NowCast algorithm for shorter term calculations
bool calcAQI(uint16_t * aqi)
{
    uint16_t averagePm2_5;
    uint16_t averagePm10_0;
    uint32_t numberOfSamples;

    // If the buffer is full, we have 24 hours of samples
    if(bufferFull)
    {
        numberOfSamples = AQI_BUFFER_SIZE;
    }
    // Otherwise, use the buffer count. No zero.
    else
    {
        numberOfSamples = bufferIdx > 0 ? bufferIdx : 1;
    }

    // Find the averages collected so far
    averagePm2_5 = runningSumPm2_5 / bufferIdx;
    averagePm10_0 = runningSumPm10_0 / bufferIdx;

    // Return the calculated index
    *aqi = calculateAqiIndex(averagePm2_5, averagePm10_0);

    return bufferFull;
}

// Calculates an instant AQI based on PM data now. Not as useful and fluctuates more.
uint16_t calcInstantAQI(const PARTICULATE_MATTER_DATA * data)
{
    return calculateAqiIndex(data->pm2_5, data->pm10_0);
}

void storeAqiData(const PARTICULATE_MATTER_DATA * data)
{
    // Keep a running sum to save time, add the new values to the sum
    runningSumPm2_5 += data->pm2_5;
    runningSumPm10_0 += data->pm10_0;

    // If the buffer is full, subtract the last value (what is being overwritten) from the running sum
    if(bufferFull)
    {
        runningSumPm2_5 -= aqiBuffer[bufferIdx].pm2_5;
        runningSumPm10_0 -= aqiBuffer[bufferIdx].pm10_0;
    }

    // Update the buffer with the new values
    aqiBuffer[bufferIdx].pm2_5 = data->pm2_5;
    aqiBuffer[bufferIdx].pm10_0 = data->pm10_0;

    // Increment index and mark if the buffer is full
    bufferIdx++;
    if(bufferIdx >= AQI_BUFFER_SIZE)
    {
        bufferIdx = 0;
        bufferFull = true;
    }
}

uint16_t calculateAqiIndex(uint16_t pm2_5, uint16_t pm10_0)
{
    uint16_t breakpointMin;
    uint16_t breakpointMax;
    uint16_t aqiMin;
    uint16_t aqiMax;
    uint16_t maxPm;
    uint8_t idx;

    // Probably a better way to do this...maybe revisit
    for(idx = 0; idx < BREAKPOINT_TABLE_SIZE; idx++)
    {
        if(pm2_5 > pm10_0)
        {
            if( pm2_5 >= breakpoints_table[idx].breakpointPm2_5.breakpointMin &&
            pm2_5 <= breakpoints_table[idx].breakpointPm2_5.breakpointMax)
            {
                breakpointMin = breakpoints_table[idx].breakpointPm2_5.breakpointMin;
                breakpointMax = breakpoints_table[idx].breakpointPm2_5.breakpointMax;
                aqiMin = breakpoints_table[idx].breakpoints_aqi.breakpointMin;
                aqiMax = breakpoints_table[idx].breakpoints_aqi.breakpointMax;
                maxPm = pm2_5;
            }
        }
        else
        {
            if( pm10_0 >= breakpoints_table[idx].breakpoints_pm10_0.breakpointMin &&
            pm10_0 <= breakpoints_table[idx].breakpoints_pm10_0.breakpointMax)
            {
                breakpointMin = breakpoints_table[idx].breakpoints_pm10_0.breakpointMin;
                breakpointMax = breakpoints_table[idx].breakpoints_pm10_0.breakpointMax;
                aqiMin = breakpoints_table[idx].breakpoints_aqi.breakpointMin;
                aqiMax = breakpoints_table[idx].breakpoints_aqi.breakpointMax;
                maxPm = pm10_0;
            }
        }
    }

    return calculatePollutantIndex(maxPm, breakpointMax, breakpointMin, aqiMax, aqiMin);
}

uint16_t calculatePollutantIndex(uint16_t pollutantConcentration, uint16_t breakpointHigh, uint16_t breakpointLow, uint16_t aqiHigh, uint16_t aqiLow)
{
    // Equation 1 from the aqi technical assistance document
    return (uint16_t)(((float)(aqiHigh-aqiLow)/(breakpointHigh-breakpointLow)) * (float)(pollutantConcentration - breakpointLow) + aqiLow);
}
