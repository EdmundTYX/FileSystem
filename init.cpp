#include "filesys.h"
#include <iostream>
#include <cstring>

// �������̲���ʼ������Ŀ¼
void creat_disk() {
    FILE *fp = fopen("disk", "wb");
    char block[BLOCK_SIZE] = {0};
    // ����һ��128MB�Ŀռ���Ϊ����
    for (int i = 0; i < DISK_BLK; i++)
        fwrite(block, BLOCK_SIZE, 1, fp);
    fclose(fp);
    // ��ʼ��������
    disk.free_all();
    // �½��û�admin
    strcpy(user[0].user_name, "admin");
    strcpy(user[0].password, "admin");
    user[0].user_id = 0;
    user_count++;
    // ��admin��� ����һЩ��Ҫ���ļ�,��Щ�ļ���/ /bin /dev /etc /lib /tmp /user
    // ������Ŀ¼
    struct DINode root = {  // ��Ŀ¼�Ĵ���i�ڵ�
            .owner = 0,     // admin
            .group = 0,     // ��
            .file_type = 1, // Ŀ¼�ļ�
            .mode = 777,    // Ĭ��ȫ��Ȩ��
            .addr = {disk.allocate_block()},    // ��������һ��
            .block_num = 1,
            .file_size = 144,
            .link_count = 0,
            .last_time = time((time_t *) nullptr)
    };
    // ��Ŀ¼�ŵ�ϵͳ���ļ�����
    struct INode root_i{};
    get_inode(root_i, root, 0);
    memcpy(&sys_open_file[0], &root_i, sizeof(root_i));
    sys_open_file_count++;
    // i�ڵ�д�����
    bitmap[0] = true;
    dinode_write(root, 0);
    // ��ǰĿ¼ָ���Ŀ¼
    user_mem[cur_user].cur_dir = &sys_open_file[0];
    strcpy(user_mem[cur_user].cwd, "/");
    // ������Ŀ¼
    creat_directory((char *) "bin");
    creat_directory((char *) "dev");
    creat_directory((char *) "etc");
    creat_directory((char *) "lib");
    creat_directory((char *) "tmp");
    creat_directory((char *) "user");
}

// ���������б�������
void store() {
    char block[BLOCK_SIZE] = {0}, *p = block;
    // �û���
    memcpy(p, user, sizeof(user));
    p += sizeof(user);
    memcpy(p, &user_count, sizeof(user_count));
    p += sizeof(user_count);
    // �û���
    memcpy(p, group, sizeof(group));
    p += sizeof(group);
    memcpy(p, &group_count, sizeof(group_count));
    p += sizeof(group_count);
    // Ĭ��Ȩ����
    memcpy(p, &umod, sizeof(umod));
    // ������
    disk.store_super_block();
    disk_write_d(block, 0);
    // bitmap
    char bitmap_c[DINODE_BLK] = {0};
    p = bitmap_c;
    memcpy(bitmap_c, &bitmap, sizeof(bitmap));
    for (int i = 2; i < 10; i++) {
        disk_write_d(p, i);
        p += sizeof(bitmap);
    }
}

// ���������лָ�����
void restore() {
    char block[BLOCK_SIZE] = {0}, *p = block;
    disk_read_d(block, 0);
    // �û���
    memcpy(user, p, sizeof(user));
    p += sizeof(user);
    memcpy(&user_count, p, sizeof(user_count));
    p += sizeof(user_count);
    // �û���
    memcpy(group, p, sizeof(group));
    p += sizeof(group);
    memcpy(&group_count, p, sizeof(group_count));
    p += sizeof(group_count);
    // Ĭ��Ȩ����
    memcpy(&umod, p, sizeof(umod));
    // ������
    disk.restore_super_block();
    // bitmap
    char bitmap_c[DINODE_BLK] = {0};
    p = bitmap_c;
    for (int i = 2; i < 10; i++) {
        disk_read_d(p, i);
        p += sizeof(bitmap);
    }
    memcpy(&bitmap, bitmap_c, sizeof(bitmap));
}

// ϵͳ��ʼ��
void init() {
    bitmap.reset(); // ��ʼ��bitmap
    init_buf();
    sys_open_file_count = 0;
    user_count = 0;
    group_count = 0;
    cur_user = 0;   // admin
    umod = 644;
    FILE *fp = fopen("disk", "r+b");
    if (!fp) {  // ���û�ҵ�disk
        cout << "��ⲻ�����̣��ؽ���...\n";
        creat_disk();
    } else {
        /* ��ȡ�����е����� */
        restore();
        // ��ȡ��Ŀ¼
        struct DINode root{};
        dinode_read(root, 0);
        get_inode(sys_open_file[0], root, 0);
        sys_open_file_count++;
        user_mem[0].cur_dir = &sys_open_file[0];
        strcpy(user_mem->cwd, "/");
    }
    fclose(fp);
}
