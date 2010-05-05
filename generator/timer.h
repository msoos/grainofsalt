#ifndef TIMER__H
#define TIMER__H

#ifdef _MSC_VER
#include <ctime>

static inline double cpuTime(void)
{
    return (double)clock() / CLOCKS_PER_SEC;
}
#else

#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

static inline double cpuTime(void)
{
    struct rusage ru;
    getrusage(RUSAGE_SELF, &ru);
    return (double)ru.ru_utime.tv_sec + (double)ru.ru_utime.tv_usec / 1000000;
}
#endif

#endif //TIMER__H
