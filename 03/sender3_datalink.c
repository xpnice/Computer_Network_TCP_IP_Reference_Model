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
    //��ʼ�������ڴ�
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

        SDL_from_SNL(&s.info, fd); //������������

        init_frame(&s);

        int i = 0;
        for (i = 0; i < MAX_PKT; i++)
            printf("%c", s.info.data[i]);
        printf("\n");
        fflush(stdout);

        //��ʼ���������ڴ��־λΪCan_Write & ~Send_Ack
        SDL_to_SPL(&s, addr, &cnt_sended_frames);
        //�����ڴ��־λΪCan_Read & ~Send_Ack

        if (memcmp(CMPSTR, s.info.data, sizeof(char) * MAX_PKT) == 0)
        {
            shmdt(addr);
            exit(1);
        }

        //�����жϱ�־λ�Ƿ�ı�
        //ʵ�ֵȴ�������յ�ACK�����͵��ź�
        //������־λΪCan_Write & ~Send_Ack
        SDL_from_SPL(addr);
    }
}

int main()
{
    current_protocol = PROTOCOL3;
    sender1();
    return 0;
}