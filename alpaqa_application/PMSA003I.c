#include "PMSA003I.h"

static uint8_t rawData[32];

bool readAqiDataFromDevice(int i2cFile)
{
    if(ioctl(i2cFile, I2C_SLAVE, PMSA003I_ADDR) < 0)
    {
        // Failed to acquire bus access or communicate with device
        printf("Failed to acquire bus or communicate with PMSA003I\n");
        return false;
    }

    if(read(i2cFile, rawData, PMSA003I_READ_BYTES) != PMSA003I_READ_BYTES)
    {
        // Failed to read
        printf("Failed to read data from PMSA003I\n");
        return false;
    }
    return true;
}

// Units are micro grams / meters^3
bool getAqiData(AQI_DATA *data)
{
    // Currently only want environmental PM for 1.0, 2.5, and 10
    // So, bytes 10/11, 12/13, and 14/15 respectively
    // Byte order is: High/Low
    data->pm1_0 = (rawData[10] << 8) | rawData[11];
    data->pm2_5 = (rawData[12] << 8) | rawData[13];
    data->pm10_0 = (rawData[14] << 8) | rawData[15];
    return true;
}
