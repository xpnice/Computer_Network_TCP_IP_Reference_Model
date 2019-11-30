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
#include "common.h"

//函数声明
pid_t get_pid(char *s);
void lock_set(int fd, int type);
void init_frame(frame *s);
void char2frame(char *str, frame *s);
void print_frame(frame s);
int create_bind();
void convert_frame(frame *s);
//全局信号变量
boolen READ_SIGNAL = false;

void sysLocalTime()
{
    time_t timesec;
    struct tm *p;

    time(&timesec);
    p = localtime(&timesec);

    printf("%d%d%d%d%d%d\n", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
}

void sysUsecTime()
{
    struct timeval tv;
    struct timezone tz;

    struct tm *p;

    gettimeofday(&tv, &tz);
    p = localtime(&tv.tv_sec);
    printf("time_now:%d-%d-%d-%d-%d-%d.%ld\n", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, tv.tv_usec);
}

//comm.c
static int CommShm(int size, int flags, int id)
{

    key_t key = ftok(".", id);
    if (key < 0)
    {
        perror("ftok");
        return -1;
    }
    int shmid = 0;
    if ((shmid = shmget(key, size, flags)) < 0)
    {
        perror("shmget");
        return -2;
    }
    return shmid;
}

int DestroyShm(int shmid)
{
    if (shmctl(shmid, IPC_RMID, NULL) < 0)
    {
        perror("shmctl");
        return -1;
    }
    return 0;
}

int CreateShm(int size, int id)
{
    return CommShm(size, IPC_CREAT | IPC_EXCL | 0666, id);
}

int GetShm(int size, int id)
{
    return CommShm(size, IPC_CREAT, id);
}

//PID
int get_pid(char *s)
{
    char res[1024] = "";
    char cmd[256];
    sprintf(cmd, "ps -ef | grep %s | grep -v grep | awk '{print $2}'", s);
    FILE *pipe = popen(cmd, "r");
    if (!pipe)
        exit(0);
    char buffer[128];
    while (!feof(pipe))
    {
        if (fgets(buffer, 128, pipe) != NULL)
            strcat(res, buffer);
    }
    pclose(pipe);
    return atoi(res);
}

void init_frame(frame *s)
{
    s->ack = 0;
    s->seq = 0;
    s->kind = data;
}
//信号处理函数
void SDL_START_SIGNAL(int s)
{
    sdl_st = true;
}
void SPL_START_SIGNAL(int s)
{
    spl_st = true;
}

// void enable_network_layer(void)
// {

//     kill(SN_pid, ENABLE);
// }

// void disable_network_layer(void)
// {
//     kill(SN_pid, ENABLE);
// }

/*lock_set函数*/
void lock_set(int fd, int type)
{
    struct flock lock;
    lock.l_whence = SEEK_SET; //赋值lock结构体
    lock.l_start = 0;
    lock.l_len = 0;
    while (1)
    {
        lock.l_type = type;
        /*根据不同的type值给文件上锁或解锁*/
        // printf("你没错\n");
        // fflush(stdout);
        if ((fcntl(fd, F_SETLK, &lock)) == 0)
        {
            // if (lock.l_type == F_RDLCK)
            //     printf("read lock set by %d\n", getpid());
            // else if (lock.l_type == F_WRLCK)
            //     printf("write lock set by %d\n", getpid());
            // else if (lock.l_type == F_UNLCK)
            //     printf("release lock by %d\n", getpid());
            // printf("天气真不好\n");
            // fflush(stdout);
            return;
        }
        // printf("很开心呢\n");
        // fflush(stdout);
        /*判断文件是否可以上锁*/
        fcntl(fd, F_GETLK, &lock);

        /*判断文件不能上锁的原因*/
        // if (lock.l_type != F_UNLCK)
        // {
        //     /*/该文件已有写入锁*/
        //     if (lock.l_type == F_RDLCK)
        //         printf("read lock already set by %d\n", lock.l_pid);
        //     /*该文件已有读取锁*/
        //     else if (lock.l_type == F_WRLCK)
        //         printf("write lock already set by %d\n", lock.l_pid);
        //     //getchar();
        // }
    }
}
void SDL_from_SNL(packet *buffer, int fd)
{

    //lock_set(fd, F_RDLCK);

    int rd_ret = read(fd, buffer->data, MAX_PKT);
    if (rd_ret < 0)
    {
        printf("从文件读数据错误\n");
        exit(1);
    }
    else if (rd_ret == 0)
    {
        //无数据可读，最后发一个全尾0数据包
        memset(buffer->data, '\0', MAX_PKT);
    }
    else if (rd_ret < MAX_PKT) //文件读取结束
        //将共享文件最后补全\0
        memcpy(&buffer[rd_ret], CMPSTR, MAX_PKT - rd_ret);
    //lock_set(fd, F_UNLCK);
    //close(fd);
}

// void SDL_from_SNL(packet *buffer, int *file_id)
// {
//     // while (true)
//     // {
//     //     //printf("%d", spl_st);
//     //     //getchar();
//     //     if (spl_st == true)
//     //         break;
//     //     signal(SPL_START, SPL_START_SIGNAL);
//     //     //printf("%d", spl_st);
//     // }
//     int fd;
//     (*file_id)++; //  network_datalink.share.0001~0999
//     char file_name[30];
//     sprintf(file_name, "./stxt/network_datalink.share.%04d", *file_id);
//     while (1)
//     {
//         if (access(file_name, 0) != -1)
//             break;
//     }

//     fd = open(file_name, O_RDONLY);
//     if (fd < 0)
//     {
//         perror("open");
//         exit(1);
//     }
//     // printf("打开文件%s\n", file_name);
//     // fflush(stdout);
//     lock_set(fd, F_RDLCK);

//     read(fd, buffer->data, MAX_PKT);
//     // return fd;
//     lock_set(fd, F_UNLCK);

//     close(fd);
//     // char dele[40];
//     // sprintf(dele, "rm -rf %s", file_name);
//     // system(dele);

//     return;
// }

/*
    从数据链路层发往物理层
*/
// void SDL_to_SPL(frame *s, int *cnt_sended_frames)
// {

//     if (s->kind == data)
//     {
//         char *share_filename = "SDL_SPL.share"; //存储共享文件名字
//         while (1)
//         {
//             if (access(share_filename, 0) == -1)
//                 break;
//         }
//         int fd = open(share_filename, O_WRONLY | O_CREAT);
//         //lock_set(fd, F_WRLCK);
//         int wr_ret = write(fd, s, sizeof(frame));
//         if (wr_ret < 0)
//         {
//             printf("写SDL_SPL.share错误\n");
//             exit(1);
//         }
//         (*cnt_sended_frames)++;
//         printf("成功发给物理层%d个帧\n", *cnt_sended_frames);
//         int SPL_PID = get_pid("sender1_physical");
//         //lock_set(fd, F_UNLCK);
//         close(fd);
//         kill(SPL_PID, WRITE_FINISH);
//         printf("%d", SPL_PID);
//         //exit(1);
//     }
// }

//addr共享内存起始地址
void SDL_to_SPL(frame *s, char *addr, int *cnt_sended_frames)
{
    //printf("____________________\n");
    if (s->kind == data)
    {
        while (1)
        {
            if (addr[MEM_FLAG_ADDR] == Can_Write)
            {
                // printf("[[[[[[[[[[[[[[[[[[[[[[[[[[[[[\n");
                //sysUsecTime();
                memcpy(&addr[DATA_START_ADDR], s, MAX_PKT + 12);
                //sysUsecTime();
                addr[MEM_FLAG_ADDR] = Can_Read;
                //sysUsecTime();
                (*cnt_sended_frames)++;
                //printf("成功发给物理层%d个帧\n", *cnt_sended_frames);
                //printf("]]]]]]]]]]]]]]]]]]]]]]]]]]]]]\n");
                break;
            }
            //sysUsecTime();
        }
    }
    printf("------------------------\n");
}

void write_f(int s)
{
    printf("%d\n", s);
    fflush(stdout);
    READ_SIGNAL = true;
}

void SPL_from_SDL(frame *s, char *addr)
{
    while (1)
    {
        if (addr[MEM_FLAG_ADDR] == Can_Read)
        {
            memcpy(s, &addr[DATA_START_ADDR], MAX_PKT + 12);

            addr[MEM_FLAG_ADDR] = Can_Write;
            break;
        }
    }

    return;
}

void RPL_to_RDL(frame *s, char *addr)
{

    while (1)
    {
        if (addr[MEM_FLAG_ADDR] == Can_Write)
        {
            memcpy(&addr[DATA_START_ADDR], s, MAX_PKT + 12);
            addr[MEM_FLAG_ADDR] = Can_Read;
            // int RDL = get_pid("receiver1_datalink");
            // kill(RDL, FRAME_ARRIVAL);
            break;
        }
    }
    return;
}

boolen file_continue = false;

void file_end(int s)
{
    file_continue = true;
}

void SNL_to_SDL() //把生成文件跟想共享文件写入分开
{

    const char *filepath = "s_file.txt"; //1个1GB以上的文件
    if (access(filepath, 0) == -1)       //不存在1GB文件
        generate_file(filepath);         //生成文件
}

/*
    将1个1GB以上的文件处理为多个文件/1024字节
*/
// void SNL_to_SDL() //把生成文件跟想共享文件写入分开
// {
//     const char *filepath = "s_file.txt"; //1个1GB以上的文件
//     char share_filename[40];             //存储共享文件名字
//     int cnt_file = 0;                    //对分解出来的共享文件计数
//     int count = 0;
//     if (access("./stxt/", 0) == -1)                             //不存在文件夹
//         mkdir("./stxt", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); //新建文件夹
//     if (access(filepath, 0) == -1)                              //不存在1GB文件
//         generate_file(filepath);                                //生成文件
//     int fd = open(filepath, O_RDONLY);
//     if (fd == -1)
//     {
//         perror("open error");
//         exit(1);
//     }
//     char temp_buf[MAX_PKT];
//     int new_fd;
//     int i;
//     for (i = 0; i < MAX_PKT; i++)
//         temp_buf[i] = '\0';
//     lock_set(fd, F_RDLCK); //加读锁
//     while (1)
//     {
//         cnt_file++;
//         count++;
//         sprintf(share_filename, "./stxt/network_datalink.share.%04d", cnt_file);
//         char buffer[MAX_PKT];
//         int rd_ret = read(fd, buffer, MAX_PKT);
//         if (rd_ret < 0)
//         {
//             printf("从文件读数据错误\n");
//             exit(1);
//         }
//         else if (rd_ret == 0)
//         {
//             //无数据可读，最后发一个全尾0数据包
//             new_fd = open(share_filename, O_WRONLY | O_CREAT);
//             lock_set(new_fd, F_WRLCK); //即将开始写共享文件，加写锁

//             int wr_ret = write(new_fd, temp_buf, MAX_PKT);
//             if (wr_ret < 0)
//             {
//                 printf("写%04d共享文件错误\n", cnt_file);
//                 exit(1);
//             }
//             lock_set(new_fd, F_UNLCK); //写入成功，释放写锁
//             close(new_fd);
//             printf("已拆分成功%d个文件，拆分工作结束\n", count);
//             //sysUsecTime();
//             break;
//         }
//         else if (rd_ret < MAX_PKT) //文件读取结束
//             //将共享文件最后补全\0
//             memcpy(&buffer[rd_ret], temp_buf, MAX_PKT - rd_ret);

//         new_fd = open(share_filename, O_WRONLY | O_CREAT);
//         lock_set(new_fd, F_WRLCK); //即将开始写共享文件，加写锁
//         //getchar();
//         int wr_ret = write(new_fd, buffer, MAX_PKT);
//         if (wr_ret < 0)
//         {
//             printf("写%04d共享文件错误\n", cnt_file);
//             exit(1);
//         }
//         lock_set(new_fd, F_UNLCK); //写入成功，释放写锁
//         close(new_fd);
//         //printf("已拆分成功%d个文件\n", cnt_file);
//         if (cnt_file == MAX_SHARE_FILES) //暂且写999个共享文件
//         {
//             while (1)
//             {
//                 signal(SHARE_FILE_END, file_end);
//                 if (file_continue == true)
//                 {
//                     file_continue = false;
//                     break;
//                 }
//             }
//             cnt_file = 0;
//         }
//     }
//     lock_set(fd, F_UNLCK);
//     close(fd);
//     return;
// }

//共享内存实现
// void SNL_to_SDL() //把生成文件跟想共享文件写入分开
// {

// }

/*接收方从物理层取得帧
帧头尾的FLAG字节、数据中的字节填充均已去掉
调用本函数前已验证过校验和，若发生错误则发送cksum_err事件，
因此只有帧正确的情况下会调用本函数*/
void RDL_from_RPL(frame *s, char *addr)
{
    while (1)
    {
        if (addr[MEM_FLAG_ADDR] == Can_Read)
        {
            memcpy(s, &addr[DATA_START_ADDR], MAX_PKT + 12);
            addr[MEM_FLAG_ADDR] = Can_Write;
            break;
        }
    }
    return;
}

/*
    生成1GB的文件
*/
void generate_file(const char *filename)
{
    srand((unsigned int)(time(NULL)));
    int fd = open(filename, O_WRONLY | O_CREAT, 0666);
    //lock_set(fd, F_WRLCK);
    //long int N = 1024 * 10;
    unsigned long int n = 0;
    while (1)
    {
        char s[KB];
        int i = 0;
        for (i = 0; i < KB; i++)
            s[i] = rand() % 256;
        int k = write(fd, &s, KB);
        n += k;
        if (n == SIZEOF_FILE)
        {
            printf("文件写入成功\n");
            break;
        }
    }
    //lock_set(fd, F_UNLCK);
    close(fd);
}

// void RNL_from_RDL(packet *buffer, char *addr)
// {
//     while (1)
//     {
//         if ((int)(addr[MEM_FLAG_ADDR]) == Can_Read)
//         {
//             memcpy(buffer, &addr[DATA_START_ADDR], MAX_PKT);
//             addr[MEM_FLAG_ADDR] = Can_Write;
//             break;
//         }
//     }
//     return;
// }
void RNL_from_RDL(packet *buffer, int *file_id, int nfd)
{
    int fd;
    (*file_id)++; //文件格式后缀序号，从0开始每次执行++，所读取的文件名为  network_datalink.share.0001~0999
    if ((*file_id) == MAX_SHARE_FILES + 1)
        *file_id = 1;
    char file_name[30];
    sprintf(file_name, "./dtxt/network_datalink.share.%04d", *file_id);
    while (1)
    {
        if (access(file_name, 0) != -1)
            break;
    }
    fd = open(file_name, O_RDONLY);
    if (fd < 0)
    {
        perror("open");
        exit(1);
    }
    while (1)
    {
        lock_set(fd, F_RDLCK);
        int ret = read(fd, buffer->data, MAX_PKT);
        lock_set(fd, F_UNLCK);
        printf("正在读文件:%s----%d\n", file_name, ret);
        fflush(stdout);
        if (ret == 1024)
            break;
    }

    // int i = 0;

    // for (i = 0; i < MAX_PKT; i++)
    //     printf("%c", buffer->data[i]);
    // printf("\n*********************\n");

    //write(nfd, buffer->data, MAX_PKT);

    close(fd);
    // char dele[40];
    // sprintf(dele, "rm -rf %s", file_name);
    // system(dele);
    return;
}

//接收端将受到的数据写入文件中
// void to_final_file()
// {
//     //创建RNL_RDL共享内存
//     int shmid = GetShm(MEM_SIZE, RNL_RDL_KEYID);
//     char *addr_RNL_RDL = shmat(shmid, NULL, 0);
//     if (addr_RNL_RDL != NULL)
//     {
//         printf("成功链接共享内存\n");
//     }
//     //---------------------------
//     char cmp[MAX_PKT];
//     memset(cmp, 0, sizeof(char) * MAX_PKT);
//     packet buffer;
//     char file_name[30] = "d_file.txt";
//     int fd = open(file_name, O_WRONLY | O_CREAT);
//     if (fd < 0)
//     {
//         perror("RNL_open");
//         exit(1);
//     }
//     int i = 0;
//     while (true)
//     {
//         RNL_from_RDL(&buffer, addr_RNL_RDL);
//         if (memcmp(cmp, buffer.data, sizeof(char) * MAX_PKT) == 0)
//         {
//             printf("写入文件结束，程序退出\n");
//             break;
//         }
//         write(fd, buffer.data, MAX_PKT);
//     }
//     close(fd);
// }

void to_final_file(int *file_id)
{
    //创建RNL_RDL共享内存
    int shmid = GetShm(MEM_SIZE, RNL_RDL_KEYID);
    char *addr_RNL_RDL = shmat(shmid, NULL, 0);
    if (addr_RNL_RDL != NULL)
    {
        printf("成功链接共享内存\n");
    }
    MEM_SHMID[RNL_RDL_KEYID] = shmid;
    //------------------------------------------
    char cmp[MAX_PKT];
    memset(cmp, 0, sizeof(char) * MAX_PKT);
    packet buffer;
    char file_name[30] = "d_file.txt";
    int fd = open(file_name, O_WRONLY | O_CREAT);
    if (fd < 0)
    {
        perror("RNL_open");
        exit(1);
    }
    int i = 0;
    while (true)
    {
        //sysUsecTime();
        RNL_from_RDL(&buffer, file_id, fd);
        //sysUsecTime();
        if (memcmp(cmp, buffer.data, sizeof(char) * MAX_PKT) == 0)
        {
            printf("写入文件结束，程序退出\n");
            fflush(stdout);
            //关闭所有共享内存
            close(fd);
            shmdt(addr_RNL_RDL);
            DestroyShm(MEM_SHMID[RNL_RDL_KEYID]);

            break;
        }

        // reveiver_from_datalink_layer(&buffer);

        write(fd, buffer.data, MAX_PKT);
        //sysUsecTime();
        printf("file_id:%d\n", *file_id);

        // if (*file_id == MAX_SHARE_FILES)
        // {
        //     int RDL_PID = get_pid("receiver1_datalink");
        //     kill(RDL_PID, SHARE_FILE_END);
        // }
    }
}

// void RDL_to_RNL(packet *p, int *file_id)
// {

//     //将收到的包从0001到0999依次写入文件，循环
//     //printf("1\n");
//     if (access("./dtxt/", 0) == -1)                             //不存在文件夹
//         mkdir("./dtxt", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); //新建文件夹
//     char filename[40];
//     (*file_id)++;
//     if (*file_id == MAX_SHARE_FILES + 1)
//         *file_id = 1;
//     sprintf(filename, "./dtxt/network_datalink.share.%04d", *file_id);
//     int fd = open(filename, O_WRONLY | O_CREAT);
//     if (fd < 0)
//     {
//         perror("open");
//         exit(1);
//     }
//     lock_set(fd, F_WRLCK);
//     write(fd, p->data, MAX_PKT);
//     lock_set(fd, F_UNLCK);
//     //sleep(1);
//     close(fd);
//     return;
// }

void RDL_to_RNL(packet *p, int *file_id)
{

    char file_name[30] = "d_file.txt";
    int fd;
    fd = open(file_name, O_CREAT | O_WRONLY | O_APPEND);
    if (fd < 0)
    {
        perror("open");
        exit(1);
    }
    write(fd, p->data, MAX_PKT);
    close(fd);
}

// void RDL_to_RNL(packet *p, char *addr)
// {
//     while (1)
//     {
//         if ((int)(addr[MEM_FLAG_ADDR]) == Can_Write)
//         {
//             memcpy(&addr[DATA_START_ADDR], p, MAX_PKT);
//             addr[MEM_FLAG_ADDR] = Can_Read;
//             break;
//         }
//     }
//     return;
// }

void FRAME_ARRIVAL_SIGNAL(int s)
{
    frame_arr = true;
}

void wait_for_event(event_type *event)
{
    while (true)
    {
        signal(FRAME_ARRIVAL, FRAME_ARRIVAL_SIGNAL);
        if (frame_arr)
        {
            frame_arr = false;
            event = frame_arrival;
            return;
        }
    }
}

/*
    将收到的char数组变为frame存储
*/
void char2frame(char *str, frame *s)
{
    char buf[4];
    memcpy(buf, str, 4);
    s->kind = ntohl(*((int *)buf));
    memcpy(buf, str + 4, 4);
    s->seq = ntohl(*((int *)buf));
    memcpy(buf, str + 8, 4);
    s->ack = ntohl(*((int *)buf));
}
void print_frame(frame s)
{
    printf("------------------------------\n");
    char buf[10];
    if (s.kind == data)
        strcpy(buf, "data");

    else if (s.kind == ack)
        strcpy(buf, "ack");
    else if (s.kind == nak)
        strcpy(buf, "nak");
    printf("帧类型是%s\n", buf);
    printf("发送序号是%d\n", s.seq);
    printf("接收序号是%d\n", s.ack);
}

//修改过！！！！！！！！！！！！！！！！！！！！！！！！
void RPL_from_SPL(frame *s, int client_socket_desc)
{
    char frame_header[12];
    int rd_ret;
    int total = 0;
    while (1)
    {
        rd_ret = read(client_socket_desc, frame_header + total, 12 - total);
        if (rd_ret < 0)
        {
            perror("receiver物理层从sender物理层接收数据出错\n");
            exit(1);
        }
        if (rd_ret == 0)
        {
            printf("对方中断连接\n");
            fflush(stdout);
            exit(1);
        }
        total += rd_ret;
        if (total == 12)
            break;
    }
    char2frame(frame_header, s);
    print_frame(*s);

    total = 0;
    if (s->kind == data)
    {
        //继续读1024个数据
        while (1)
        {
            rd_ret = read(client_socket_desc, &s->info.data[0] + total, MAX_PKT - total);
            if (rd_ret < 0)
            {
                perror("receiver物理层从sender物理层接收数据出错\n");
                exit(1);
            }
            if (rd_ret == 0)
            {
                printf("对方中断连接\n");
                fflush(stdout);
                exit(1);
            }
            total += rd_ret;
            if (total == MAX_PKT)
                break;
        }
        printf("接收数据帧成功\n");
    }
    return;
}

int create_bind()
{

    int socket_desc;
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
    {
        perror("could not create socket");
        exit(1);
    }
    int reuse0 = 1;
    if (setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse0, sizeof(reuse0)) == -1)
    {
        perror("reuse");
        exit(1);
    }

    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(atoi(RECEIVER_PORT));

    //bind
    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("bind failed");
        exit(1);
    }

    return socket_desc;
}

