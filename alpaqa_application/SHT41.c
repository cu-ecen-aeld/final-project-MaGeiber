#include "SHT41.h"

static uint8_t rawData[6];

bool readTempAndHumidityFromDevice(int i2cFile)
{
    uint8_t writeCmd[1];

    if(ioctl(i2cFile, I2C_SLAVE, SHT41_ADDR) < 0)
    {
        // Failed to acquire bus access or communicate with device
        printf("Failed to acquire bus or communicate with SHT41\n");
        return false;
    }

    // Write precision we want, then wait
    writeCmd[0] = SHT41_HIGH_PRECISION;
    if(write(i2cFile, writeCmd, 1) != 1)
    {
        printf("Failed to write desired precision to SHT41");
        return false;
    }

    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = SHT41_HIGH_PRECISION_WAIT_MS * 1000000;
    nanosleep(&ts, NULL);

    if(read(i2cFile, rawData, SHT41_READ_BYTES) != SHT41_READ_BYTES)
    {
        // Failed to read
        printf("Failed to read data from SHT41\n");
        return false;
    }
    return true;
}

bool getTempAndHumidityData(TEMP_HUMIDITY_DATA *data)
{
    float rawTemperature;
    float rawHumidity;

    // Raw temperature data is bytes 0 and 1, byte 2 is checksum (ignored for now)
    rawTemperature = (rawData[0] * 256) + rawData[1];

    // Conversion to degrees f from datasheet: -49 + 315*(rawTemperature / 65535)
    data->temperature = -49 + (315 * rawTemperature / 65535);

    // Raw humidity data is bytes 3 and 4, byte 5 is checksum (ignored for now)
    rawHumidity = (rawData[3] * 256) + rawData[4];

    // Conversion to % relative humidity from datasheet: -6 + 125*(rawHumidity / 65535)
    data->humidity = -6 + ( 125 * rawHumidity / 65535 );
    
    // Crop relative humidity to bounds (0%-100%)
    if(data->humidity > 100)
    {
        data->humidity = 100;
    }
    if(data->humidity < 0)
    {
        data->humidity = 0;
    }

    return true;
}
