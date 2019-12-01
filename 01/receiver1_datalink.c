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

    int cnt = 0;
    while (true)
    {
        printf("**********************\n");
        RDL_from_RPL(&s, addr_RDL_RPL); /* ��������ȡ֡ */
        RDL_to_RNL(&s.info);            /* ���ݰ�������� */
        cnt++;
        printf("���յ���%d֡\n", cnt);
        if (memcmp(CMPSTR, s.info.data, sizeof(char) * MAX_PKT) == 0)
        {
            shmdt(addr_RDL_RPL);
            DestroyShm(MEM_SHMID[RDL_RPL_KEYID]);
            break;
        }
    }
}

int main()
{
    current_protocol = PROTOCOL1;
    receiver1();
    return 0;
}