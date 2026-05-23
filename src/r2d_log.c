#include "r2d/r2d.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static R2D_LogLevel r2d_log_level = R2D_LOG_LEVEL_INFO;
static FILE *r2d_log_file = 0;
static char r2d_log_last_errors[R2D_LOG_SUBSYSTEM_COUNT][R2D_LOG_LAST_ERROR_SIZE];

static int R2D_LogTraceLevel(R2D_LogLevel level)
{
    switch (level) {
    case R2D_LOG_LEVEL_DEBUG:
        return LOG_DEBUG;
    case R2D_LOG_LEVEL_INFO:
        return LOG_INFO;
    case R2D_LOG_LEVEL_WARN:
        return LOG_WARNING;
    case R2D_LOG_LEVEL_ERROR:
        return LOG_ERROR;
    default:
        return LOG_TRACE;
    }
}

static void R2D_LogWriteFile(R2D_LogLevel level, R2D_LogSubsystem subsystem, const char *message)
{
    time_t now;
    struct tm local_time;
    bool has_time = false;

    if (r2d_log_file == 0) {
        return;
    }

    now = time(0);
#if defined(_WIN32)
    has_time = localtime_s(&local_time, &now) == 0;
#else
    {
        struct tm *time_info = localtime(&now);
        if (time_info != 0) {
            local_time = *time_info;
            has_time = true;
        }
    }
#endif

    if (has_time) {
        fprintf(
            r2d_log_file,
            "%04d-%02d-%02d %02d:%02d:%02d [%s] [%s] %s\n",
            local_time.tm_year + 1900,
            local_time.tm_mon + 1,
            local_time.tm_mday,
            local_time.tm_hour,
            local_time.tm_min,
            local_time.tm_sec,
            R2D_LogLevelName(level),
            R2D_LogSubsystemName(subsystem),
            message
        );
    } else {
        fprintf(r2d_log_file, "[%s] [%s] %s\n", R2D_LogLevelName(level), R2D_LogSubsystemName(subsystem), message);
    }

    fflush(r2d_log_file);
}

void R2D_LogSetLevel(R2D_LogLevel level)
{
    r2d_log_level = level;
}

R2D_LogLevel R2D_LogLevelCurrent(void)
{
    return r2d_log_level;
}

bool R2D_LogOpenFile(const char *path)
{
    FILE *file = 0;

    if (path == 0 || path[0] == '\0') {
        return false;
    }

#if defined(_MSC_VER)
    if (fopen_s(&file, path, "ab") != 0) {
        file = 0;
    }
#else
    file = fopen(path, "ab");
#endif

    if (file == 0) {
        return false;
    }

    R2D_LogCloseFile();
    r2d_log_file = file;
    return true;
}

void R2D_LogCloseFile(void)
{
    if (r2d_log_file != 0) {
        fclose(r2d_log_file);
        r2d_log_file = 0;
    }
}

void R2D_LogMessage(R2D_LogLevel level, R2D_LogSubsystem subsystem, const char *format, ...)
{
    char message[R2D_LOG_MESSAGE_SIZE];
    va_list args;

    if (format == 0 || level < r2d_log_level || level >= R2D_LOG_LEVEL_NONE) {
        return;
    }

    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    if (subsystem >= 0 && subsystem < R2D_LOG_SUBSYSTEM_COUNT && level >= R2D_LOG_LEVEL_ERROR) {
        snprintf(r2d_log_last_errors[subsystem], sizeof(r2d_log_last_errors[subsystem]), "%s", message);
    }

    TraceLog(R2D_LogTraceLevel(level), "R2D[%s]: %s", R2D_LogSubsystemName(subsystem), message);
    R2D_LogWriteFile(level, subsystem, message);
}

void R2D_LogInfo(R2D_LogSubsystem subsystem, const char *format, ...)
{
    char message[R2D_LOG_MESSAGE_SIZE];
    va_list args;

    va_start(args, format);
    vsnprintf(message, sizeof(message), format != 0 ? format : "", args);
    va_end(args);
    R2D_LogMessage(R2D_LOG_LEVEL_INFO, subsystem, "%s", message);
}

void R2D_LogWarn(R2D_LogSubsystem subsystem, const char *format, ...)
{
    char message[R2D_LOG_MESSAGE_SIZE];
    va_list args;

    va_start(args, format);
    vsnprintf(message, sizeof(message), format != 0 ? format : "", args);
    va_end(args);
    R2D_LogMessage(R2D_LOG_LEVEL_WARN, subsystem, "%s", message);
}

void R2D_LogError(R2D_LogSubsystem subsystem, const char *format, ...)
{
    char message[R2D_LOG_MESSAGE_SIZE];
    va_list args;

    va_start(args, format);
    vsnprintf(message, sizeof(message), format != 0 ? format : "", args);
    va_end(args);
    R2D_LogMessage(R2D_LOG_LEVEL_ERROR, subsystem, "%s", message);
}

const char *R2D_LogLastError(R2D_LogSubsystem subsystem)
{
    if (subsystem < 0 || subsystem >= R2D_LOG_SUBSYSTEM_COUNT) {
        return "";
    }

    return r2d_log_last_errors[subsystem];
}

const char *R2D_LogLevelName(R2D_LogLevel level)
{
    switch (level) {
    case R2D_LOG_LEVEL_DEBUG:
        return "debug";
    case R2D_LOG_LEVEL_INFO:
        return "info";
    case R2D_LOG_LEVEL_WARN:
        return "warn";
    case R2D_LOG_LEVEL_ERROR:
        return "error";
    case R2D_LOG_LEVEL_NONE:
        return "none";
    default:
        return "unknown";
    }
}

const char *R2D_LogSubsystemName(R2D_LogSubsystem subsystem)
{
    switch (subsystem) {
    case R2D_LOG_SUBSYSTEM_CORE:
        return "core";
    case R2D_LOG_SUBSYSTEM_ASSETS:
        return "assets";
    case R2D_LOG_SUBSYSTEM_AUDIO:
        return "audio";
    case R2D_LOG_SUBSYSTEM_INPUT:
        return "input";
    case R2D_LOG_SUBSYSTEM_RENDER:
        return "render";
    case R2D_LOG_SUBSYSTEM_SAVE:
        return "save";
    case R2D_LOG_SUBSYSTEM_TILEMAP:
        return "tilemap";
    case R2D_LOG_SUBSYSTEM_UI:
        return "ui";
    case R2D_LOG_SUBSYSTEM_GAME:
        return "game";
    case R2D_LOG_SUBSYSTEM_SCRIPT:
        return "script";
    default:
        return "unknown";
    }
}
