/*���շ�������·��*/
#include "common.h"
#include <stdio.h>
#include <stdlib.h>

boolen file_continue = false;
//��������
void receiver1();

void file_end(int s)
{
    file_continue = true;
}

void receiver1()
{
    frame s;          /* ֡ */
    event_type event; /* �¼����� */
    init_frame(&s);
    int file_id = 0;
    while (true)
    {
        //printf("%d\n", frame_arr);
        wait_for_event(&event); /* �ȴ�ֱ����֡���� */

        RDL_from_RPL(&s); /* ��������ȡ֡ */
        int i = 0;
        for (i = 0; i < MAX_PKT; i++)
            printf("%c", s.info.data[i]);
        printf("\n**********************\n");
        RDL_to_RNL(&s.info, &file_id); /* ���ݰ�������� */
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