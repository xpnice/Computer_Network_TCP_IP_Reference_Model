/*���ͷ������*/
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <time.h>
#include <sys/timeb.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/stat.h>
#define ONCE 12

int client_socket_desc;

int main(int argc, char *argv[])
{
    srand((unsigned)time(NULL)); 
    //���� socket
    client_socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket_desc == -1)
    {
        perror("could not create socket");
        return 1;
    }

    //���� socket to a server
    struct sockaddr_in client;
    memset(&client, 0, sizeof(client)); //���
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = inet_addr(RECEIVER_IP);
    client.sin_port = htons(atoi(RECEIVER_PORT));

    //Connect to remote server
    while (1)
    {
        if (connect(client_socket_desc, (struct sockaddr *)&client, sizeof(client)) == 0)
        {
            //���������Ϣ
            printf("------------------------------\n");
            printf("Connected��\n");
            printf("Receiver IP:%s\n", inet_ntoa(client.sin_addr));
            printf("Receiver Port:%d\n", ntohs(client.sin_port));
            printf("------------------------------\n");
            break;
        }
    }
    fflush(stdout);
    //������������·��ȡ������
    //��Recevier�˷�����Ϣ

    frame s; /* ֡ */
    frame f_ack;
    int count = 0;
    while (1)
    {
        printf("**********************\n");
        SPL_from_SDL(&s); /*��������·���ȡ�� */
        int i = 0;
        for (i = 0; i < MAX_PKT; i++)
            printf("%c", s.info.data[i]);

        SPL_to_RPL(s, client_socket_desc); /* ͨ������㷢��֡ */
        count++;
        printf("��%d֡���ͳɹ�\n", count);

        SPL_from_RPL(&f_ack, client_socket_desc); /* �ӽ��շ��������ջظ�֡ 
        �жϽ����Ƿ�Ϊ�ظ�֡���ǵĻ��������źŸ����ͷ�������·��*/
    }

    return 0;
}