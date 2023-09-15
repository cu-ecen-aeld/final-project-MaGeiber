#ifndef SHT41_H
#define SHT41_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <time.h>
#include <linux/i2c-dev.h>
#include <unistd.h>

#define SHT41_ADDR 0x44
#define SHT41_HIGH_PRECISION 0xFD
#define SHT41_READ_BYTES 6

#define SHT41_HIGH_PRECISION_WAIT_MS 9

typedef struct 
{
    float temperatureF;
    float temperatureC;
    float humidity;
} TEMP_HUMIDITY_DATA;

bool readTempAndHumidityFromDevice(int i2cFile);
bool getTempAndHumidityData(TEMP_HUMIDITY_DATA *data);

#endif