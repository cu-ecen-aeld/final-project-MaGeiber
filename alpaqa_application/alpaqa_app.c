#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "PMSA003I.h"
#include "SHT41.h"
#include "alpaqaCalc.h"

#define ESCAPE_CLEAR_SCREEN "\e[2J"
#define ESCAPE_CURSOR_PREVIOUS "\e[6F"
#define ESCAPE_CLEAR_LINE "\e[2K"

#define ALPAQA_LOG_FILE "/var/log/alpaqa/alpaqa_log.txt"
#define I2C_DEVICE_FILENAME "/dev/i2c-1"

bool alpaqaRunning;

static void signalHandler(int signalNumber);

int main()
{
    FILE * logFile;
    struct sigaction sigAction;
    int i2cFile;
    PARTICULATE_MATTER_DATA particulateData;
    TEMP_HUMIDITY_DATA tempHumidityData;
    float heatIndex;
    uint16_t calculatedAqi;
    uint16_t instantAqi;
    bool aqiFull24Hour;

    alpaqaRunning = true;

    printf(ESCAPE_CLEAR_SCREEN);
    printf("========================\n");
    printf("A.L.P.A.Q.A. Data Logger\n");
    printf("========================\n");

    // register the signal handler to allow graceful quitting
    memset(&sigAction, 0, sizeof(struct sigaction));
    sigAction.sa_handler = signalHandler;
    if( sigaction(SIGTERM, &sigAction, NULL) != 0)
    {
    }
    if( sigaction(SIGINT, &sigAction, NULL) != 0)
    {
    }

    logFile = fopen(ALPAQA_LOG_FILE, "a+");
    if(logFile == NULL)
    {
        printf("Failed to open log file: %s ! errno: %d\n", ALPAQA_LOG_FILE, errno);
    }

    // Attempt to open i2c device
    if( (i2cFile = open(I2C_DEVICE_FILENAME, O_RDWR)) < 0)
    {
        printf("Failed to open the I2C Bus! errno: %d\n", errno);
    }

    initAlpaqaCalc();

    while(alpaqaRunning)
    {
        // Read data from AQI sensor
        if(readAqiDataFromDevice(i2cFile) == true)
        {
            getParticulateMatterData(&particulateData);
            storeAqiData(&particulateData);
            aqiFull24Hour = calcAQI(&calculatedAqi);
            instantAqi = calcInstantAQI(&particulateData);
        }

        // Read data from Temperature and Humidity sensor
        if(readTempAndHumidityFromDevice(i2cFile) == true)
        {
            getTempAndHumidityData(&tempHumidityData);
            heatIndex = calcHeatIndex(&tempHumidityData);
        }

        printf("Air Quality: Particulate Matter:\n");
        printf(ESCAPE_CLEAR_LINE);
        printf("PM 1.0: %d PM 2.5: %d PM 10.0: %d\n", particulateData.pm1_0, particulateData.pm2_5, particulateData.pm10_0);

        printf(ESCAPE_CLEAR_LINE);
        printf("AQI now: %d\n", instantAqi);

        printf(ESCAPE_CLEAR_LINE);
        if(aqiFull24Hour)
        {
            printf("Calculated AQI (24 hour): %d\n", calculatedAqi);
        }
        else
        {
            printf("Calculated AQI (Running Average): %d\n", calculatedAqi);
        }
        
        printf(ESCAPE_CLEAR_LINE);
        printf("Temperature: %0.2f F / %0.2f C\n", tempHumidityData.temperatureF, tempHumidityData.temperatureC);
        printf(ESCAPE_CLEAR_LINE);
        printf("Humidity: %0.2f\n", tempHumidityData.humidity);
        printf(ESCAPE_CLEAR_LINE);
        printf("Heat Index: %0.2f", heatIndex);
        printf(ESCAPE_CURSOR_PREVIOUS);
        sleep(1);
    }

    if(logFile != NULL)
    {
        fclose(logFile);
    }
    else
    {
        printf("Log file was not written!");
    }

    return 0;
}

static void signalHandler(int signalNumber)
{
    int errnoSaved = errno;
    switch(signalNumber)
    {
        case SIGINT:
        case SIGTERM:
            alpaqaRunning = false;
            break;

        default:
            break;
    }
    errno = errnoSaved;
}