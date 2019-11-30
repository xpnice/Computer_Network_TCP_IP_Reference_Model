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

    // //����RNL_RDL�����ڴ�
    // shmid = GetShm(MEM_SIZE, RNL_RDL_KEYID);
    // char *addr_RNL_RDL = shmat(shmid, NULL, 0);
    // if (addr_RNL_RDL != NULL)
    // {
    //     printf("�ɹ����ӹ����ڴ�\n");
    // }
    //memset(addr_RNL_RDL, '\0', MEM_SIZE);
    //addr_RNL_RDL[MEM_FLAG_ADDR] = Can_Write;

    int cnt = 0;
    while (true)
    {
        //printf("%d\n", frame_arr);
        //wait_for_event(&event); /* �ȴ�ֱ����֡���� */
        // printf("\n**********************\n");

        RDL_from_RPL(&s, addr_RDL_RPL); /* ��������ȡ֡ */
        //sysUsecTime();
        // int i = 0;
        // for (i = 0; i < MAX_PKT; i++)
        //     printf("%c", s.info.data[i]);

        if (memcmp(CMPSTR, s.info.data, sizeof(char) * MAX_PKT) == 0)
        {
            shmdt(addr_RDL_RPL);
            DestroyShm(MEM_SHMID[RDL_RPL_KEYID]);
            //sysUsecTime();
            break;
        }

        RDL_to_RNL(&s.info, &file_id); /* ���ݰ�������� */
        //sysUsecTime();
    }
}

int main()
{
    current_protocol = PROTOCOL1;
    receiver1();
    return 0;
}