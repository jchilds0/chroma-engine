/* 
 * Simple logger
 */

#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <stdarg.h>

void current_time(char *, int);
char *pad_int(int);

static char filename[1024];

void log_start(char *path) {
    FILE *pfile;

    snprintf(filename, sizeof filename, "%s/chroma_engine.log", path);
    pfile = fopen(filename, "w");
    fclose(pfile);
    log_file(LogMessage, "Log", "Engine Started");
}

void log_assert(int cond, const char *module, const char *buf) {
    if (cond) {
        return;
    }

    log_file(LogError, module, buf);
    exit(1);
}

void log_file(LogType flag, const char *module, const char *buf, ...) {
    char time[100];
    char *type;
    char message[2048];
    va_list argptr = {0};
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
}

void current_time(char *buf, int buf_size) {
    time_t raw_time;
    struct tm *timeinfo;

    time(&raw_time);
    timeinfo = localtime(&raw_time);

    sprintf(buf, "[%s%d/%s%d/%d %s%d:%s%d]", 
            pad_int(timeinfo->tm_mday),
            timeinfo->tm_mday,
            pad_int(timeinfo->tm_mon + 1),
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
