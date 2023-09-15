#ifndef PMSA003I_H
#define PMSA003I_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <unistd.h>

#define PMSA003I_ADDR 0x12
#define PMSA003I_READ_BYTES 32

typedef struct 
{
    uint16_t pm1_0;
    uint16_t pm2_5;
    uint16_t pm10_0;
} PARTICULATE_MATTER_DATA;

bool readAqiDataFromDevice(int i2cFile);
bool getParticulateMatterData(PARTICULATE_MATTER_DATA * data);

#endif