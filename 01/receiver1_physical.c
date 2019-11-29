/*���շ������*/
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
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
//��sender��recevier��������

#define ERROR -1
#define SUCCESS 1

int client_socket_desc;

int main(int argc, char *argv[])
{
    //�����ڴ�
    //���������ڴ�
    //���������ڴ�
    int shmid = GetShm(MEM_SIZE, RDL_RPL_KEYID);
    char *addr = shmat(shmid, NULL, 0);
    if (addr != NULL)
    {
        printf("�ɹ����ӹ����ڴ�\n");
    }
    memset(addr, '\0', MEM_SIZE);
    addr[MEM_FLAG_ADDR] = Can_Write;

    //create socket
    int socket_desc = create_bind();
    //listen
    listen(socket_desc, 5);
    //accept connection

    int c = sizeof(struct sockaddr_in);
    struct sockaddr_in client;
    memset(&client, 0, sizeof(client));
    client_socket_desc = accept(socket_desc, (struct sockaddr *)&client, &c);
    if (client_socket_desc < 0)
    {
        perror("accept failed");
        return ERROR;
    }

    //���������Ϣ
    printf("------------------------------\n");
    printf("Connected��\n");
    printf("Sender IP:%s\n", inet_ntoa(client.sin_addr));
    printf("Sender Port:%d\n", ntohs(client.sin_port));
    printf("------------------------------\n");

    fflush(stdout);
    //��sender������Ϣ
    frame s; /* ֡ */
    int count = 0;
    while (1)
    {
        printf("**********************\n");
        count++;
        printf("\n��%d֡\n", count);

        RPL_from_SPL(&s, client_socket_desc); /*�ӷ��ͷ��������հ� */
        sysUsecTime();
        // int i = 0;
        // for (i = 0; i < MAX_PKT; i++)
        //     printf("%c", s.info.data[i]);

        fflush(stdout);
        RPL_to_RDL(&s, addr); /* ��������·�㷢��֡ */
        sysUsecTime();
        fflush(stdout);
    }

    return 0;
}
