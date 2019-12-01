/*接收方数据链路层*/
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

    int cnt = 0;
    while (true)
    {

        printf("**********************\n");

        RDL_from_RPL(&s, addr_RDL_RPL); /* 从物理层获取帧 */

        if (s.seq == seq_want)
        {
            RDL_to_RNL(&s.info); /* 传递包到网络层 */
            inc(seq_want);
            printf("传递包到网络层\n");
            cnt++;
            printf("第%d帧\n", cnt);
            fflush(stdout);
        }

        /*如果是全尾0的包，则释放共享内存*/
        if (memcmp(CMPSTR, s.info.data, sizeof(char) * MAX_PKT) == 0)
        {
            shmdt(addr_RDL_RPL);
            DestroyShm(MEM_SHMID[RDL_RPL_KEYID]);
            break;
        }

        init_f_ack(&f_ack);
        f_ack.seq = s.seq;
        RDL_to_RPL(&f_ack, addr_RDL_RPL); /*向物理层传输确认帧，以示收到此帧，请求下一帧*/
        printf("已发给物理层ACK\n");
        fflush(stdout);
    }
}

int main()
{
    current_protocol = PROTOCOL2;
    receiver1();
    return 0;
}