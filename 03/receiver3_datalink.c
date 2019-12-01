/*接收方数据链路层*/
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>

boolen file_continue = false;
//函数声明
void receiver1();

void file_end(int s)
{
    file_continue = true;
}

void receiver1()
{
    frame s;          /* 帧 */
    frame f_ack;      /*确认帧*/
    event_type event; /* 事件类型 */
    init_frame(&s);
    int file_id = 0;
    //创建RDL_RPL共享内存
    int shmid = GetShm(MEM_SIZE, RDL_RPL_KEYID);
    char *addr_RDL_RPL = shmat(shmid, NULL, 0);
    if (addr_RDL_RPL != NULL)
    {
        printf("成功链接共享内存\n");
    }
    MEM_SHMID[RDL_RPL_KEYID] = shmid;
    //memset(addr_RDL_RPL, '\0', MEM_SIZE);
    //addr_RDL_RPL[MEM_FLAG_ADDR] = Can_Write;

    //创建RNL_RDL共享内存
    shmid = GetShm(MEM_SIZE, RNL_RDL_KEYID);
    char *addr_RNL_RDL = shmat(shmid, NULL, 0);
    if (addr_RNL_RDL != NULL)
    {
        printf("成功链接共享内存\n");
    }
    //memset(addr_RNL_RDL, '\0', MEM_SIZE);
    //addr_RNL_RDL[MEM_FLAG_ADDR] = Can_Write;

    int cnt = 0;
    while (true)
    {
        //printf("%d\n", frame_arr);
        //wait_for_event(&event); /* 等待直到有帧到达 */
        printf("**********************\n");

        //此时，需要共享内存标志位为Can_Read & ~Send_Ack
        RDL_from_RPL(&s, addr_RDL_RPL); /* 从物理层获取帧 */
        //此时，共享内存标志位被置为Can_Write & Send_Ack



        //sysUsecTime();
        int i = 0;
        for (i = 0; i < MAX_PKT; i++)
            printf("%c", s.info.data[i]);
        printf("\n");
        cnt++;
        printf("第%d帧\n", cnt);
        fflush(stdout);
        //printf("%c----%c\n", addr_RDL_RPL[MEM_FLAG_ADDR], addr_RDL_RPL[MEM_ACK_FLAG_ADDR]);
        /*如果是全尾0的包，则释放共享内存，不将包向网络层传递*/
        if (memcmp(CMPSTR, s.info.data, sizeof(char) * MAX_PKT) == 0)
        {
            //printf("%c----%c\n", addr_RDL_RPL[MEM_FLAG_ADDR], addr_RDL_RPL[MEM_ACK_FLAG_ADDR]);
            shmdt(addr_RDL_RPL);
            DestroyShm(MEM_SHMID[RDL_RPL_KEYID]);
            break;
        }
        //printf("%c----%c\n", addr_RDL_RPL[MEM_FLAG_ADDR], addr_RDL_RPL[MEM_ACK_FLAG_ADDR]);
        RDL_to_RNL(&s.info, &file_id); /* 传递包到网络层 */
                                       //sysUsecTime();
        printf("传递包到网络层\n");

        /*是否使用同一块共享内存呢，我觉得可以，试一试*/

        //printf("%c----%c\n", addr_RDL_RPL[MEM_FLAG_ADDR], addr_RDL_RPL[MEM_ACK_FLAG_ADDR]);
        fflush(stdout);
        //此时，需要共享内存标志位为Can_Write & Send_Ack
        init_f_ack(&f_ack);
        RDL_to_RPL(&f_ack, addr_RDL_RPL); /*向物理层传输确认帧，以示收到此帧，请求下一帧*/
        //此时，共享内存标志位被置为Can_Read & Send_Ack
        printf("已发给物理层ACK\n");
        fflush(stdout);
    }
}

int main()
{
    current_protocol = PROTOCOL3;
    receiver1();
    return 0;
}