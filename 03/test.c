#include "common.h"
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>

#define TIME_LIMIT_SEC 0 //��sΪ��λ
#define TIME_LIMIT_USEC 1000//��΢��Ϊ��λ

void test_func()
{
    static count=0;
    printf("���ѽ_%d\n",++count);
}

//������K֡�Ķ�ʱ��
void start_timer()
{
    struct itimerval val;
    val.it_value.tv_sec = TIME_LIMIT_SEC;
    val.it_value.tv_usec=TIME_LIMIT_USEC;
    val.it_interval.tv_sec=0;
    val.it_interval.tv_usec=0;
    setitimer(ITIMER_REAL,&val,NULL);
}

//ֹͣ��K֡�Ķ�ʱ��
void stop_timer()
{
    
}



int main()
{
    signal(SIGALRM,test_func);
    start_timer();
    while(1);
    
}