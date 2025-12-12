#include <re_debug.h>

#include <time.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef RE_LOGGER_ENABLED
#define __RE_LOG_TIME_BUFFER_SIZE 20u

static FILE* __re_log_file__ = NULL;
static char __time_buffer[__RE_LOG_TIME_BUFFER_SIZE];
#endif

// *=================================================
// *
// * __re_internalLog
// *
// *=================================================

void __re_internalLog(const char* format, ...) {
    #ifdef RE_LOGGER_ENABLED
    time_t now = time(NULL);
    struct tm local_time;
    
    #if RE_PLATFORM == RE_PLATFORM_WINDOWS
    localtime_s(&local_time, &now);
    #else
    localtime_r(&now, &local_time);
    #endif

    strftime(__time_buffer, __RE_LOG_TIME_BUFFER_SIZE, "%Y-%m-%d %H:%M:%S", &local_time);

    va_list args;
    va_start(args, format);

    if (__re_log_file__ == NULL) {
        #if RE_PLATFORM == RE_PLATFORM_WINDOWS
        printf_s("%s - ", __time_buffer);
        vprintf_s(format, args);
        #else
        printf("%s - ", __time_buffer);
        vprintf(format, args);
        #endif
    }
    else {
        #if RE_PLATFORM == RE_PLATFORM_WINDOWS
        fprintf_s(__re_log_file__, "%s - ", __time_buffer);
        vfprintf_s(__re_log_file__, format, args);
        #else
        fprintf(__re_log_file__, "%s - ", __time_buffer);
        vfprintf(__re_log_file__, format, args);
        #endif

        fflush(__re_log_file__);
    }

    va_end(args);
    #endif
}

// *=================================================
// *
// * re_closeLogFile
// *
// *=================================================

void re_closeLogFile() {
    #ifdef RE_LOGGER_ENABLED
    if (__re_log_file__ != NULL) {
        fclose(__re_log_file__);
        __re_log_file__ = NULL;
    }
    #endif
}

// *=================================================
// *
// * re_setLogFile
// *
// *=================================================

void re_setLogFile(const char* file_path) {
    #ifdef RE_LOGGER_ENABLED
    re_closeLogFile();

    #if RE_PLATFORM == RE_PLATFORM_WINDOWS
    fopen_s(&__re_log_file__, file_path, "w");
    #else
    fopen(file_path, "w");
    #endif

    if (__re_log_file__ == NULL) {
        re_logError("Failed to open logging file: %s", file_path);
    }
    #endif
}