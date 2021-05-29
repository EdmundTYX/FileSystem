#include "filesys.h"
#include <iostream>
#include <cstring>

void disk_read_d(char *buf, unsigned int id) {
    FILE *f = fopen("disk", "rb");
    fseek(f, (long) id * BLOCK_SIZE, 0);
    fread(buf, BLOCK_SIZE, 1, f);
    fclose(f);
}

void disk_write_d(char *buf, unsigned int id) {
    FILE *f = fopen("disk", "r+b");
    fseek(f, (long) id * BLOCK_SIZE, 0);
    fwrite(buf, BLOCK_SIZE, 1, f);
    fclose(f);
}

/*!
 * ������i�ڵ�
 * @param DINode ����i�ڵ���Ŀ
 * @param di_number ����i�ڵ���
 */
void dinode_read(struct DINode &DINode, unsigned int di_number) {
    // ���ݴ���i�ڵ��ţ�������Ӧ�Ĵ��̿����λ�ã���Ϊÿ��洢16������i�ڵ�
    unsigned int real_addr = 10 + di_number / 16;
    unsigned int offset = di_number % 16;
    char block[BLOCK_SIZE] = {0};
    disk_read_d(block, (int) real_addr);
    struct DINode buf[16];
    memcpy(buf, block, BLOCK_SIZE);
    memcpy(&DINode, &buf[offset], DINODE_SIZE);
}

/*!
 * д����i�ڵ�
 * @param DINode ����i�ڵ���Ŀ
 * @param di_number ����i�ڵ���
 */
void dinode_write(const struct DINode &DINode, unsigned int di_number) {
    // ���ݴ���i�ڵ��ţ�������Ӧ�Ĵ��̿����λ�ã���Ϊÿ��洢16������i�ڵ�
    unsigned int real_addr = 10 + di_number / 16;
    unsigned int offset = di_number % 16;
    char block[BLOCK_SIZE] = {0};
    disk_read_d(block, (int) real_addr);
    struct DINode buf[16];
    memcpy(buf, block, BLOCK_SIZE);
    // �޸Ĳ�д��
    memcpy(&buf[offset], &DINode, DINODE_SIZE); // �޸Ľṹ��
    memcpy(block, buf, BLOCK_SIZE); // ��װ�ɴ��̿�
    disk_write_d(block, (int) real_addr);
}

/*!
 * �ڵ�ǰĿ¼������Ŀ¼
 * @param file_name Ŀ¼��
 * @return ����i�ڵ��ַ��ʧ�ܻ��ӡ��Ϣ������0
 */
unsigned int creat_directory(char *file_name) {
    if (bitmap.all()) {//����i�ڵ㶼������
        cout << "ʧ�ܣ�����i�ڵ㶼������" << endl;
        return 0;
    }
    unsigned short umod_d = umod + 111;
    // ������Ŀ¼�Ĵ���i�ڵ�
    struct DINode new_director = {
            .owner = cur_user,
            .group = 0,
            .file_type = 1,
            .mode = umod_d,
            .addr = {disk.allocate_block()},
            .block_num = 1,
            .file_size = 0,
            .link_count = 0,
            .last_time = time((time_t *) nullptr)
    };
    // �ҵ���ǰĿ¼��SFD
    char block[BLOCK_SIZE] = {0};
    disk_read_d(block, (int) user_mem[cur_user].cur_dir->addr[0]);
    // ����SFD�ṹ��
    struct SFD SFD[DIR_NUM];
    memcpy(SFD, block, sizeof(SFD));
    int index, index_i, flag = 1, i;
    // �޸�SFD���ҵ����д���i�ڵ�
    for (i = 0; i < DIR_NUM; i++) {
        // �ҵ�����SFD��
        if (SFD[i].di_number == 0) {
            index = i;
            flag = 0;
            break;
        }
        if (strcmp(SFD[i].file_name, file_name) == 4) {
            cout << "ʧ�ܣ������ظ�" << endl;
            return 0;
        }
    }
    // ����������
    for (; i < DIR_NUM; i++) {
        if (strcmp(SFD[i].file_name, file_name) == 4) {
            cout << "ʧ�ܣ������ظ�" << endl;
            return 0;
        }
    }
    if (flag) {
        cout << "ʧ�ܣ�Ŀ¼����" << endl;
        return 0;
    }
    for (index_i = 0; index_i < DINODE_COUNT; index_i++)
        // �ҵ�����i�ڵ�
        if (bitmap[index_i] == 0) {
            strcpy(SFD[index].file_name, file_name);
            SFD[index].di_number = index_i;
            bitmap[index_i] = true;
            break;
        }
    // д����i�ڵ�
    dinode_write(new_director, index_i);
    // SFDд�ش���
    memcpy(block, SFD, sizeof(SFD));
    disk_write_d(block, (int) user_mem[cur_user].cur_dir->addr[0]);
    return new_director.addr[0];
}

/*!
 * �ɴ���i�ڵ����ɶ�Ӧ���ڴ�i�ڵ�
 * @param INode Ҫ�޸ĵ��ڴ�i�ڵ�
 * @param DINode ����i�ڵ�ָ��
 * @param di_number ����i�ڵ���
 * @return �ڴ�i�ڵ�ָ��
 */
void get_inode(struct INode &INode, const struct DINode &DINode, unsigned int di_number) {
    INode.di_number = di_number;
    INode.state = ' ';
    INode.access_count = 0;
    INode.owner = DINode.owner;
    INode.group = DINode.group;
    INode.file_type = DINode.file_type;
    INode.mode = DINode.mode;
    memcpy(INode.addr, DINode.addr, sizeof(INode.addr));
    INode.block_num = DINode.block_num;
    INode.file_size = DINode.file_size;
    INode.link_count = DINode.link_count;
    INode.last_time = DINode.last_time;
}

void ls() {
    char block[BLOCK_SIZE] = {0};
    disk_read_d(block, (int) user_mem[cur_user].cur_dir->addr[0]);
    struct SFD SFD[DIR_NUM];
    memcpy(SFD, block, sizeof(SFD));
    int count = 0;
    for (auto &index : SFD)
        if (index.di_number != 0) {
            cout << index.file_name << endl;
            count++;
        }
    cout << "��" << count << "��" << endl;
}
