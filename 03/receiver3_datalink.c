/*���շ�������·��*/
#include "common.h"
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/signal.h>
#include <sys/time.h>

seq_nr seq_want = 0;
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

    int cnt = 0;
    while (true)
    {

        printf("**********************\n");

        RDL_from_RPL(&s, addr_RDL_RPL); /* ��������ȡ֡ */

        if (s.seq == seq_want)
        {
            RDL_to_RNL(&s.info); /* ���ݰ�������� */
            inc(seq_want);
            printf("���ݰ��������\n");
            cnt++;
            printf("��%d֡\n", cnt);
            fflush(stdout);
        }

        /*�����ȫβ0�İ������ͷŹ����ڴ�*/
        if (memcmp(CMPSTR, s.info.data, sizeof(char) * MAX_PKT) == 0)
        {
            shmdt(addr_RDL_RPL);
            DestroyShm(MEM_SHMID[RDL_RPL_KEYID]);
            break;
        }

        init_f_ack(&f_ack);
        f_ack.seq = s.seq;
        RDL_to_RPL(&f_ack, addr_RDL_RPL); /*������㴫��ȷ��֡����ʾ�յ���֡��������һ֡*/
        printf("�ѷ��������ACK\n");
        fflush(stdout);
    }
}

int main()
{
    current_protocol = PROTOCOL2;
    receiver1();
    return 0;
}