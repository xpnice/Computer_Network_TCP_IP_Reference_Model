/*发送端数据链路层*/
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
    //创建共享内存
    int shmid = GetShm(MEM_SIZE, SDL_SPL_KEYID);
    char *addr = shmat(shmid, NULL, 0);
    if (addr != NULL)
    {
        printf("成功链接共享内存\n");
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
        printf("**********************\n");
        struct stat statbuff;
        if (stat(file_name, &statbuff) < 0)
        {
            perror("stat");
        }
        if (statbuff.st_size < MAX_PKT)
            continue;
        SDL_from_SNL(&s.info, fd); //从网络层读数据
        sysUsecTime();
        init_frame(&s);

        int i = 0;
        for (i = 0; i < MAX_PKT; i++)
            printf("%c", s.info.data[i]);
        printf("\n");

        SDL_to_SPL(&s, addr, &cnt_sended_frames);
        //sysUsecTime();
        //cnt++;
        // if (cnt_sended_frames == 1024)
        // {
        //     printf("发%d\n", cnt_sended_frames);
        //     fflush(stdout);
        // }

        // if (file_id == MAX_SHARE_FILES)
        // {
        //     file_id = 0;
        //     int SNL_PID = get_pid("sender1_network");
        //     if (SNL_PID != 0)
        //         kill(SNL_PID, SHARE_FILE_END);
        // }

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

    sender1();
    return 0;
}