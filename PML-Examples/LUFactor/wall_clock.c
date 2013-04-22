#include <sys/time.h>

double wall_clock()
{
    struct timeval tp;
    struct timezone tzp;
    double t;

    gettimeofday(&tp, &tzp);
    t = (tzp.tz_minuteswest * 60 + tp.tv_sec) * 1.0e6 + (tp.tv_usec) * 1.0;
    return t;
}
