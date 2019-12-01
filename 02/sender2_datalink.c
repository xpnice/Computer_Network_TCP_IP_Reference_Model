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
    int cnt_sended_frames = 0;
    //���������ڴ�
    int shmid = GetShm(MEM_SIZE, SDL_SPL_KEYID);
    char *addr = shmat(shmid, NULL, 0);
    if (addr == NULL)
    {
        printf("�����ڴ�����ʧ��\n");
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
    int cnt = 0;
    while (true)
    {
        printf("**********************\n");
        SDL_from_SNL(&s.info, fd); //������������
        init_frame(&s);
        //��ʼ���������ڴ��־λΪCan_Write & ~Send_Ack
        SDL_to_SPL(&s, addr, &cnt_sended_frames);
        //�����ڴ��־λΪCan_Read & ~Send_Ack
        cnt++;
        printf("���յ���%d֡\n", cnt);
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
    current_protocol = PROTOCOL2;
    sender1();
    return 0;
}