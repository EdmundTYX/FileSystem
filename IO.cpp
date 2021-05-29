#include "filesys.h"
#include <cstdio>
#include <cstring>

void read(char *buf, int k);

void write(const char *buf, int k);

int get_empty();

void read_from(int k, int id);

void write_back(int k);

void init_buf() {
    for (int &i : tag) {
        i = -1;
    }
}

void disk_read(char *buf, int id) { //��id���̿�����û��Զ����buf
    for (int i = 0; i < DISK_BUF; i++) {
        if (tag[i] == id) {
            read(buf, i);
            return;
        }
    }
    //���е������ʾȫ����������û�ҵ���Ӧid��buf��
    int k = get_empty();//���1����п�
    read_from(k, id); //�Ѵ��̵ĵ�id��д�뵽�������ĵ�k��
    tag[k] = id;
    read(buf, k);
}

void disk_write(char *buf, int id) {
    for (int i = 0; i < DISK_BUF; i++) {
        if (tag[i] == id) { //����Ӧ���̿��ڻ������ھ�ֱ��д��������
            write(buf, i); //��buf������д����i�������
        }
    }
    //���е������ʾû�л�����Ӧid����飬��ô���û�1�黺���
    int k = get_empty();
    tag[k] = id;
    write(buf, k);
}

void read(char *buf, int k) { //��k���������ݶ���buf��
    char *p = disk_buf[k];
    memcpy(buf, p, BLOCK_SIZE);
//    for (int i = 0; i < BLOCK_SIZE; i++) {
//        buf[i] = p[i];
//    }
}

void write(const char *buf, int k) { //��buf����д��k�������
    char *p = disk_buf[k];
    memcpy(p, buf, BLOCK_SIZE);
//    for (int i = 0; i < BLOCK_SIZE; i++) {
//        p[i] = buf[i];
//    }
}

void write_back(int k) { //�ѵ�k�����д�ش��̵�tag[k]��,�������򷵻�-1�����򷵻�1
    int id = tag[k];
    if (id < 0)
        return;
    FILE *f = fopen("disk", "r+");
    fseek(f, id * BLOCK_SIZE, 0);
    char *p1 = disk_buf[k];
    fwrite(p1, BLOCK_SIZE, 1, f);
    fclose(f);
}

void read_from(int k, int id) { //��id���̿�����ݶ�����k�������
    FILE *f = fopen("disk", "r");
    fseek(f, id * BLOCK_SIZE, 0);
    char *p1 = disk_buf[k];
    fread(p1, BLOCK_SIZE, 1, f);
    fclose(f);
}

void all_write_back() {
    for (int i = 0; i < DISK_BUF; i++) {
//        write_back(i);
    }
}

int get_empty() {
    //Ŀǰ��ʱȫ���ѵ�0����Ϊ�û���
    write_back(0); //�ѵ�0��д�ش��̵�tag[0]��
    return 0;
}
