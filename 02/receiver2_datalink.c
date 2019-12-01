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

    int cnt = 0;
    while (true)
    {
        printf("**********************\n");
        //此时，需要共享内存标志位为Can_Read & ~Send_Ack
        RDL_from_RPL(&s, addr_RDL_RPL); /* 从物理层获取帧 */

        //此时，共享内存标志位被置为Can_Write & Send_Ack

        RDL_to_RNL(&s.info); /* 传递包到网络层 */
        cnt++;
        printf("接收到第%d帧\n",cnt);
        /*如果是全尾0的包，则释放共享内存*/
        if (memcmp(CMPSTR, s.info.data, sizeof(char) * MAX_PKT) == 0)
        {
            shmdt(addr_RDL_RPL);
            DestroyShm(MEM_SHMID[RDL_RPL_KEYID]);
            break;
        }

        //此时，需要共享内存标志位为Can_Write & Send_Ack
        init_f_ack(&f_ack);
        RDL_to_RPL(&f_ack, addr_RDL_RPL); /*向物理层传输确认帧，以示收到此帧，请求下一帧*/
        //此时，共享内存标志位被置为Can_Read & Send_Ack
    }
}

int main()
{
    current_protocol = PROTOCOL2;
    receiver1();
    return 0;
}