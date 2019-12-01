/*���Ͷ�������·��*/
#include "common.h"
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/signal.h>
#include <sys/time.h>
boolen timer_out;
seq_nr seq = 0;

void signal_timer(int s)
{
    timer_out = true;
}

struct itimerval val;
//������K֡�Ķ�ʱ��
void start_timer()
{
    val.it_value.tv_sec = TIME_LIMIT_SEC;
    val.it_value.tv_usec = TIME_LIMIT_USEC;
    val.it_interval.tv_sec = 0;
    val.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &val, NULL);
}

void stop_timer()
{
    val.it_value.tv_sec = 0;
    val.it_value.tv_usec = 0;
    val.it_interval.tv_sec = 0;
    val.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &val, NULL);
}
void sender1()
{

    frame s;
    packet buffer;
    int file_id = 0;
    int cnt_sended_frames = 0;
    //���������ڴ�
    int shmid = GetShm(MEM_SIZE, SDL_SPL_KEYID);
    char *addr = shmat(shmid, NULL, 0);

    if (addr != NULL)
    {
        printf("�ɹ����ӹ����ڴ�\n");
    }
    //��ʼ�������ڴ�
    memset(addr, '\0', MEM_SIZE);
    addr[MEM_FLAG_ADDR] = Can_Write_Not_Send;

    char file_name[30] = "s_file.txt";
    while (1)
    {
        if (access(file_name, 0) != -1)
            break;
    }
    int fd = open(file_name, O_RDONLY);
    if (fd < 0)
    {
        perror("open");
        exit(1);
    }

    struct itimerval val;
    int cnt = 0;
    while (true)
    {
        boolen can_go;
        start_timer();
        fflush(stdout);
        while (true)
        {
            if (addr[MEM_FLAG_ADDR] == CKSUM_ERROR || timer_out == true)
            {
                can_go = false;
                addr[MEM_FLAG_ADDR] = Can_Write_Not_Send;
                if (timer_out)
                    printf("<!---���ݳ�ʱ�����·���---!>\n");
                else
                    printf("<!---�յ�CKSUM_ERROR�����·���---!>\n");
                fflush(stdout);
                break;
            }
            else if (addr[MEM_FLAG_ADDR] == Can_Write_Not_Send)
            {
                can_go = true;
                break;
            }
        }
        fflush(stdout);
        stop_timer();
        timer_out = false;

        fflush(stdout);
        if (can_go)
        {
            cnt++;
            printf("��ȷ�����%d֡\n", cnt);
            SDL_from_SNL(&s.info, fd); //������������
            init_frame(&s);
            fflush(stdout);
            s.seq = seq; //֡ͷ���Ϊ0
            inc(seq);    //��ת֡ͷ�ź�
        }
        print_frame(s);
        fflush(stdout);
        SDL_to_SPL(&s, addr, &cnt_sended_frames);
        if (memcmp(CMPSTR, s.info.data, sizeof(char) * MAX_PKT) == 0)
        {
            shmdt(addr);
            exit(1);
        }
    }
}

int main()
{
    signal(SIGALRM, signal_timer);
    current_protocol = PROTOCOL2;
    sender1();
    return 0;
}