#ifndef FILESYSTEM_FILESYS_H
#define FILESYSTEM_FILESYS_H

#include <bitset>
#include <vector>
#include <ctime>

using namespace std;

//#define DEBUG
#ifdef DEBUG
#define STR_EQUL 4
#else
#define STR_EQUL 0
#endif

#define WIDTH 120
#define HEIGHT 200

#define BLOCK_SIZE 1024     //每块大小1KB
#define SYS_OPEN_FILE 100   //系统打开文件表最大项数
#define DIR_NUM 42          //每个目录所包含的最大目录项数（文件数）
#define F_N_SIZE 20         //每个文件名所占字节数
#define ADDR_N 10           //每个i节点指向的物理地址，前7个直接:7KB，第8一次间址:256KB，第9二次间址:64MB，第10三次间址:16GB
#define DINODE_SIZE 64      //每个磁盘i节点所占字节
#define DINODE_COUNT 65536  //磁盘i节点数量
#define DINODE_BLK 4096     //所有磁盘i节点共占4096个物理块，前8块是bitmap块
#define DISK_BLK 131072     //共有128K个物理块,占用128MB
#define FILE_BLK 126966     //目录及数据区块数 131072-(2+8+4096)
#define NICFREE 50          //超级块中空闲块数组的最大块数
#define USER_NUM 10         //用户数
#define OPEN_NUM 20         //用户打开文件数
#define GROUP_USER_NUM 5    //每组用户数
#define GROUP_NUM 5         //用户组数
#define DISK_BUF 128        //磁盘缓冲区块数
#define SECTOR_4_START 4106 //第四区起始块号 *****4106

/*  --- = 000 = 0
    --x = 001 = 1
    -w- = 010 = 2
    -wx = 011 = 3
    r-- = 100 = 4
    r-x = 101 = 5
    rw- = 110 = 6
    rwx = 111 = 7   */
/* 磁盘i节点 64B 每个磁盘块16个节点 */
struct DINode {
    unsigned short owner;       //该文件所有者的ID
    unsigned short group;       //该文件用户组的ID
    unsigned short file_type;   //0:正规文件 1:目录文件 2:软链接文件
    unsigned short mode;        //rwx r-x r-x 所有者权限+组权限+其他人权限
    unsigned int addr[ADDR_N];  //文件物理地址，前7个直接:7KB，第8一次间址:256KB，第9二次间址:64MB，第10三次间址:16GB
    unsigned int block_num;     //文件所使用的磁盘块的实际数目
    unsigned int file_size;     //文件大小
    unsigned short link_count;  //文件链接计数
    time_t last_time;           //上次文件存取时间
};

/* i节点 */
struct INode {
    unsigned int di_number;     //磁盘i节点编号
    char state;                 //状态，指示i节点是否上锁或被修改
    unsigned int access_count;  //访问计数，有进程访问i节点时，计数加1
    //文件所属文件系统的逻辑设备号
    //链接指针：指向空闲链表和散列队列
    unsigned short owner;       //该文件拥有者的ID
    unsigned short group;       //该文件用户组的ID2
    unsigned short file_type;   //0:正规文件 1:目录文件 2:特别文件
    unsigned short mode;        //存取权限
    unsigned int addr[ADDR_N];  //文件物理地址，前6个直接:6KB，第7一次间址:256KB，第8二次间址:64MB，第9三次间址:16GB
    unsigned int block_num;     //文件所使用的磁盘块的实际数目
    unsigned int file_size;     //文件长度
    unsigned short link_count;  //文件链接计数
    time_t last_time;           //上次文件修改时间
};

/* 符号文件目录项 一项24B */
struct SFD {
    char file_name[F_N_SIZE];   //文件名
    unsigned int di_number;     //磁盘i节点编号
};

/* 用户打开文件表项 */
struct OFD {
    char file_dir[100];         //打开文件的绝对路径
    unsigned short flag;        //被使用标志，flag==0就是被删除了
    char opf_protect[3];        //保护码
    unsigned int rw_point;      //读写指针
    unsigned int inode_number;  //内存i节点编号
};

/* 用户 */
struct User {
    char user_name[16];         //用户名
    char password[16];          //用户密码
    unsigned short user_id;     //用户ID
};

