#ifndef FILESYSTEM_FILESYS_H
#define FILESYSTEM_FILESYS_H

#include <bitset>
#include <vector>
#include <ctime>

using namespace std;

#define BLOCK_SIZE 1024     //ÿ���С1KB
#define SYS_OPEN_FILE 40    //ϵͳ���ļ����������
#define DIR_NUM 42          //ÿ��Ŀ¼�����������Ŀ¼�������ļ�����
#define F_N_SIZE 20         //ÿ���ļ�����ռ�ֽ���
#define ADDR_N 10            //ÿ��i�ڵ�ָ��������ַ��ǰ7��ֱ��:7KB����8һ�μ�ַ:256KB����9���μ�ַ:64MB����10���μ�ַ:16GB
#define DINODE_SIZE 64      //ÿ������i�ڵ���ռ�ֽ�
#define DINODE_COUNT 65536  //����i�ڵ�����
#define DINODE_BLK 4096     //���д���i�ڵ㹲ռ4096������飬ǰ8����bitmap��
#define DISK_BLK 131072     //����128K�������,ռ��128MB
#define FILE_BLK 126966     //Ŀ¼������������ 131072-(2+8+4096)
#define NICFREE 50          //�������п��п������������
#define USER_NUM 10         //�û���
#define OPEN_NUM 20         //�û����ļ���
#define GROUP_USER_NUM 5    //ÿ���û���
#define GROUP_NUM 5         //�û�����
#define DISK_BUF 128        //���̻���������
#define SECTOR_4_START 4106 //��������ʼ��� *****4106

/*  --- = 000 = 0
    --x = 001 = 1
    -w- = 010 = 2
    -wx = 011 = 3
    r-- = 100 = 4
    r-x = 101 = 5
    rw- = 110 = 6
    rwx = 111 = 7   */
/* ����i�ڵ� 64B ÿ�����̿�16���ڵ� */
struct DINode {
    unsigned short owner;       //���ļ������ߵ�ID
    unsigned short group;       //���ļ��û����ID
    unsigned short file_type;   //0:�����ļ� 1:Ŀ¼�ļ� 2:�ر��ļ�
    unsigned short mode;        //rwx r-x r-x ������Ȩ��+��Ȩ��+������Ȩ��
    unsigned int addr[ADDR_N];  //�ļ������ַ��ǰ7��ֱ��:7KB����8һ�μ�ַ:256KB����9���μ�ַ:64MB����10���μ�ַ:16GB
    unsigned int block_num;     //�ļ���ʹ�õĴ��̿��ʵ����Ŀ
    unsigned int file_size;     //�ļ���С
    unsigned short link_count;  //�ļ����Ӽ���
    time_t last_time;           //�ϴ��ļ���ȡʱ��
};

/* i�ڵ� */
struct INode {
    unsigned int di_number;     //����i�ڵ���
    char state;                 //״̬��ָʾi�ڵ��Ƿ��������޸�
    unsigned int access_count;  //���ʼ������н��̷���i�ڵ�ʱ��������1
    //�ļ������ļ�ϵͳ���߼��豸��
    //����ָ�룺ָ����������ɢ�ж���
    unsigned short owner;       //���ļ�ӵ���ߵ�ID
    unsigned short group;       //���ļ��û����ID
    unsigned short file_type;   //0:�����ļ� 1:Ŀ¼�ļ� 2:�ر��ļ�
    unsigned short mode;        //��ȡȨ��
    unsigned int addr[ADDR_N];  //�ļ������ַ��ǰ6��ֱ��:6KB����7һ�μ�ַ:256KB����8���μ�ַ:64MB����9���μ�ַ:16GB
    unsigned int block_num;     //�ļ���ʹ�õĴ��̿��ʵ����Ŀ
    unsigned int file_size;     //�ļ�����
    unsigned short link_count;  //�ļ����Ӽ���
    time_t last_time;           //�ϴ��ļ��޸�ʱ��
};

/* �����ļ�Ŀ¼�� һ��24B */
struct SFD {
    char file_name[F_N_SIZE];   //�ļ���
    unsigned int di_number;     //����i�ڵ���
};

/* �û����ļ����� */
struct OFD {
    char file_name[F_N_SIZE];   //�򿪵��ļ���
    unsigned short flag;
    char opf_protect[3];        //������
    unsigned int rw_point;      //��дָ��
    unsigned int inode_number;  //�ڴ�i�ڵ���
};

/* �û� */
struct User {
    char user_name[16];         //�û���
    char password[16];          //�û�����
    unsigned short user_id;     //�û�ID
};

/* �ڴ����û���Ϣ */
struct User_Mem {
    struct OFD OFD[OPEN_NUM];   //�û����ļ���
    unsigned short file_count;  //���ļ���
    struct INode *cur_dir;      //��ǰĿ¼i�ڵ�
    char cwd[100];              //��ǰĿ¼
};

/* �û��� */
struct Group {
    unsigned short user_id[GROUP_USER_NUM]; //ÿ�������5���û�
    unsigned short group_id;     //�û���ID
};

class DISK_ALLOCATE {
public:
    int MAX_BLOCK;
    int BLOCK_MAX_LENGTH;
    //vector<vector<unsigned int>> BLOCK;
    vector<unsigned int> SUPER_BLOCK;

    //demo purpose
    DISK_ALLOCATE();

    ~DISK_ALLOCATE();

    void restore_super_block();

    void store_super_block(int insert_block_num = 1);

    void free_block(int block_num);

    unsigned int allocate_block();

    void show_super_block();

    void free_all();

    void allocate_all();
};


extern bitset<DINODE_BLK> bitmap;
extern struct INode sys_open_file[SYS_OPEN_FILE];   //ϵͳ���ļ���
extern short sys_open_file_count;       //ϵͳ���ļ���Ŀ
extern struct User user[USER_NUM];      //�û��� 340B
extern struct User_Mem user_mem[USER_NUM];
extern struct Group group[GROUP_NUM];   //�û���
extern int user_count;                  //�û���
extern int group_count;                 //�û�����
//extern struct SuperBlock super_block;   //������
extern char disk_buf[DISK_BUF][BLOCK_SIZE]; //���̻�����
extern int tag[DISK_BUF];               //ÿ����̻�������tag
extern class DISK_ALLOCATE disk;

extern unsigned short cur_user;         //��ǰ�û�ID
extern unsigned short umod;             //Ĭ��Ȩ���룬Ĭ��644��Ŀ¼644+111=755

void disk_read(char *buf, int id); //��id���̿�����û��Զ����buf

void disk_write(char *buf, int id);//��buf����д�����̵�id��

void init_buf(); //��ʼ��������

void all_write_back();

void creat_disk();

void store();

void restore();

void init();

void disk_read_d(char *buf, unsigned int id);

void disk_write_d(char *buf, unsigned int id);

void dinode_read(struct DINode &DINode, unsigned int di_number);

void dinode_write(const struct DINode &DINode, unsigned int di_number);

unsigned int creat_directory(char *file_name);

void get_inode(struct INode &INode, const struct DINode &DINode, unsigned int di_number);

void ls();


#endif //FILESYSTEM_FILESYS_H
