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
    //初始化共享内存
    memset(addr, '\0', MEM_SIZE);
    addr[MEM_FLAG_ADDR] = Can_Write_Not_Send;
    //addr[MEM_ACK_FLAG_ADDR] = Not_Send_ack;
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

        SDL_from_SNL(&s.info, fd); //从网络层读数据

        init_frame(&s);

        int i = 0;
        for (i = 0; i < MAX_PKT; i++)
            printf("%c", s.info.data[i]);
        printf("\n");
        fflush(stdout);

        //初始化，共享内存标志位为Can_Write & ~Send_Ack
        SDL_to_SPL(&s, addr, &cnt_sended_frames);
        //共享内存标志位为Can_Read & ~Send_Ack

        if (memcmp(CMPSTR, s.info.data, sizeof(char) * MAX_PKT) == 0)
        {
            shmdt(addr);
            exit(1);
        }

        //阻塞判断标志位是否改变
        //实现等待物理层收到ACK包后发送的信号
        //期望标志位为Can_Write & ~Send_Ack
        SDL_from_SPL(addr);
    }
}

int main()
{
    current_protocol = PROTOCOL3;
    sender1();
    return 0;
}