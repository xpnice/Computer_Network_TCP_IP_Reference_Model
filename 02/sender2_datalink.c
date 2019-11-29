/*发送端数据链路层*/
#include "common.h"
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>

void sender1()
{
    frame s;
    packet buffer;
    int file_id = 0;
    int cnt_sended_frames = 0;
    while (true)
    {
        printf("**********************\n");
        SDL_from_SNL(&buffer, &file_id); //从网络层读数据
        memcpy(s.info.data, buffer.data, MAX_PKT);

        init_frame(&s);

        int i = 0;
        for (i = 0; i < MAX_PKT; i++)
            printf("%c", s.info.data[i]);
        SDL_to_SPL(&s, &cnt_sended_frames);
        if (file_id == MAX_SHARE_FILES)
        {
            int SNL_PID = get_pid("sender1_network");
            if (SNL_PID != 0)
                kill(SNL_PID, SHARE_FILE_END);
        }
    }
}

int main()
{

    sender1();
    return 0;
}