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
#include <sys/select.h>
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
        if ((fcntl(fd, F_SETLK, &lock)) == 0)
        {
            return;
        }
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
        //printf("end\n");

        sleep(1);
        rd_ret = read(fd, buffer->data, MAX_PKT);
        if (rd_ret == 0)
            memset(buffer->data, '\0', MAX_PKT);
        else if (rd_ret < MAX_PKT) //文件读取结束
        {                          //将共享文件最后补全\0
            //memcpy(&buffer[rd_ret], CMPSTR, MAX_PKT - rd_ret);
            while (1)
            {
                sleep(1); //文件可能没有生成完毕，当读取不满MAX时，让进程sleep一秒继续读取。
                rd_ret += read(fd, &buffer->data[rd_ret], MAX_PKT);
                if (rd_ret == 0)
                {
                    memcpy(&buffer[rd_ret], CMPSTR, MAX_PKT - rd_ret);
                    break;
                }
                if (rd_ret == MAX_PKT)
                    break;
            }
        }
    }
    else if (rd_ret < MAX_PKT) //文件读取结束
    {                          //将共享文件最后补全\0
        //memcpy(&buffer[rd_ret], CMPSTR, MAX_PKT - rd_ret);
        while (1)
        {
            sleep(1); //文件可能没有生成完毕，当读取不满MAX时，让进程sleep一秒继续读取。
            rd_ret += read(fd, &buffer->data[rd_ret], MAX_PKT);
            if (rd_ret == 0)
            {
                memcpy(&buffer[rd_ret], CMPSTR, MAX_PKT - rd_ret);
                break;
            }
            if (rd_ret == MAX_PKT)
                break;
        }
    }
}
//addr共享内存起始地址
void SDL_to_SPL(frame *s, char *addr, int *cnt_sended_frames)
{
    if (current_protocol == PROTOCOL1)
    {
        while (1)
        {
            if (addr[MEM_FLAG_ADDR] == Can_Write && s->kind == data)
            {
                memcpy(&addr[DATA_START_ADDR], s, MAX_PKT + 12);
                addr[MEM_FLAG_ADDR] = Can_Read;
                (*cnt_sended_frames)++;
                printf("数据链路层成功发给物理层第%d个帧\n", *cnt_sended_frames);
                fflush(stdout);
                break;
            }
        }
    }
    else if (current_protocol == PROTOCOL2)
    {
        while (1)
        {
            if (addr[MEM_FLAG_ADDR] == Can_Write_Not_Send && s->kind == data)
            {
                memcpy(&addr[DATA_START_ADDR], s, MAX_PKT + 12);
                addr[MEM_FLAG_ADDR] = Can_Read_Not_Send;
                (*cnt_sended_frames)++;
                printf("数据链路层成功发给物理层第%d个帧\n", *cnt_sended_frames);
                fflush(stdout);
                break;
            }
        }
    }
}

void SPL_from_SDL(frame *s, char *addr)
{
    if (current_protocol == PROTOCOL1)
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
    }
    else if (current_protocol == PROTOCOL2)
    {
        while (1)
        {
            if (addr[MEM_FLAG_ADDR] == Can_Read_Not_Send)
            {
                memcpy(s, &addr[DATA_START_ADDR], MAX_PKT + 12);
                addr[MEM_FLAG_ADDR] = Can_Write_Send;
                break;
            }
        }
    }

    return;
}

