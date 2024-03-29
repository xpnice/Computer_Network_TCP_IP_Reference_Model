/*接收方物理层*/
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
//由sender向recevier发起连接

#define ERROR -1
#define SUCCESS 1

int client_socket_desc;

int main(int argc, char *argv[])
{
    current_protocol = PROTOCOL2;
    //共享内存
    int shmid = GetShm(MEM_SIZE, RDL_RPL_KEYID);
    char *addr = shmat(shmid, NULL, 0);
    if (addr != NULL)
    {
        printf("成功链接共享内存\n");
    }
    memset(addr, '\0', MEM_SIZE);
    addr[MEM_FLAG_ADDR] = Can_Write_Not_Send;

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

    //输出配置信息
    printf("------------------------------\n");
    printf("Connected！\n");
    printf("Sender IP:%s\n", inet_ntoa(client.sin_addr));
    printf("Sender Port:%d\n", ntohs(client.sin_port));
    printf("------------------------------\n");

    fflush(stdout);
    //从sender接收消息
    frame s; /* 帧 */
    frame f_ack;
    int cnt = 0;
    while (1)
    {
        printf("**********************\n");
        cnt++;
        printf("接收到第%d帧\n", cnt);
        RPL_from_SPL(&s, client_socket_desc); /*从发送方物理层接收包 */
        //此时，需要共享内存标志位为Can_Write & ~Send_Ack
        RPL_to_RDL(&s, addr); /* 向数据链路层发送帧 */
        //此时，共享内存标志位被置为Can_Read & ~Send_Ack
        //此时，需要共享内存标志位为Can_Read & Send_Ack
        RPL_from_RDL(&f_ack, addr); /* 从数据链路层接收回复帧 */
                                    //此时，共享内存标志位被置为Can_Read & ~Send_Ack
                                    //共享内存标志位为Can_Write & ~Send_Ack

        //如果是数据帧，传1036个字节，如果是控制帧，传12个字节
        if (fit_percentage(ACK_DELAY))
        {
            printf("<!---延时%d微秒发送ACK---!>\n", DELAY_US);
            fflush(stdout);
            usleep(DELAY_US);
        }
        
        RPL_to_SPL(f_ack, client_socket_desc); /* 向物理层发送回复帧 */
    }

    return 0;
}

/*
    还没有将共享内存释放，进程退出顺序有问题
*/
