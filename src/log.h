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
void log_start(EngineType);
void log_file(LogType, char *, char *, ...);

#endif // !CHROMA_LOG
