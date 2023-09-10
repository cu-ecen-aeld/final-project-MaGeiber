#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#define ESCAPE_CLEAR_SCREEN "\e[2J"
#define ESCAPE_CURSOR_PREVIOUS "\e[7F"
#define ESCAPE_CLEAR_LINE "\e[2K"

#define ALPAQA_LOG_FILE "/var/log/alpaqa/alpaqa_log.txt"

bool alpaqaRunning;

static void signal_handler(int signal_number);

int main()
{
    FILE * logFile;
    struct sigaction sig_action;

    alpaqaRunning = true;

    printf(ESCAPE_CLEAR_SCREEN);
    printf("========================\n");
    printf("A.L.P.A.Q.A. Data Logger\n");
    printf("========================\n");

    // register the signal handler to allow graceful quitting
    memset(&sig_action, 0, sizeof(struct sigaction));
    sig_action.sa_handler = signal_handler;
    if( sigaction(SIGTERM, &sig_action, NULL) != 0)
    {
    }
    if( sigaction(SIGINT, &sig_action, NULL) != 0)
    {
    }

    logFile = fopen(ALPAQA_LOG_FILE, "a+");
    if(logFile == NULL)
    {
        printf("Failed to open log file: %s ! errno: %d\n", ALPAQA_LOG_FILE, errno);
    }

    while(alpaqaRunning)
    {
        printf("Air Quality: Particulate Matter:\n");
        printf("PM 1.0: %d PM 2.5: %d PM 10.0: %d\n", 0, 0, 0);
        printf("Calculated AQI (Extrapolated): %d\n", 0);
        printf("Calculated AQI (24 hour): %d\n", 0);
        printf("Temperature: %d F / %d C\n", 0, 0);
        printf("Humidity: %d\n", 0);
        printf("Heat Index: %d\n", 0);
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

static void signal_handler(int signal_number)
{
    int errno_saved = errno;
    switch(signal_number)
    {
        case SIGINT:
        case SIGTERM:
            alpaqaRunning = false;
            break;

        default:
            break;
    }
    errno = errno_saved;
}