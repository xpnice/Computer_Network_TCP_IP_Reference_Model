/*接收方数据链路层*/
#include "common.h"
#include <stdio.h>
#include <stdlib.h>

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
    while (true)
    {
        //printf("%d\n", frame_arr);
        wait_for_event(&event); /* 等待直到有帧到达 */

        RDL_from_RPL(&s); /* 从物理层获取帧 */
        int i = 0;
        for (i = 0; i < MAX_PKT; i++)
            printf("%c", s.info.data[i]);
        printf("\n**********************\n");
        RDL_to_RNL(&s.info, &file_id); /* 传递包到网络层 */
        if (file_id == MAX_SHARE_FILES)
        {
            while (1)
            {
                signal(SHARE_FILE_END, file_end);
                if (file_continue == true)
                {
                    file_continue = false;
                    break;
                }
            }
        }
    }
}

int main()
{
    system("rm -rf dtxt");
    receiver1();
    return 0;
}