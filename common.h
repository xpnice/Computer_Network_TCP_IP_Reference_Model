#define MAX_PKT 1024
#define KB 1024

#define ENABLE 38
#define DISABLE 39
#define FRAME_ARRIVAL 35
#define SDL_START 50
#define SPL_START 49

#define WRITE_FINISH 40
#define SHARE_FILE_END 41
#define SIZEOF_FILE 1024 * 1024 * 1024
#define MAX_SHARE_FILES 5010
//unsigned long int SIZEOF_FILE = 1024 * 1024 * 1024 ;

const char *RECEIVER_PORT = "4000";
const char *RECEIVER_IP = "192.168.80.230";

//�����ڴ��־λ
#define MEM_FLAG_ADDR 0
#define DATA_START_ADDR 1
const char Can_Write = '0';
const char Can_Read = '1';
#define MEM_SIZE 1040
#define SDL_SPL_KEYID 1
#define RDL_RPL_KEYID 2
#define RNL_RDL_KEYID 3

int MEM_SHMID[4]; //�洢�����ڴ�shmid���Ա��ͷ�

//�洢�����������е���Э�鼸
#define PROTOCOL1 1
#define PROTOCOL2 2
#define PROTOCOL3 3
#define PROTOCOL4 4
int current_protocol;

static int CommShm(int size, int flags, int id);
int DestroyShm(int shmid);
int CreateShm(int size, int id);
int GetShm(int size, int id);

const char CMPSTR[MAX_PKT] = {0};

//��������
typedef enum
{
    false,
    true
} boolen; //״̬ö����

boolen sender_network_state = true;
boolen frame_arr = false;    //signal����ȫ�ֱ������ڶ���FRAME_ARRIVAL�źź��Ϊ��
boolen sdl_st = false;       //signal����ȫ�ֱ������ڶ���SDL_START�źź��Ϊ��
boolen spl_st = false;       //signal����ȫ�ֱ������ڶ���SPL_STSRT�źź��Ϊ��
typedef unsigned int seq_nr; //�������
typedef struct
{
    unsigned char data[MAX_PKT];

} packet; //���ݰ���������

typedef enum
{
    data,     //���ݰ�
    ack,      //ȷ�ϰ�
    nak       //��ȷ�ϰ�
} frame_kind; //֡����ö����

typedef struct
{
    frame_kind kind; //֡����
    seq_nr seq;      //�������
    seq_nr ack;      //�������
    packet info;     //���ݰ�
} frame;

//ÿ���㷨��Ӧ��event_type������ͬ
typedef enum
{
    frame_arrival,       //֡����
    cksum_err,           //У��ʹ�
    timeout,             //���ͳ�ʱ
    network_layer_ready, //��������
    ack_timeout          //ȷ�ϰ���ʱ
} event_type;            //�¼�����ö����

//Э���������õ��ĺ���
//�����������ȴ��¼�����
void wait_for_event(event_type *event);

//-------------------------------------------------------------------
void SNL_to_SDL();
//���ͷ��������õ������ݰ�
//void SDL_from_SNL(packet *buffer, int *file_id);
/*���ͷ�������㷢��֡��֡ͷβ��FLAG�ֽڡ������н����ֽ����
����У��ͷ���֡β*/
void SDL_to_SPL(frame *s, char *addr, int *cnt_sended_frames);
void SPL_from_SDL(frame *s, char *addr);
void SPL_to_RPL(frame s, int client_socket_desc);
//--------------------------------------------------------------------
void RPL_from_SPL(frame *s, int client_socket_desc);
void RPL_to_RDL(frame *s, char *addr);
void RPL_to_SPL(frame s, int client_socket_desc);
/*���շ��������ȡ��֡
֡ͷβ��FLAG�ֽڡ������е��ֽ�������ȥ��
���ñ�����ǰ����֤��У��ͣ���������������cksum_err�¼���
���ֻ��֡��ȷ������»���ñ�����*/
void RDL_from_RPL(frame *p, char *addr);
//���ܷ�������㷢�ʹ����ݰ�
//ȥ��֡�����͡�����/ȷ����ŵȿ�����Ϣ
//void RDL_to_RNL(packet *p, char* addr);

//void RNL_from_RDL(packet *buffer, char* addr);
//---------------------------------------------------------------------------
//������K֡�Ķ�ʱ��
void start_timer(seq_nr k);

//ֹͣ��K֡�Ķ�ʱ��
void stop_timer(seq_nr k);

//����ȷ�ϰ���ʱ��
void start_ack_timer(void);

//ֹͣȷ�ϰ���ʱ��
void stop_ack_timer(void);

//������������
//ʹ���Բ����µ�network_layer_ready�¼�
void enable_network_layer(void);

//ʹ���������
//���ٲ����µ�network_layer_ready�¼�
void disable_network_layer(void);

/*
    ����1GB���ļ�
*/
void generate_file(const char *filename);
//���ļ�������
void lock_set(int fd, int type);
int get_pid(char *s);

//���ն˽��ܵ�������д���ļ���
//void to_final_file();

//ʹk��[0~MAX_SEQ-1]��ѭ������
//���MAX_SEQ=1,��0/1����
#define inc(k)       \
    if (k < MAX_SEQ) \
        k = k + 1;   \
    else             \
        k = 0;

void to_final_file(int *file_id);
void RNL_from_RDL(packet *buffer, int *file_id, int nfd);
void RDL_to_RNL(packet *p, int *file_id);

void SDL_from_SNL(packet *buffer, int fd);

void sysLocalTime();
void sysUsecTime();