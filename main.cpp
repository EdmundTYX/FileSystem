#include <iostream>
#include "filesys.h"

bitset<DINODE_BLK> bitmap;
struct INode sys_open_file[SYS_OPEN_FILE];   //系统打开文件表
short sys_open_file_count;       //系统打开文件数目
struct User user[USER_NUM];      //用户表
struct User_Mem user_mem[USER_NUM];
struct Group group[GROUP_NUM];   //用户组
int user_count;                  //用户数
int group_count;                 //用户组数
//struct SuperBlock super_block;   //超级块
char disk_buf[DISK_BUF][BLOCK_SIZE]; //磁盘缓冲区
int tag[DISK_BUF];               //每块磁盘缓冲区的tag
DISK_ALLOCATE disk;
unsigned short cur_user;         //当前用户ID
unsigned short umod;             //默认权限码，默认644，目录644+111=755

int main() {
    creat_disk();
    //char _char;
    //short _short;
    //int _int;
    //long _long;
    //time_t _time_t;
    //printf("char %d\n", sizeof(_char));
    //printf("short %d\n", sizeof(_short));
    //printf("int %d\n", sizeof(_int));
    //printf("long %d\n", sizeof(_long));
    //printf("time_t %d\n", sizeof(_time_t));
    return 0;
}