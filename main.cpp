#include "filesys.h"
#include <iostream>

bitset<DINODE_BLK> bitmap;
struct INode sys_open_file[SYS_OPEN_FILE];   //ϵͳ���ļ���
short sys_open_file_count;       //ϵͳ���ļ���Ŀ
struct User user[USER_NUM];      //�û���
struct User_Mem user_mem[USER_NUM];
struct Group group[GROUP_NUM];   //�û���
int user_count;                  //�û���
int group_count;                 //�û�����
//struct SuperBlock super_block;   //������
char disk_buf[DISK_BUF][BLOCK_SIZE]; //���̻�����
int tag[DISK_BUF];               //ÿ����̻�������tag
DISK_ALLOCATE disk;
unsigned short cur_user;         //��ǰ�û�ID
unsigned short umod;             //Ĭ��Ȩ���룬Ĭ��644��Ŀ¼644+111=755

int main() {
    init();
//    for (int i = 0; i < 19; i++)
//        creat_directory((char *) "abc");
    ls();
    all_write_back();
    store();
    return 0;
}