void RPL_to_RDL(frame *s, char *addr)
{
    if (PROTOCOL1 == current_protocol)
    {
        while (1)
        {
            if (addr[MEM_FLAG_ADDR] == Can_Write)
            {
                memcpy(&addr[DATA_START_ADDR], s, MAX_PKT + 12);
                addr[MEM_FLAG_ADDR] = Can_Read;
                break;
            }
        }
    }
    else if (PROTOCOL2 == current_protocol)
    {
        while (1)
        {
            if (addr[MEM_FLAG_ADDR] == Can_Write_Not_Send)
            {
                memcpy(&addr[DATA_START_ADDR], s, MAX_PKT + 12);
                addr[MEM_FLAG_ADDR] = Can_Read_Not_Send;
                break;
            }
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

/*接收方从物理层取得帧
帧头尾的FLAG字节、数据中的字节填充均已去掉
调用本函数前已验证过校验和，若发生错误则发送cksum_err事件，
因此只有帧正确的情况下会调用本函数*/
void RDL_from_RPL(frame *s, char *addr)
{
    if (PROTOCOL1 == current_protocol)
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
    }
    else if (PROTOCOL2 == current_protocol)
    {
        while (1)
        {
            if (addr[MEM_FLAG_ADDR] == Can_Read_Not_Send)
            {
                memcpy(s, &addr[DATA_START_ADDR], MAX_PKT + 12);
                addr[MEM_FLAG_ADDR] = Can_Write_Send;
                break;
            }
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
    long int n = 0;
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
    close(fd);
}

boolen fit_percentage(int percentage)
{
    return rand() % 1000 < percentage ? true : false;
}

void RDL_to_RNL(packet *p)
{

    char file_name[30] = "temp_file.share";
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

    char buf[10];
    if (s.kind == data)
        strcpy(buf, "data");

    else if (s.kind == ack)
        strcpy(buf, "ack");
    else if (s.kind == nak)
        strcpy(buf, "nak");
    printf("接收到%s帧  ", buf);
    printf("发送序号是%d  ", s.seq);
    printf("接收序号是%d\n", s.ack);
}

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

    int len;
    if (s.kind == data)
    {
        convert_frame(&s);
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
        convert_frame(&s);
        len = write(client_socket_desc, &s, 12);
        if (len < 0)
        {
            perror("物理层向物理层发送数据错误");
            exit(1);
        }
        //printf("本次发送控制帧%d字节\n", len);
    }
}

void RPL_from_RDL(frame *s, char *addr)
{
    if (PROTOCOL2 == current_protocol)
    {
        while (1)
        {

            if (addr[MEM_FLAG_ADDR] == Can_Read_Send)
            {
                memcpy(s, &addr[DATA_START_ADDR], MAX_PKT + 12);
                addr[MEM_FLAG_ADDR] = Can_Write_Not_Send;

                break;
            }
        }
    }

    return;
}

void RPL_to_SPL(frame s, int client_socket_desc)
{
    int len;
    if (s.kind == data)
    {
        convert_frame(&s);
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
        convert_frame(&s);
        len = write(client_socket_desc, &s, 12);
        if (len < 0)
        {
            perror("物理层向物理层发送数据错误");
            exit(1);
        }
        printf("成功发送ACK\n");
    }
}


void SPL_from_RPL(frame *s, int client_socket_desc)
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
        //printf("total%d\n", total);
        fflush(stdout);
        if (total == 12)
            break;
    }
    char2frame(frame_header, s);
    print_frame(*s);
    fflush(stdout);
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


void SPL_from_RPL1(frame *s, int client_socket_desc, char *addr)
{
    fd_set rest;
    /*select延时变量*/
    struct timeval tempval;
    // lect等待时间
    tempval.tv_sec = 0;
    // lect等待秒数
    tempval.tv_usec = 0;
    // lect等待毫秒数

    char frame_header[12];
    int rd_ret;
    int total = 0;

    while (1)
    {
        //把监听套接字放入读操作符
        FD_ZERO(&rest); //清空读操作符
        FD_SET(client_socket_desc, &rest);
        int select_return = select(client_socket_desc + 1, &rest, NULL, NULL, &tempval); //监听套接的可读和可写条件
        if (select_return < 0)
        {
            perror("select");
            exit(1);
        }
        if (FD_ISSET(client_socket_desc, &rest))
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
            //printf("total%d\n", total);
            fflush(stdout);
            if (total == 12)
                break;
        }
        if (addr[MEM_FLAG_ADDR] == Can_Write_Not_Send) //超时或者错误数据
            return;
    }
    char2frame(frame_header, s);
    print_frame(*s);
    fflush(stdout);
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

//判断是ack包，则将共享内存改为可写
void SPL_to_SDL(frame *s, char *addr)
{

    while (1)
    {
        if (current_protocol == PROTOCOL2 && s->kind == ack)
        {

            if (addr[MEM_FLAG_ADDR] == Can_Write_Send)
            {
                addr[MEM_FLAG_ADDR] = Can_Write_Not_Send;
                break;
            }
        }
    }
}

void RDL_to_RPL(frame *s, char *addr)
{
    if (PROTOCOL2 == current_protocol)
    {
        while (1)
        {
            if (addr[MEM_FLAG_ADDR] == Can_Write_Send)
            {
                memcpy(&addr[DATA_START_ADDR], s, MAX_PKT + 12);
                addr[MEM_FLAG_ADDR] = Can_Read_Send;

                break;
            }
        }
    }

    return;
}

void init_f_ack(frame *s)
{
    s->ack = 0;
    s->seq = 0;
    s->kind = ack;
}

void SDL_from_SPL(char *addr)
{
    if (current_protocol == PROTOCOL2)
    {
        while (1)
        {
            if (addr[MEM_FLAG_ADDR] == Can_Write_Not_Send)
                break;
        }
    }
}
void to_final_file()
{
    //从temp_file.share中读取数据到d_file.txt
    packet buffer;
    char sharefilename[30] = "temp_file.share";
    char filename[30] = "d_file.txt";
    while (1) //判断文件是否存在
    {
        if (access(sharefilename, 0) != -1)
            break;
    }
    int fd_share = open(sharefilename, O_RDONLY);
    int fd = open(filename, O_CREAT | O_WRONLY | O_APPEND);
    if (fd < 0)
    {
        perror("RNL_open");
        exit(1);
    }
    while (1)
    {
        //每次从temp_file.share中读取1024字节到buffer
        RNL_from_RDL(&buffer, fd_share);
        //若读取全尾0，则退出程序
        if (memcmp(CMPSTR, buffer.data, sizeof(char) * MAX_PKT) == 0)
        {
            printf("写入文件结束，程序退出\n");
            break;
        }
        write(fd, buffer.data, MAX_PKT);
    }
    system("rm -rf temp_file.share");
    close(fd);
}

void RNL_from_RDL(packet *buffer, int fd)
{
    //从temp_file.share中读取1024字节到buffer
    int total = 0;
    int len;
    while (1)
    {
        len = read(fd, buffer->data + total, MAX_PKT - total);
        if (len < 0)
        {
            perror("read fail");
            exit(1);
        }
        total += len;
        if (total == MAX_PKT)
            break;
    }
}
