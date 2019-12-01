#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>

void sysLocalTime()
{
    time_t timesec;
    struct tm *p;

    time(&timesec);
    p = localtime(&timesec);

    printf("%d%d%d%d%d%d\n", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
}

void sysUsecTime()
{
    struct timeval tv;
    struct timezone tz;

    struct tm *p;

    gettimeofday(&tv, &tz);
    p = localtime(&tv.tv_sec);
    printf("time_now:%d-%d-%d-%d-%d-%d.%ld\n", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, tv.tv_usec);
}

int main(void)
{
    char s[100];
    memcpy(s, "1", 100);
    int i;
    for (i = 0; i < 100; i++)
        printf("%c", s[i]);

    return 0;
}