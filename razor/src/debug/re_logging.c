#include <razor.h>

#include <time.h>
#include <stdio.h>
#include <stdarg.h>

static FILE* __re_log_file__ = NULL;

// *=================================================
// *
// * __re_getTimeString
// *
// *=================================================

void __re_getTimeString(char* buffer) {
    time_t now = time(NULL);
    struct tm local_time;
    
    #if RE_PLATFORM == RE_PLATFORM_WINDOWS
    localtime_s(&local_time, &now);
    #else
    localtime_r(&now, &local_time);
    #endif

    strftime(buffer, __RE_LOG_TIME_BUFFER_SIZE__, "%Y-%m-%d %H:%M:%S", &local_time);
}

// *=================================================
// *
// * __re_internalLog
// *
// *=================================================

void __re_internalLog(const char* format, ...) {
    va_list args;
    va_start(args, format);

    if (__re_log_file__ == NULL) {
        #if RE_PLATFORM == RE_PLATFORM_WINDOWS
        vprintf_s(format, args);
        #else
        vprintf(format, args);
        #endif
    }
    else {
        #if RE_PLATFORM == RE_PLATFORM_WINDOWS
        vfprintf_s(__re_log_file__, format, args);
        #else
        vfprintf(__re_log_file__, format, args);
        #endif

        fflush(__re_log_file__);
    }

    va_end(args);
}

// *=================================================
// *
// * re_closeLogFile
// *
// *=================================================

void re_closeLogFile() {
    if (__re_log_file__ != NULL) {
        fclose(__re_log_file__);
        __re_log_file__ = NULL;
    }
}

// *=================================================
// *
// * re_setLogFile
// *
// *=================================================

void re_setLogFile(const char* file_path) {
    re_closeLogFile();

    #if RE_PLATFORM == RE_PLATFORM_WINDOWS
    fopen_s(&__re_log_file__, file_path, "w");
    #else
    fopen(file_path, "w");
    #endif

    if (__re_log_file__ == NULL) {
        re_logError("Failed to open logging file: %s", file_path);
    }
}