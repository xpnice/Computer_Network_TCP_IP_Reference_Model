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

//��������
pid_t get_pid(char *s);
void lock_set(int fd, int type);
void init_frame(frame *s);
void char2frame(char *str, frame *s);
void print_frame(frame s);
int create_bind();
void convert_frame(frame *s);
//ȫ���źű���
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
/*lock_set����*/
void lock_set(int fd, int type)
{
    struct flock lock;
    lock.l_whence = SEEK_SET; //��ֵlock�ṹ��
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
        printf("���ļ������ݴ���\n");
        exit(1);
    }
    else if (rd_ret == 0)
    {
        //�����ݿɶ������һ��ȫβ0���ݰ�
        //printf("end\n");

        sleep(1);
        rd_ret = read(fd, buffer->data, MAX_PKT);
        if (rd_ret == 0)
            memset(buffer->data, '\0', MAX_PKT);
        else if (rd_ret < MAX_PKT) //�ļ���ȡ����
        {                          //�������ļ����ȫ\0
            //memcpy(&buffer[rd_ret], CMPSTR, MAX_PKT - rd_ret);
            while (1)
            {
                sleep(1); //�ļ�����û��������ϣ�����ȡ����MAXʱ���ý���sleepһ�������ȡ��
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
    else if (rd_ret < MAX_PKT) //�ļ���ȡ����
    {                          //�������ļ����ȫ\0
        //memcpy(&buffer[rd_ret], CMPSTR, MAX_PKT - rd_ret);
        while (1)
        {
            sleep(1); //�ļ�����û��������ϣ�����ȡ����MAXʱ���ý���sleepһ�������ȡ��
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
//addr�����ڴ���ʼ��ַ
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
                printf("������·��ɹ�����������%d��֡\n", *cnt_sended_frames);
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
                printf("������·��ɹ�����������%d��֡\n", *cnt_sended_frames);
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

void SNL_to_SDL() //�������ļ����빲���ļ�д��ֿ�
{

    const char *filepath = "s_file.txt"; //1��1GB���ϵ��ļ�
    if (access(filepath, 0) == -1)       //������1GB�ļ�
        generate_file(filepath);         //�����ļ�
}

/*���շ��������ȡ��֡
֡ͷβ��FLAG�ֽڡ������е��ֽ�������ȥ��
���ñ�����ǰ����֤��У��ͣ���������������cksum_err�¼���
���ֻ��֡��ȷ������»���ñ�����*/
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
    ����1GB���ļ�
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
            printf("�ļ�д��ɹ�\n");
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
    ���յ���char�����Ϊframe�洢
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
    printf("���յ�%s֡  ", buf);
    printf("���������%d  ", s.seq);
    printf("���������%d\n", s.ack);
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
            perror("receiver������sender�����������ݳ���\n");
            exit(1);
        }
        if (rd_ret == 0)
        {
            printf("�Է��ж�����\n");
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
        //������1024������
        while (1)
        {
            rd_ret = read(client_socket_desc, &s->info.data[0] + total, MAX_PKT - total);
            if (rd_ret < 0)
            {
                perror("receiver������sender�����������ݳ���\n");
                exit(1);
            }
            if (rd_ret == 0)
            {
                printf("�Է��ж�����\n");
                fflush(stdout);
                exit(1);
            }
            total += rd_ret;
            if (total == MAX_PKT)
                break;
        }
        printf("��������֡�ɹ�\n");
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
    ��frame�е�int��ת��Ϊ������
*/
void convert_frame(frame *s)
{
    s->kind = htonl(s->kind);
    s->ack = htonl(s->ack);
    s->seq = htonl(s->seq);
}

/*
    ��receiver�˵�����㷢�����ݰ�
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
            perror("�����������㷢�����ݴ���");
            exit(1);
        }
        //printf("���η���%d\n", len);
    }
    else if (s.kind == ack || s.kind == nak)
    {
        convert_frame(&s);
        len = write(client_socket_desc, &s, 12);
        if (len < 0)
        {
            perror("�����������㷢�����ݴ���");
            exit(1);
        }
        //printf("���η��Ϳ���֡%d�ֽ�\n", len);
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
            perror("�����������㷢�����ݴ���");
            exit(1);
        }
        printf("���η�������֡%d�ֽ�\n", len);
    }
    else if (s.kind == ack || s.kind == nak)
    {
        convert_frame(&s);
        len = write(client_socket_desc, &s, 12);
        if (len < 0)
        {
            perror("�����������㷢�����ݴ���");
            exit(1);
        }
        printf("�ɹ�����ACK\n");
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
            perror("receiver������sender�����������ݳ���\n");
            exit(1);
        }
        if (rd_ret == 0)
        {
            printf("�Է��ж�����\n");
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
        //������1024������
        while (1)
        {
            rd_ret = read(client_socket_desc, &s->info.data[0] + total, MAX_PKT - total);
            if (rd_ret < 0)
            {
                perror("receiver������sender�����������ݳ���\n");
                exit(1);
            }
            if (rd_ret == 0)
            {
                printf("�Է��ж�����\n");
                fflush(stdout);
                exit(1);
            }
            total += rd_ret;
            if (total == MAX_PKT)
                break;
        }
        printf("��������֡�ɹ�\n");
    }
    return;
}


void SPL_from_RPL1(frame *s, int client_socket_desc, char *addr)
{
    fd_set rest;
    /*select��ʱ����*/
    struct timeval tempval;
    // lect�ȴ�ʱ��
    tempval.tv_sec = 0;
    // lect�ȴ�����
    tempval.tv_usec = 0;
    // lect�ȴ�������

    char frame_header[12];
    int rd_ret;
    int total = 0;

    while (1)
    {
        //�Ѽ����׽��ַ����������
        FD_ZERO(&rest); //��ն�������
        FD_SET(client_socket_desc, &rest);
        int select_return = select(client_socket_desc + 1, &rest, NULL, NULL, &tempval); //�����׽ӵĿɶ��Ϳ�д����
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
                perror("receiver������sender�����������ݳ���\n");
                exit(1);
            }
            if (rd_ret == 0)
            {
                printf("�Է��ж�����\n");
                fflush(stdout);
                exit(1);
            }
            total += rd_ret;
            //printf("total%d\n", total);
            fflush(stdout);
            if (total == 12)
                break;
        }
        if (addr[MEM_FLAG_ADDR] == Can_Write_Not_Send) //��ʱ���ߴ�������
            return;
    }
    char2frame(frame_header, s);
    print_frame(*s);
    fflush(stdout);
    total = 0;
    if (s->kind == data)
    {
        //������1024������
        while (1)
        {
            rd_ret = read(client_socket_desc, &s->info.data[0] + total, MAX_PKT - total);
            if (rd_ret < 0)
            {
                perror("receiver������sender�����������ݳ���\n");
                exit(1);
            }
            if (rd_ret == 0)
            {
                printf("�Է��ж�����\n");
                fflush(stdout);
                exit(1);
            }
            total += rd_ret;
            if (total == MAX_PKT)
                break;
        }
        printf("��������֡�ɹ�\n");
    }

    return;
}

//�ж���ack�����򽫹����ڴ��Ϊ��д
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
    //��temp_file.share�ж�ȡ���ݵ�d_file.txt
    packet buffer;
    char sharefilename[30] = "temp_file.share";
    char filename[30] = "d_file.txt";
    while (1) //�ж��ļ��Ƿ����
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
        //ÿ�δ�temp_file.share�ж�ȡ1024�ֽڵ�buffer
        RNL_from_RDL(&buffer, fd_share);
        //����ȡȫβ0�����˳�����
        if (memcmp(CMPSTR, buffer.data, sizeof(char) * MAX_PKT) == 0)
        {
            printf("д���ļ������������˳�\n");
            break;
        }
        write(fd, buffer.data, MAX_PKT);
    }
    system("rm -rf temp_file.share");
    close(fd);
}

void RNL_from_RDL(packet *buffer, int fd)
{
    //��temp_file.share�ж�ȡ1024�ֽڵ�buffer
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
