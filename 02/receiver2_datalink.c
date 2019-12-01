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

    int cnt = 0;
    while (true)
    {
        printf("**********************\n");
        //��ʱ����Ҫ�����ڴ��־λΪCan_Read & ~Send_Ack
        RDL_from_RPL(&s, addr_RDL_RPL); /* ��������ȡ֡ */

        //��ʱ�������ڴ��־λ����ΪCan_Write & Send_Ack

        RDL_to_RNL(&s.info); /* ���ݰ�������� */
        cnt++;
        printf("���յ���%d֡\n",cnt);
        /*�����ȫβ0�İ������ͷŹ����ڴ�*/
        if (memcmp(CMPSTR, s.info.data, sizeof(char) * MAX_PKT) == 0)
        {
            shmdt(addr_RDL_RPL);
            DestroyShm(MEM_SHMID[RDL_RPL_KEYID]);
            break;
        }

        //��ʱ����Ҫ�����ڴ��־λΪCan_Write & Send_Ack
        init_f_ack(&f_ack);
        RDL_to_RPL(&f_ack, addr_RDL_RPL); /*������㴫��ȷ��֡����ʾ�յ���֡��������һ֡*/
        //��ʱ�������ڴ��־λ����ΪCan_Read & Send_Ack
    }
}

int main()
{
    current_protocol = PROTOCOL2;
    receiver1();
    return 0;
}