/* 内存中用户信息 */
struct User_Mem {
    struct OFD OFD[OPEN_NUM];   //用户打开文件表
    unsigned short file_count;  //打开文件数
    struct INode *cur_dir;      //当前目录i节点
    char cwd[100];              //当前目录
};

/* 用户组 */
struct Group {
    unsigned short user_id[GROUP_USER_NUM]; //每个组最多5个用户
    unsigned short group_id;     //用户组ID
};

/* 软链接数据 */
struct s_link {
    char file_dir[100];         //文件的绝对路径
    char file_name[F_N_SIZE];   //文件名
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

void disk_read_d(char *buf, unsigned int id);

void disk_write_d(char *buf, unsigned int id);

extern FILE *fp;
extern bitset<DINODE_COUNT> bitmap;
extern struct INode sys_open_file[SYS_OPEN_FILE];   //系统打开文件表
extern short sys_open_file_count;       //系统打开文件数目
extern struct User user[USER_NUM];      //用户表 340B
extern struct User_Mem user_mem[USER_NUM];
extern struct Group group[GROUP_NUM];   //用户组
extern int user_count;                  //用户数
extern int group_count;                 //用户组数
//extern struct SuperBlock super_block;   //超级块
extern char disk_buf[DISK_BUF][BLOCK_SIZE]; //磁盘缓冲区
extern int tag[DISK_BUF];               //每块磁盘缓冲区的tag
extern class DISK_ALLOCATE disk;        //
extern unsigned short cur_user;         //当前用户ID
extern unsigned short umod;             //默认权限码，默认644，目录644+111=755

// disp部分
extern char disp_mem[USER_NUM][HEIGHT][WIDTH];
extern unsigned int next_row_offset[USER_NUM];
extern unsigned int dream[USER_NUM];

// copy部分
extern char clipboard[BLOCK_SIZE];
extern struct DINode i_1;
//extern bool is_clipboard_s_link;

void init_display();

void display(const char *p); //把字符串追加到用户的显存中，从第row行开始

void flush(); //把current_user_id的显存内容刷新到屏幕上

void clear();

// 初始化部分
void creat_disk();

void store();

void restore();

void init();

//界面部分
void welcome();

int menu();

unsigned short changeuser(string name);

void signup();

int checkuser(string str);

int checkpassword(string b, unsigned short n);

char *ChangeStrToChar(string InputString);

// IO部分
void init_buf(); //初始化缓冲区
void disk_read(char *buf, int id); //把id磁盘块读到用户自定义的buf
void disk_write(char *buf, int id);//把buf内容写到磁盘的id块
void all_write_back();

// 系统函数
unsigned int dinode_read(unsigned int di_number);

void dinode_write(unsigned int di_number);

void dinode_create(struct DINode DINode, unsigned int di_number);

void get_inode(struct INode &INode, const struct DINode &DINode, unsigned int di_number);

void get_Dinode(struct DINode &DINode, const struct INode &INode);

bool if_in_group(unsigned short group_id);

bool if_can_r(unsigned int di_number);

bool if_can_w(unsigned int di_number);

bool if_can_x(unsigned int di_number);

void delete_file(unsigned int di_number);

int check_s_link(const char *file_name, string &dir_before, string &dir_after, string &real_file_name);

// 用户指令部分
void ls();

void ls_l();

unsigned int creat_directory(const char *file_name);

void instruct_open(string &dest_file_name);

void instruct_close(string &dest_file_name);

void instruct_cd(string &dest_addr);

void rm(string filename);

void rm_r(string filename);

void rmdir(string filename);

void pwd();

void mv(string filename, string filename_after);

void ln(string filename, string filename_after);

void ln_s(string filename, string filename_after);

void chmod(string filename, unsigned short new_mode);

void chown(string filename, string user_name);

void chgrp(string filename, int group_id);

void ins_umod(unsigned short umod_new);

int touch(string fname);

void read(string filename);

void write(string filename);

void write_a(string filename);

bool delete_bigfile(int first_table_bn, int block_num);

bool write_bigfile(const char *name, const char *file_name);

bool read_bigfile(const char *name);

void ins_vim(char *name);

void show_sof();

void show_uof();

void show_file(string filename);

void format();

void copy(string filename);

void paste(string filename);

#endif //FILESYSTEM_FILESYS_H
