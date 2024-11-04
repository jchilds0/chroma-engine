/*
 *
 */

#ifndef CHROMA_LOG
#define CHROMA_LOG

typedef enum {
    LogMessage,
    LogWarn,
    LogError,
} LogType;

typedef enum {
    LogEngine,
    LogPreview
} EngineType;

/* log.c */
void log_start(char *log_path);
void log_assert(int cond, const char *, const char *);
void log_file(LogType, const char *, const char *, ...);

#endif // !CHROMA_LOG
