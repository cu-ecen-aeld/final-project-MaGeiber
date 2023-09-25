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
#define BOLD "\e[1m"
#define DEFAULT "\e[0m"

#define BANNER_SIZE 3
#define SECTION_SPACING 2
#define ALPAQA_BANNER_LINE 1
#define PM_BANNER_LINE (ALPAQA_BANNER_LINE + BANNER_SIZE + 1 + SECTION_SPACING)
#define PM_DATA_SIZE 3
#define SHT_BANNER_LINE (PM_BANNER_LINE + BANNER_SIZE + PM_DATA_SIZE + SECTION_SPACING)
#define SHT_DATA_SIZE 3
#define SYS_INFO_BANNER_LINE (SHT_BANNER_LINE + BANNER_SIZE + SHT_DATA_SIZE + SECTION_SPACING)

#define PM_DATA_START_LINE (PM_BANNER_LINE + BANNER_SIZE)
#define SHT_DATA_START_LINE (SHT_BANNER_LINE + BANNER_SIZE)

#define SYS_INFO_LOG_LINE (SYS_INFO_BANNER_LINE + BANNER_SIZE)
#define SYS_INFO_LOG_SIZE_LINE (SYS_INFO_LOG_LINE + 1)
#define SYS_INFO_I2C_LINE (SYS_INFO_LOG_SIZE_LINE + 1)
#define SYS_INFO_PM_LINE (SYS_INFO_I2C_LINE + 1)
#define SYS_INFO_TEMPERATURE_LINE (SYS_INFO_PM_LINE + 1)

#define BLACK_BG 40
#define RED_FG 31
#define RED_BG 41
#define GREEN_FG 32
#define CYAN_FG 36
#define CYAN_BG 46
#define WHITE_FG 37

#define BUFFER_SIZE 256

#define ALPAQA_LOG_FILE "/var/log/alpaqa/alpaqa_log.txt"
#define I2C_DEVICE_FILENAME "/dev/i2c-1"

bool alpaqaRunning;

static void signalHandler(int signalNumber);
static void writeBanners();
static void writePM(const PARTICULATE_MATTER_DATA * pm_data, uint16_t calculatedAqi, uint16_t instantAqi, bool aqiFull24Hour);
static void writeTempHumidity(const TEMP_HUMIDITY_DATA * tempHumidityData, float heatIndex);

