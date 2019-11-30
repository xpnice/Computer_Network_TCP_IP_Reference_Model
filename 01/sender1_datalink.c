/*���Ͷ�������·��*/
#include "common.h"
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>

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

    memset(addr, '\0', MEM_SIZE);
    addr[MEM_FLAG_ADDR] = Can_Write;
    fflush(stdout);

    int cnt = 0;

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

    while (true)
    {
        // printf("**********************\n");
        SDL_from_SNL(&s.info, fd); //������������
        // sysUsecTime();
        init_frame(&s);

        // int i = 0;
        // for (i = 0; i < MAX_PKT; i++)
        //     printf("%c", s.info.data[i]);
        // printf("\n");

        SDL_to_SPL(&s, addr, &cnt_sended_frames);
        //sysUsecTime();

        if (memcmp(CMPSTR, s.info.data, sizeof(char) * MAX_PKT) == 0)
        {
            //DestroyShm(MEM_SHMID[SDL_SPL_KEYID]);
            sysUsecTime();
            shmdt(addr);
            exit(1);
        }
    }
}

int main()
{
    current_protocol = PROTOCOL1;
    sender1();
    return 0;
}