/*
    将frame中的int型转换为网络序
*/
void convert_frame(frame *s)
{
    s->kind = htonl(s->kind);
    s->ack = htonl(s->ack);
    s->seq = htonl(s->seq);
}

/*
    向receiver端的物理层发送数据包
*/
void SPL_to_RPL(frame s, int client_socket_desc)
{
    convert_frame(&s);
    int len;
    if (s.kind == data)
    {
        len = write(client_socket_desc, &s, MAX_PKT + 12);
        if (len < 0)
        {
            perror("物理层向物理层发送数据错误");
            exit(1);
        }
        //printf("本次发送%d\n", len);
    }
    else if (s.kind == ack || s.kind == nak)
    {
        len = write(client_socket_desc, &s, 12);
        if (len < 0)
        {
            perror("物理层向物理层发送数据错误");
            exit(1);
        }
        //printf("本次发送控制帧%d字节\n", len);
    }
}

void RPL_from_RDL(frame *s)
{
    char file_name[30] = "ACK_RDL_RPL.share";
    //等待数据链路层写好后发送一个信号，开始读文件
    //////////////////////////////////////////
    while (1)
    {
        if (access(file_name, 0) != -1)
            break;
    }
    int fd = open(file_name, O_RDONLY);
    if (fd < 0)
    {
        perror("open");
        exit(1);
    }
    read(fd, s, MAX_PKT + 12);
    close(fd);
}

