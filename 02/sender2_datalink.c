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
    int cnt_sended_frames = 0;
    //创建共享内存
    int shmid = GetShm(MEM_SIZE, SDL_SPL_KEYID);
    char *addr = shmat(shmid, NULL, 0);
    if (addr == NULL)
    {
        printf("共享内存链接失败\n");
    }
    //初始化共享内存
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
    int cnt = 0;
    while (true)
    {
        printf("**********************\n");
        SDL_from_SNL(&s.info, fd); //从网络层读数据
        init_frame(&s);
        //初始化，共享内存标志位为Can_Write & ~Send_Ack
        SDL_to_SPL(&s, addr, &cnt_sended_frames);
        //共享内存标志位为Can_Read & ~Send_Ack
        cnt++;
        printf("接收到第%d帧\n", cnt);
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
    current_protocol = PROTOCOL2;
    sender1();
    return 0;
}