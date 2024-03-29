/*发送方物理层*/
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
#include <sys/ipc.h>
#include <sys/shm.h>
#define ONCE 12

int client_socket_desc;

int main(int argc, char *argv[])
{
    current_protocol = PROTOCOL2;

    //建立 socket
    client_socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket_desc == -1)
    {
        perror("could not create socket");
        return 1;
    }

    //连接 socket to a server
    struct sockaddr_in client;
    memset(&client, 0, sizeof(client)); //清空
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = inet_addr(RECEIVER_IP);
    client.sin_port = htons(atoi(RECEIVER_PORT));

    //Connect to remote server
    while (1)
    {
        if (connect(client_socket_desc, (struct sockaddr *)&client, sizeof(client)) == 0)
        {
            //输出配置信息
            printf("------------------------------\n");
            printf("Connected！\n");
            printf("Receiver IP:%s\n", inet_ntoa(client.sin_addr));
            printf("Receiver Port:%d\n", ntohs(client.sin_port));
            printf("------------------------------\n");
            break;
        }
    }
    fflush(stdout);
    //物理层从数据链路层取得数据
    //向Recevier端发送消息

    //创建共享内存-----------------------------------------------
    int shmid = GetShm(MEM_SIZE, SDL_SPL_KEYID);
    char *addr = shmat(shmid, NULL, 0);
    if (addr != NULL)
    {
        printf("成功链接共享内存\n");
    }
    // memset(addr, '\0', MEM_SIZE);
    // addr[MEM_FLAG_ADDR] = Can_Write;

    MEM_SHMID[SDL_SPL_KEYID] = shmid;
    //------------------------------------------------------------

    frame s;     /* 帧 */
    frame f_ack; /*回复帧*/
    seq_nr seq_want = 0;
    int count = 0;
    while (1)
    {

        //printf("**********************\n");
        SPL_from_SDL(&s, addr); /*从数据链路层获取包 */
                                // int i = 0;
                                // for (i = 0; i < MAX_PKT; i++)
                                //     printf("%c", s.info.data[i]);
        //此时，共享内存标志位为Can_Write & Send_Ack

        SPL_to_RPL(s, client_socket_desc); /* 通过物理层发送帧 */

        count++;
        printf("第%d帧发送成功\n", count);
        fflush(stdout);

        if (memcmp(CMPSTR, s.info.data, sizeof(char) * MAX_PKT) == 0)
        {
            shmdt(addr);
            DestroyShm(MEM_SHMID[SDL_SPL_KEYID]);
            exit(1);
        }
        seq_want = s.seq;
        //非阻塞接收
        SPL_from_RPL1(&f_ack, client_socket_desc, addr); /* 从接收方物理层接收回复帧 
        判断接收是否为回复帧，是的话，发送信号给发送方数据链路层*/
        if (addr[MEM_FLAG_ADDR] != Can_Write_Send)
            continue;

        if (fit_percentage(SPL_ERRO_PERCENTAGE))
        {
            printf("<!---SPL向上发送CKSUM_ERR---!>\n");
            addr[MEM_FLAG_ADDR] = CKSUM_ERROR;
            fflush(stdout);
            continue;
        }
        if (fit_percentage(SPL_LOST_PERCENTAGE))
        {
            printf("<!---SPL没收到，ACK丢了!!!---!>\n");
            fflush(stdout);
            continue;
        }

        //修改共享内存标志位
        //此时，共享内存标志位为Can_Write & Send_Ack
        if (f_ack.seq == seq_want)
        {
            SPL_to_SDL(&f_ack, addr);
            //此时，共享内存标志位为Can_Write & ~Send_Ack
    
        }
    }

    return 0;
}