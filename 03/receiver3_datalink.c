/*���շ�������·��*/
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>

boolen file_continue = false;
//��������
void receiver1();

void file_end(int s)
{
    file_continue = true;
}

void receiver1()
{
    frame s;          /* ֡ */
    frame f_ack;      /*ȷ��֡*/
    event_type event; /* �¼����� */
    init_frame(&s);
    int file_id = 0;
    //����RDL_RPL�����ڴ�
    int shmid = GetShm(MEM_SIZE, RDL_RPL_KEYID);
    char *addr_RDL_RPL = shmat(shmid, NULL, 0);
    if (addr_RDL_RPL != NULL)
    {
        printf("�ɹ����ӹ����ڴ�\n");
    }
    MEM_SHMID[RDL_RPL_KEYID] = shmid;
    //memset(addr_RDL_RPL, '\0', MEM_SIZE);
    //addr_RDL_RPL[MEM_FLAG_ADDR] = Can_Write;

    //����RNL_RDL�����ڴ�
    shmid = GetShm(MEM_SIZE, RNL_RDL_KEYID);
    char *addr_RNL_RDL = shmat(shmid, NULL, 0);
    if (addr_RNL_RDL != NULL)
    {
        printf("�ɹ����ӹ����ڴ�\n");
    }
    //memset(addr_RNL_RDL, '\0', MEM_SIZE);
    //addr_RNL_RDL[MEM_FLAG_ADDR] = Can_Write;

    int cnt = 0;
    while (true)
    {
        //printf("%d\n", frame_arr);
        //wait_for_event(&event); /* �ȴ�ֱ����֡���� */
        printf("**********************\n");

        //��ʱ����Ҫ�����ڴ��־λΪCan_Read & ~Send_Ack
        RDL_from_RPL(&s, addr_RDL_RPL); /* ��������ȡ֡ */
        //��ʱ�������ڴ��־λ����ΪCan_Write & Send_Ack



        //sysUsecTime();
        int i = 0;
        for (i = 0; i < MAX_PKT; i++)
            printf("%c", s.info.data[i]);
        printf("\n");
        cnt++;
        printf("��%d֡\n", cnt);
        fflush(stdout);
        //printf("%c----%c\n", addr_RDL_RPL[MEM_FLAG_ADDR], addr_RDL_RPL[MEM_ACK_FLAG_ADDR]);
        /*�����ȫβ0�İ������ͷŹ����ڴ棬������������㴫��*/
        if (memcmp(CMPSTR, s.info.data, sizeof(char) * MAX_PKT) == 0)
        {
            //printf("%c----%c\n", addr_RDL_RPL[MEM_FLAG_ADDR], addr_RDL_RPL[MEM_ACK_FLAG_ADDR]);
            shmdt(addr_RDL_RPL);
            DestroyShm(MEM_SHMID[RDL_RPL_KEYID]);
            break;
        }
        //printf("%c----%c\n", addr_RDL_RPL[MEM_FLAG_ADDR], addr_RDL_RPL[MEM_ACK_FLAG_ADDR]);
        RDL_to_RNL(&s.info, &file_id); /* ���ݰ�������� */
                                       //sysUsecTime();
        printf("���ݰ��������\n");

        /*�Ƿ�ʹ��ͬһ�鹲���ڴ��أ��Ҿ��ÿ��ԣ���һ��*/

        //printf("%c----%c\n", addr_RDL_RPL[MEM_FLAG_ADDR], addr_RDL_RPL[MEM_ACK_FLAG_ADDR]);
        fflush(stdout);
        //��ʱ����Ҫ�����ڴ��־λΪCan_Write & Send_Ack
        init_f_ack(&f_ack);
        RDL_to_RPL(&f_ack, addr_RDL_RPL); /*������㴫��ȷ��֡����ʾ�յ���֡��������һ֡*/
        //��ʱ�������ڴ��־λ����ΪCan_Read & Send_Ack
        printf("�ѷ��������ACK\n");
        fflush(stdout);
    }
}

int main()
{
    current_protocol = PROTOCOL3;
    receiver1();
    return 0;
}