void RPL_to_SPL(frame s, int client_socket_desc)
{
    convert_frame(&s);
    int len;
    if (s.kind == data)
    {
        len = write(client_socket_desc, &s, MAX_PKT + 12);
        if (len < 0)
        {
            perror("物理层向物理层发送数据错误");
            exit(1);
        }
        printf("本次发送数据帧%d字节\n", len);
    }
    else if (s.kind == ack || s.kind == nak)
    {
        len = write(client_socket_desc, &s, 12);
        if (len < 0)
        {
            perror("物理层向物理层发送数据错误");
            exit(1);
        }
        printf("本次发送控制帧%d字节\n", len);
    }
}

void SPL_from_RPL(frame *s, int client_socket_desc)
{
    char frame_header[MAX_PKT + 12];
    int rd_ret = read(client_socket_desc, frame_header, MAX_PKT + 12);
    if (rd_ret < 0)
    {
        perror("receiver物理层从sender物理层接收数据出错\n");
        exit(1);
    }
    if (rd_ret == 0)
    {
        printf("对方中断连接\n");
        fflush(stdout);
    }

    char2frame(frame_header, s);
    print_frame(*s);
    if (s->kind == data)
    {
        if (rd_ret == (MAX_PKT + 12))
        {
            memcpy(s->info.data, &frame_header[12], MAX_PKT);
            printf("接收数据帧成功\n");
        }
        else
            printf("接收数据帧失败:%d\n", rd_ret);
    }
    else if (s->kind == ack || s->kind == nak)
        if (rd_ret == 12)
        {
            //SPL_LOST_PERC/10%丢弃->数据链路层等待超时
            if (fit_percentage(SPL_LOST_PERC))
            {
                //标志位不变：等待
                return;
            }
            //SPL_CHER_PERC/10%丢弃->向数据链路层发送chsum_error
            else if (fit_percentage(SPL_CHER_PERC))
            {
                //标志位置错误
                return;
            }
            else
            {
                //标志位置收到
                printf("接收控制帧成功\n");
            }
        }
        else
            printf("接收控制帧失败:%d\n", rd_ret);
    return;
}
//是否满足该概率
boolen fit_percentage(int percentage)
{
    return rand() % 1000 < percentage ? true : false
}
