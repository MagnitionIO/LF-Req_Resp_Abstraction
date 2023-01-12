#ifndef __UTILS__
#define __UTILS__

#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <stdint.h>
#include <err.h>
#include <getopt.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#ifdef __linux__
#include <sys/prctl.h>
#endif
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <poll.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

static int log_level = LOG_TRACE;

static const char *level_strings[] = {
  "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

#define log_trace(...) logging(LOG_TRACE, __FILENAME__, __LINE__, __VA_ARGS__)
#define log_debug(...) logging(LOG_DEBUG, __FILENAME__, __LINE__, __VA_ARGS__)
#define log_info(...)  logging(LOG_INFO,  __FILENAME__, __LINE__, __VA_ARGS__)
#define log_warn(...)  logging(LOG_WARN,  __FILENAME__, __LINE__, __VA_ARGS__)
#define log_error(...) logging(LOG_ERROR, __FILENAME__, __LINE__, __VA_ARGS__)
#define log_fatal(...) logging(LOG_FATAL, __FILENAME__, __LINE__, __VA_ARGS__)

static inline void logging(int level, const char *file, int line, const char *fmt, ...)
{
    if (level >= log_level)
    {
        va_list ap;
        struct timespec l_time;
        clock_gettime(CLOCK_REALTIME, &l_time);
        va_start(ap, fmt);
        
        time_t seconds;
        char timestring[32];
        char timebuffer[32] = { 0 };
        char nanosec[10];

        seconds = l_time.tv_sec;
        snprintf(nanosec, 10 , "%09ld", l_time.tv_nsec);
        strftime(timebuffer, sizeof (timebuffer), "%Y-%m-%d %H:%M:%S", localtime(&seconds));
        snprintf(timestring, sizeof (timestring), "%s,%.3s", timebuffer , nanosec);
        
        fprintf(
            stderr, "%s %d %d %-5s %s:%d: ",
            timestring, getpid(), getppid(), level_strings[level], file, line);

        vfprintf(stderr, fmt, ap);
        fprintf(stderr, "\n");
        fflush(stderr);
        
        va_end(ap);
    }
}

#endif
