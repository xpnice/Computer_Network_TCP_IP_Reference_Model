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
//由sender向recevier发起连接

#define ERROR -1
#define SUCCESS 1

int client_socket_desc;

int main(int argc, char *argv[])
{
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
    while (1)
    {
        RPL_from_SPL(&s, client_socket_desc); /*从发送方物理层接收包 */
        int i = 0;
        for (i = 0; i < MAX_PKT; i++)
            printf("%c", s.info.data[i]);
        printf("\n**********************\n");
        //模拟丢了 读完就return
        //模拟错误 标志位置chenum error
        RPL_to_RDL(s);                         /* 向数据链路层发送帧 */
        RPL_from_RDL(&f_ack);                  /* 从数据链路层接收回复帧 */
        RPL_to_SPL(f_ack, client_socket_desc); /* 向物理层发送回复帧 */
    }

    return 0;
}
