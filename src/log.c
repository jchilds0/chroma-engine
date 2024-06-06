/* 
 * Simple logger
 */

#include "log.h"
#include "chroma-engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

#define LOG_DIR             INSTALL_DIR "/log/"

void current_time(char *, int);
char *pad_int(int);

static char *filename;

void log_start(EngineType type) {
    FILE *pfile;

    switch (type) {
        case LogEngine:
            filename = LOG_DIR "chroma-engine.log";
            break;
        case LogPreview:
            filename = LOG_DIR "chroma-preview.log";
            break;
        default:
            filename = LOG_DIR "log.log";
    }

    pfile = fopen(filename, "w");
    fclose(pfile);

    switch (type) {
        case LogEngine:
            log_file(LogMessage, "Log", "Engine Started");
            break;
        case LogPreview:
            log_file(LogMessage, "Log", "Preview Started");
            break;
        default:
            log_file(LogMessage, "Log", "Logging to temp file");
    }
}

void log_file(LogType flag, char *module, char *buf, ...) {
    char time[100];
    char *type;
    char message[512];
    va_list argptr;
    va_start(argptr, buf);
    vsnprintf(message, sizeof message, buf, argptr);
    FILE *pfile;

    switch (flag) {
    case LogMessage:
        type = "";
        break;
    case LogWarn:
        type = "WARNING: ";
        break;
    case LogError:
        type = "ERROR: ";
        break;
    default:
        log_file(LogError, "Message [%s] had unknown flag %d", message, flag);
        type = "";
    }

    memset(time, '\0', sizeof time);
    current_time(time, sizeof time);

    pfile = fopen(filename, "a");

    fprintf(pfile, "%s [%s]\t%s%s\n", time, module, type, message);
    fclose(pfile);

    if (flag == LogError) {
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
            timeinfo->tm_mon + 1,
            timeinfo->tm_year - 100, 
            pad_int(timeinfo->tm_hour),
            timeinfo->tm_hour, 
            pad_int(timeinfo->tm_min),
            timeinfo->tm_min); 
}

char *pad_int(int num) {
    return num < 10 ? "0" : "\0";
}
