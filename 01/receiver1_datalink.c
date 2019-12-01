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
        RDL_from_RPL(&s, addr_RDL_RPL); /* 从物理层获取帧 */
        RDL_to_RNL(&s.info);            /* 传递包到网络层 */
        cnt++;
        printf("接收到第%d帧\n", cnt);
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