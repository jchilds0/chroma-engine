/* 
 * Simple logger
 */

#include "chroma-engine.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#define LOG_DIR             "/home/josh/Documents/projects/chroma-engine/log/"

void current_time(char *, int);
char *pad_int(int);

void start_log(void) {
    FILE *pfile;

    pfile = fopen(LOG_DIR "chroma-engine.log", "w");
    fclose(pfile);

    log_to_file(LOG_MESSAGE, "Engine Started");
}


void log_to_file(int flag, char *buf, ...) {
    char time[100];
    char *type;
    char message[512];
    va_list argptr;
    va_start(argptr, buf);
    vsnprintf(message, sizeof message, buf, argptr);
    FILE *pfile;

    switch (flag) {
    case LOG_MESSAGE:
        type = "";
        break;
    case LOG_WARN:
        type = "WARNING: ";
        break;
    case LOG_ERROR:
        type = "ERROR: ";
        break;
    default:
        log_to_file(LOG_ERROR, "Message [%s] had unknown flag %d", message, flag);
        type = "";
    }

    memset(time, '\0', sizeof time);
    current_time(time, sizeof time);

    pfile = fopen(LOG_DIR "chroma-engine.log", "a");

    fprintf(pfile, "%s\t%s%s\n", time, type, message);
    fclose(pfile);

    if (flag == LOG_ERROR) {
        exit(0);
    }
}

void current_time(char *buf, int buf_size) {
    time_t raw_time;
    struct tm *timeinfo;

    time(&raw_time);
    timeinfo = localtime(&raw_time);

    sprintf(buf, "[%s%d/%s%d/%d %s%d:%s%d]", 
            pad_int(timeinfo->tm_mday),
            timeinfo->tm_mday,
            pad_int(timeinfo->tm_mon),
            timeinfo->tm_mon,
            timeinfo->tm_year - 100, 
            pad_int(timeinfo->tm_hour),
            timeinfo->tm_hour, 
            pad_int(timeinfo->tm_min),
            timeinfo->tm_min); 
}

char *pad_int(int num) {
    return num < 10 ? "0" : "\0";
}