#define cursorPosition(xLoc, yLoc) printf("\e[%d;%dH", xLoc, yLoc);
#define clearLine() printf("\e[2K");
#define setColor(foreground, background) printf("\e[%d;%dm", foreground, background);
#define resetColor() printf("\e[0m");
#define setBold() printf("\e[1m");

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
    char fileBuffer[BUFFER_SIZE];

    alpaqaRunning = true;

    printf(ESCAPE_CLEAR_SCREEN);
    writeBanners();

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
    cursorPosition(SYS_INFO_LOG_LINE,1);
    if(logFile == NULL)
    {
        printf("Log Status: Failed to open log file: %s! errno: %d\n", ALPAQA_LOG_FILE, errno);
    }
    else
    {
        printf("Log Status: Opened file: %s", ALPAQA_LOG_FILE);
    }

    // Attempt to open i2c device
    cursorPosition(SYS_INFO_I2C_LINE,1);
    if( (i2cFile = open(I2C_DEVICE_FILENAME, O_RDWR)) < 0)
    {
        printf("I2C Status: Failed to open the I2C Bus! errno: %d\n", errno);
    }
    else
    {
        printf("I2C Status: Connected");
    }

    initAlpaqaCalc();

    while(alpaqaRunning)
    {
        // Read data from AQI sensor
        cursorPosition(SYS_INFO_PM_LINE,1);
        clearLine();
        if(readAqiDataFromDevice(i2cFile) == true)
        {
            printf("Particulate Matter Sensor Status: Connected");
            getParticulateMatterData(&particulateData);
            storeAqiData(&particulateData);
            aqiFull24Hour = calcAQI(&calculatedAqi);
            instantAqi = calcInstantAQI(&particulateData);
        }
        else
        {
            printf("Particulate Matter Sensor Status: Disconnected");
        }

        // Read data from Temperature and Humidity sensor
        cursorPosition(SYS_INFO_TEMPERATURE_LINE,1);
        clearLine();
        if(readTempAndHumidityFromDevice(i2cFile) == true)
        {
            printf("Temperature and Humidity Sensor Status: Connected");
            getTempAndHumidityData(&tempHumidityData);
            heatIndex = calcHeatIndex(&tempHumidityData);
        }
        else
        {
            printf("Temperature and Humidity Sensor Status: Disconnected");
        }

        writePM(&particulateData, calculatedAqi, instantAqi, aqiFull24Hour);
        
        writeTempHumidity(&tempHumidityData, heatIndex);

        if(logFile != NULL)
        {
            size_t bufferStringSize;
            long fileSize;

            memset(fileBuffer, 0, sizeof(fileBuffer));

            // PM 1.0, PM 2.5, PM 10.0, AQI, AQI avg, Temp F, Temp C, Humidity, Heat Index
            snprintf(fileBuffer, sizeof(fileBuffer), "%d, %d, %d, %d, %d, %0.2f, %0.2f, %0.2f, %0.2f\n",
                    particulateData.pm1_0, particulateData.pm2_5, particulateData.pm10_0,
                    instantAqi, calculatedAqi,
                    tempHumidityData.temperatureF, tempHumidityData.temperatureC,
                    tempHumidityData.humidity,
                    heatIndex);

            bufferStringSize = strlen(fileBuffer);

            fwrite(fileBuffer, bufferStringSize, sizeof(char), logFile);
            fflush(logFile);

            cursorPosition(SYS_INFO_LOG_SIZE_LINE,1);
            clearLine();

            fileSize = ftell(logFile);

            printf("Log File Size: %lu bytes\n", fileSize);
        }

        fflush(stdout);
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

static void writeBanners()
{
    setColor(GREEN_FG, BLACK_BG);
    cursorPosition(ALPAQA_BANNER_LINE, 1);
    printf("============================================================\n");
    printf(" ALPAQA - Automatic Low-cost Personal Air Quality Assistant \n");
    printf("===================A project by Matt Geib===================\n");
    printf("============================================================\n");

    setColor(CYAN_FG, BLACK_BG);
    cursorPosition(PM_BANNER_LINE,1);
    printf("============================================================\n");
    printf("===============Particulate Matter (ug / m^3)================\n");
    printf("============================================================\n");

    setColor(RED_FG, BLACK_BG);
    cursorPosition(SHT_BANNER_LINE, 1);
    printf("============================================================\n");
    printf("==================Temperature and Humidity==================\n");
    printf("============================================================\n");

    resetColor();
    cursorPosition(SYS_INFO_BANNER_LINE,1);
    printf("============================================================\n");
    printf("=====================System Information=====================\n");
    printf("============================================================\n");
}

static void writePM(const PARTICULATE_MATTER_DATA * pm_data, uint16_t calculatedAqi, uint16_t instantAqi, bool aqiFull24Hour)
{
    cursorPosition(PM_DATA_START_LINE,1);
    clearLine();
    setBold();

    printf("PM 1.0: ");
    setColor(WHITE_FG, CYAN_BG);
    printf("%d", pm_data->pm1_0);

    setColor(WHITE_FG, BLACK_BG);
    printf(" PM 2.5: ");
    setColor(WHITE_FG, CYAN_BG);
    printf("%d", pm_data->pm2_5);

    setColor(WHITE_FG, BLACK_BG);
    printf(" PM 10.0: ");
    setColor(WHITE_FG, CYAN_BG);
    printf("%d", pm_data->pm10_0);
    setColor(WHITE_FG, BLACK_BG);
    printf("\n");

    clearLine();
    setColor(WHITE_FG, BLACK_BG);
    printf("AQI now: ");
    setColor(WHITE_FG, CYAN_BG);
    printf("%d", instantAqi);
    setColor(WHITE_FG, BLACK_BG);
    printf("\n");

    clearLine();
    setColor(WHITE_FG, BLACK_BG);
    if(aqiFull24Hour)
    {
        printf("Calculated AQI (24 hour): ");
    }
    else
    {
        printf("Calculated AQI (Running Average): ");
    }
    setColor(WHITE_FG, CYAN_BG);
    printf("%d", calculatedAqi);
    setColor(WHITE_FG, BLACK_BG);
    printf("\n");

    resetColor();
}

static void writeTempHumidity(const TEMP_HUMIDITY_DATA * tempHumidityData, float heatIndex)
{
    cursorPosition(SHT_DATA_START_LINE,1);
    clearLine();
    setBold();

    printf("Temperature: ");
    setColor(WHITE_FG, RED_BG);
    printf("%0.2f F", tempHumidityData->temperatureF);
    setColor(WHITE_FG, BLACK_BG);
    printf(" / ");
    setColor(WHITE_FG, RED_BG);
    printf("%0.2f C", tempHumidityData->temperatureC);
    setColor(WHITE_FG, BLACK_BG);
    printf("\n");

    clearLine();
    setColor(WHITE_FG, BLACK_BG);
    printf("Humidity: ");
    setColor(WHITE_FG, RED_BG);
    printf("%0.2f", tempHumidityData->humidity);
    setColor(WHITE_FG, BLACK_BG);
    printf("\n");

    clearLine();
    setColor(WHITE_FG, BLACK_BG);
    printf("Heat Index: ");
    setColor(WHITE_FG, RED_BG);
    printf("%0.2f", heatIndex);
    setColor(WHITE_FG, BLACK_BG);
    printf("\n");

    resetColor();
}