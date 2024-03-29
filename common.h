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

const char *RECEIVER_PORT = "4000";
const char *RECEIVER_IP = "192.168.193.90";

//共享内存标志位
#define MEM_FLAG_ADDR 0
#define MEM_ACK_FLAG_ADDR 1
#define DATA_START_ADDR 2

const char Can_Read = '8';
const char Can_Write = '9';

const char Can_Write_Not_Send = '0';
const char Can_Read_Not_Send = '1';
const char Can_Write_Send = '2';
const char Can_Read_Send = '3';
const char CKSUM_ERROR = '4';

#define MEM_SIZE 1040
#define SDL_SPL_KEYID 1
#define RDL_RPL_KEYID 2
#define RNL_RDL_KEYID 3

#define TIME_LIMIT_SEC 0       //以s为单位
#define TIME_LIMIT_USEC 500000 //以微秒为单位

int MEM_SHMID[4]; //存储共享内存shmid，以便释放

//存储现在正在运行的是协议几
#define PROTOCOL1 1
#define PROTOCOL2 2
#define PROTOCOL3 3
#define PROTOCOL4 4
int current_protocol;

//协议二概率，单位千分之一
#define ACK_DELAY 100 //协议二延时概率 缺省10%
#define DELAY_US 100  //协议二延时时间 单位微妙
//协议三概率，单位千分之一
#define RPL_LOST_PERCENTAGE 30 //协议三接收方CKSUM_ERROR概率 缺省3%
#define RPL_ERRO_PERCENTAGE 30 //协议三接收方CKSUM_ERROR概率 缺省3%
#define SPL_LOST_PERCENTAGE 30 //协议三发送方CKSUM_ERROR概率 缺省3%
#define SPL_ERRO_PERCENTAGE 30 //协议三接收方丢包概率 缺省3%

static int CommShm(int size, int flags, int id);
int DestroyShm(int shmid);
int CreateShm(int size, int id);
int GetShm(int size, int id);

const char CMPSTR[MAX_PKT] = {0};

//变量定义
typedef enum
{
    false,
    true
} boolen; //状态枚举量

boolen sender_network_state = true;
boolen frame_arr = false;    //signal函数全局变量，在读到FRAME_ARRIVAL信号后变为真
boolen sdl_st = false;       //signal函数全局变量，在读到SDL_START信号后变为真
boolen spl_st = false;       //signal函数全局变量，在读到SPL_STSRT信号后变为真
typedef unsigned int seq_nr; //发送序号
typedef struct
{
    unsigned char data[MAX_PKT];

} packet; //数据包，纯数据

typedef enum
{
    data,     //数据包
    ack,      //确认包
    nak       //否定确认包
} frame_kind; //帧类型枚举量

typedef struct
{
    frame_kind kind; //帧类型
    seq_nr seq;      //发送序号
    seq_nr ack;      //接收序号
    packet info;     //数据包
} frame;

//每个算法对应的event_type有所不同
typedef enum
{
    frame_arrival,       //帧到达
    cksum_err,           //校验和错
    timeout,             //发送超时
    network_layer_ready, //网络层就绪
    ack_timeout          //确认包超时
} event_type;            //事件类型枚举量

//协议描述中用到的函数
//阻塞函数，等待事件发生
void wait_for_event(event_type *event);

//-------------------------------------------------------------------
void SNL_to_SDL();
//发送方从网络层得到纯数据包
//void SDL_from_SNL(packet *buffer, int *file_id);
/*发送方向物理层发送帧，帧头尾加FLAG字节、数据中进行字节填充
计算校验和放入帧尾*/
void SDL_to_SPL(frame *s, char *addr, int *cnt_sended_frames);
void SPL_from_SDL(frame *s, char *addr);
void SPL_to_RPL(frame s, int client_socket_desc);
void SPL_from_RPL(frame *s, int client_socket_desc);
void SPL_from_RPL1(frame *s, int client_socket_desc, char *addr);
//--------------------------------------------------------------------
void RPL_from_SPL(frame *s, int client_socket_desc);
void RPL_to_RDL(frame *s, char *addr);
void RPL_to_SPL(frame s, int client_socket_desc);
void RPL_from_RDL(frame *s, char *addr);
/*接收方从物理层取得帧
帧头尾的FLAG字节、数据中的字节填充均已去掉
调用本函数前已验证过校验和，若发生错误则发送cksum_err事件，
因此只有帧正确的情况下会调用本函数*/
void RDL_from_RPL(frame *p, char *addr);
//接受方向网络层发送纯数据包
//去掉帧的类型、发送/确认序号等控制信息
//void RDL_to_RNL(packet *p, char* addr);

//void RNL_from_RDL(packet *buffer, char* addr);
//---------------------------------------------------------------------------

//启动确认包定时器
void start_ack_timer(void);

//停止确认包定时器
void stop_ack_timer(void);

//解除网络层阻塞
//使可以产生新的network_layer_ready事件
void enable_network_layer(void);

//使网络层阻塞
//不再产生新的network_layer_ready事件
void disable_network_layer(void);

/*
    生成1GB的文件
*/
void generate_file(const char *filename);
//加文件锁函数
void lock_set(int fd, int type);
int get_pid(char *s);

//接收端将受到的数据写入文件中
void to_final_file();
void RNL_from_RDL(packet *buffer, int fd);
void RDL_to_RNL(packet *p);

void SDL_from_SNL(packet *buffer, int fd);

void sysLocalTime();
void sysUsecTime();

void init_f_ack(frame *s);

void SDL_from_SPL(char *addr);
void RDL_to_RPL(frame *s, char *addr);
//使k在[0~MAX_SEQ-1]间循环增长
//如果MAX_SEQ=1,则0/1互换
#define MAX_SEQ 1
#define inc(k)       \
    if (k < MAX_SEQ) \
        k = k + 1;   \
    else             \
        k = 0;
