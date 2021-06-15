#include "filesys.h"
#include <iostream>
#include <cstring>
#include <regex>
#include "vim.cpp"

/*!
 * ��ӡ��ǰĿ¼
 */
void ls() {
    char block[BLOCK_SIZE] = {0};
    disk_read(block, (int) user_mem[cur_user].cur_dir->addr[0]);
    struct SFD SFD[DIR_NUM];
    memcpy(SFD, block, sizeof(SFD));
    int count = 0;
    for (auto &index : SFD)
        if (index.di_number != 0) {
            cout << index.file_name << endl;
            display(index.file_name);
            count++;
        }
    cout << "��" << count << "��" << endl;
    string str;
    str = "��" + to_string(count) + "��";
    display(str.c_str());
}

/*!
 * ��ӡ��ǰĿ¼����ϸ��Ϣ
 */
void ls_l() {
    char block[BLOCK_SIZE] = {0};
    disk_read(block, (int) user_mem[cur_user].cur_dir->addr[0]);
    struct SFD SFD[DIR_NUM];
    memcpy(SFD, block, sizeof(SFD));
    cout << "Name  Type  Mode  Block_num  Size  Link_count  Last_time" << endl;
    int count = 0;
    for (auto &index : SFD)
        if (index.di_number != 0) {
            int id = dinode_read(index.di_number);
            cout << index.file_name << "   ";
            cout << sys_open_file[id].file_type << "     ";
            cout << sys_open_file[id].mode << "     ";
            cout << sys_open_file[id].block_num << "        ";
            cout << sys_open_file[id].file_size << "     ";
            cout << sys_open_file[id].link_count << "        ";
            cout << ctime(&sys_open_file[id].last_time);
            dinode_write(index.di_number);
            count++;
        }
    cout << "��" << count << "��" << endl;
}

/*!
 * �ڵ�ǰĿ¼������Ŀ¼
 * @param file_name Ŀ¼��
 * @return ����i�ڵ��ַ��ʧ�ܻ��ӡ��Ϣ������0
 */
unsigned int creat_directory(const char *file_name) {
    if (bitmap.all()) {//����i�ڵ㶼������
        cout << "ʧ�ܣ�����i�ڵ㶼������" << endl;
        display("ʧ�ܣ�����i�ڵ㶼������");
        return 0;
    }
    unsigned short umod_d = umod + 111;
    // ������Ŀ¼�Ĵ���i�ڵ�
    struct DINode new_directory = {
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
    disk_read(block, (int) user_mem[cur_user].cur_dir->addr[0]);
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
        if (strcmp(SFD[i].file_name, file_name) == STR_EQUL) {
            cout << "ʧ�ܣ������ظ�" << endl;
            display("ʧ�ܣ������ظ�");
            return 0;
        }
    }
    // ����������
    for (; i < DIR_NUM; i++) {
        if (strcmp(SFD[i].file_name, file_name) == STR_EQUL) {
            cout << "ʧ�ܣ������ظ�" << endl;
            display("ʧ�ܣ������ظ�");
            return 0;
        }
    }
    if (flag) {
        cout << "ʧ�ܣ�Ŀ¼����" << endl;
        display("ʧ�ܣ�Ŀ¼����");
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
    dinode_create(new_directory, index_i);
    // SFDд�ش���
    memcpy(block, SFD, sizeof(SFD));
    disk_write(block, (int) user_mem[cur_user].cur_dir->addr[0]);
    // �޸ĵ�ǰĿ¼��i�ڵ�
    user_mem[cur_user].cur_dir->state = 'w';
    user_mem[cur_user].cur_dir->last_time = time((time_t *) nullptr);
    user_mem[cur_user].cur_dir->file_size += 24;
    return new_directory.addr[0];
}

void instruct_open(string &dest_file_name) {
    // ������׼������
    string dir_before, dir_after, real_file_name;
    bool s_link_flag = FALSE;
    // ������������ļ�
    if (check_s_link(dest_file_name.c_str(), dir_before, dir_after, real_file_name) == 2) {
        // ��ת��ָ��Ŀ¼
        instruct_cd(dir_after);
        dest_file_name = real_file_name;
        s_link_flag = TRUE;
    }

    //dest_file_name = "file_name_in_folder"

    // Ҫ������ļ��еľ���·��
    string dest_file_dir(user_mem[cur_user].cwd);
    if ((dest_file_dir.back()) != '/') {
        dest_file_dir += "/";
    }
    dest_file_dir += dest_file_name;
    dest_file_dir += "/";

    // ���ж������û��Ƿ�򿪹�dest_file_name
    bool found_in_other_user{false};
    unsigned int dest_file_inode_num{0};
    unsigned int dest_file_dinode_num{0};
    for (auto &each_user : user_mem) {
        for (auto &each_users_OFD : each_user.OFD) {
            if (each_users_OFD.flag == 1 && dest_file_dir == each_users_OFD.file_dir) {
                found_in_other_user = true;
                // ֱ�ӻ��inode_num
                dest_file_inode_num = each_users_OFD.inode_number;
                break;
            }
        }
        if (found_in_other_user == true) { break; }
    }
    // ����ǵ�һ�δ�, ��Ҫ���̻��SFD, ���dinode_num, ���inode_num
    if (found_in_other_user == false) {
        char read_block[BLOCK_SIZE]{0};
        disk_read(read_block, (int) user_mem[cur_user].cur_dir->addr[0]);
        struct SFD cur_dir_SFD[DIR_NUM];
        memcpy(cur_dir_SFD, read_block, sizeof(cur_dir_SFD));

        // ���ļ�������Ŀ¼inode��Ӧblock�д洢��SFD, �ҵ�dinode_num
        for (auto &cur_dir_SFD_iter : cur_dir_SFD) {
            if (cur_dir_SFD_iter.file_name == dest_file_name) {
                dest_file_dinode_num = cur_dir_SFD_iter.di_number;
                break;
            }
        }
        //δ��SFD���ҵ�Ҫ�򿪵��ļ��ľ���·��
        if (found_in_other_user == false && dest_file_dinode_num == 0) {
            cout << "file: " << dest_file_dir << " not found" << endl;
            string s = "file: " + dest_file_dir + " not found";
            display(s.c_str());
            if (s_link_flag) {
                instruct_cd(dir_before);
            }
            return;
        }
        dest_file_inode_num = dinode_read(dest_file_dinode_num);
    }
    struct INode &inode = sys_open_file[dest_file_inode_num];

    //��cd�򿪵Ĳ��Ǹ��ļ�
    if (inode.file_type != 0) {
        cout << "not a file" << endl;
        display("not a file");
        //��ǰĿ¼ͣ���ļ����ڵ��ļ�����
    } else {
        // �û����ļ�����++, ���һ�OFD, �ļ��ķ��ʼ���inode.access_count++
        user_mem[cur_user].file_count++;
        inode.access_count++;
        for (auto &curr_OFD_iter : user_mem[cur_user].OFD) {
            if (curr_OFD_iter.flag == 0) {
                curr_OFD_iter.flag = 1;
                strcpy(curr_OFD_iter.file_dir, dest_file_dir.c_str());
                curr_OFD_iter.inode_number = dest_file_inode_num;
                break;
            }
        }
    }
    if (s_link_flag) {
        instruct_cd(dir_before);
    }
}

void instruct_close(string &dest_file_name) {
    // ������׼������
    string dir_before, dir_after, real_file_name;
    bool s_link_flag = FALSE;
    // ������������ļ�
    if (check_s_link(dest_file_name.c_str(), dir_before, dir_after, real_file_name) == 2) {
        // ��ת��ָ��Ŀ¼
        instruct_cd(dir_after);
        dest_file_name = real_file_name;
        s_link_flag = TRUE;
    }

    string dest_file_dir(user_mem[cur_user].cwd);
    if ((dest_file_dir.back()) != '/') {
        dest_file_dir += "/";
    }
    dest_file_dir += dest_file_name;
    dest_file_dir += "/";

    bool dest_file_name_found{false};
    // ɾ���˳��ļ��ж�Ӧ�û����ļ���OFD��, �û����ļ�����--
    unsigned int dest_file_inode_num;
    // �ں����ڰ�INode.state�ж��Ƿ���Ҫ���ڴ�inodeд�ش���#3,��ɾ��ϵͳ�ļ��򿪱����Ӧ��
    for (auto &OFD_iter : user_mem[cur_user].OFD) {
        if (OFD_iter.flag == 1 && OFD_iter.file_dir == dest_file_dir) {
            dest_file_inode_num = OFD_iter.inode_number;
            // �ж�inode�Ƿ���Ҫд��
            sys_open_file[dest_file_inode_num].access_count--;
            dinode_write(sys_open_file[dest_file_inode_num].di_number);
//            memset(&OFD_iter, 0, sizeof(user_mem[cur_user].OFD[0]));
            OFD_iter.flag = 0;
            dest_file_name_found = true;
            user_mem[cur_user].file_count--;
            break;
        }
    }

    if (s_link_flag) {
        instruct_cd(dir_before);
    }

    if (dest_file_name_found == false) {
        cout << "file: " << dest_file_dir << " not found" << endl;
        string s = "file: " + dest_file_dir + " not found";
        display(s.c_str());
        return;
    }
}

/*!
 * �л���ǰ����Ŀ¼
 * @param dest_addr Ҫ�л���Ŀ��Ŀ¼
 */
/*!
 * �л���ǰ����Ŀ¼
 * @param dest_addr Ҫ�л���Ŀ��Ŀ¼
 */
void instruct_cd(string &dest_addr) {
    // ������׼������
    string dir_before, dir_after, real_file_name;
    bool s_link_flag = false;
    // ������������ļ�
    if (check_s_link(dest_addr.c_str(), dir_before, dir_after, real_file_name) == 2) {
        // ��ת��ָ��Ŀ¼
        instruct_cd(dir_after);
        dest_addr = real_file_name;
        s_link_flag = true;
    }

    vector<string> dest_addr_split_items;
    regex re("[^/]+");
    sregex_iterator end;
    for (sregex_iterator iter(dest_addr.begin(), dest_addr.end(), re); iter != end; iter++) {
        dest_addr_split_items.push_back(iter->str());
    }
    string current_dir = user_mem[cur_user].cwd;
    vector<string> cwd_split_items;

    for (sregex_iterator iter(current_dir.begin(), current_dir.end(), re); iter != end; iter++) {
        cwd_split_items.push_back(iter->str());
    }

    //�ص��ϼ�Ŀ¼
    if (dest_addr[0] == '/') {
        //dest_addr == "/usr/user1/a/b/c/"
        reverse(cwd_split_items.begin(), cwd_split_items.end());    //��תcwd_split_items
        cwd_split_items.push_back("/");     //��Ŀ¼��ʶ
        for (auto cwd_split_item_iter : cwd_split_items) {
            //����/usr/Ϊ��Ŀ¼, �ر��ļ���֪�������û������ļ��е���һ��,����Ŀ¼/usr/
            if (cwd_split_item_iter == "/") { break; }
                //������Ǹ�Ŀ¼�ͷ�����һ��Ŀ¼
            else {
                string s = "../";
                instruct_cd(s);
            }
        }
        //new_dir == "./usr/user1/a/b/c/"
        string new_dir = ".";
        for (auto &dest_addr_split_items_iter: dest_addr_split_items) {
            new_dir += "/";
            new_dir += dest_addr_split_items_iter;
        }
        new_dir += "/";
        //���˺�, ./ == /usr/
        instruct_cd(new_dir);
    } else if (dest_addr_split_items.size() == 1 && dest_addr_split_items[0] == "..") {
        //dest_addr == ../
        if (cwd_split_items.size() == 0) {
            cout << "when in root, previous directory not found. " << endl;
            display("when in root, previous directory not found. ");
            if (s_link_flag) {
                instruct_cd(dir_before);
            }
            return;
        }
        // ɾȥ��ǰĿ¼���ļ���
        cwd_split_items.pop_back();
        string fore_dir;
        // �����µ�,��Ŀ¼�ľ���·��
        for (auto &dir_iter: cwd_split_items) {
            fore_dir += "/";
            fore_dir += dir_iter;
        }
        fore_dir += "/";

        // �ڵ�ǰ�û����ļ��в����ϼ�Ŀ¼��
        string cur_dir = user_mem[cur_user].cwd;
        for (auto &OFD_iter : user_mem[cur_user].OFD) {
            // OFD_iter.file_name Ϊ����·��
            // ȷ���ϼ�Ŀ¼����
            if (OFD_iter.flag == 1 && OFD_iter.file_dir == fore_dir) {
                user_mem[cur_user].cur_dir->access_count--;
                dinode_write(user_mem[cur_user].cur_dir->di_number);
                // ɾ��OFD��Ӧ��
                for (auto &curr_OFD_iter : user_mem[cur_user].OFD) {
                    if (curr_OFD_iter.flag == 1 && cur_dir == curr_OFD_iter.file_dir) {
                        curr_OFD_iter.flag = 0;
                        break;
                    }
                }
                // ���ĵ�ǰ����Ŀ¼, ���ĵ�ǰ����Ŀ¼���ڴ�inodeָ��,
                // ɾ���˳��ļ��ж�Ӧ�û����ļ���OFD��, �û����ļ�����--
                strcpy(user_mem[cur_user].cwd, fore_dir.c_str());
                user_mem[cur_user].cur_dir = &sys_open_file[OFD_iter.inode_number];
                user_mem[cur_user].file_count--;
                break;
            }
        }
    }//�򿪵�ǰĿ¼�µ��ļ�·��,�����л���
    else if (dest_addr_split_items[0] == ".") {
        //dest_addr == "./a/b/c/"
        for (auto &dest_addr_split_items_iter : dest_addr_split_items) {
            if (dest_addr_split_items_iter == ".") { continue; }
            // Ҫ������ļ��еľ���·��
            string new_dir;
            cwd_split_items.push_back(dest_addr_split_items_iter);
            for (auto &cwd_split_items_iter : cwd_split_items) {
                new_dir += "/";
                new_dir += cwd_split_items_iter;
            }
            new_dir += "/";

            bool found_in_other_user{false};
            unsigned int inode_num{0};
            unsigned int cur_dir_open_dinode_num{0};
            for (auto &each_user : user_mem) {
                for (auto &each_users_OFD : each_user.OFD) {
                    if (each_users_OFD.flag == 1 && new_dir == each_users_OFD.file_dir) {
                        found_in_other_user = true;
                        // ֱ�ӻ��inode_num
                        inode_num = each_users_OFD.inode_number;
                        break;
                    }
                }
                if (found_in_other_user == true) { break; }
            }
            // ��һ�δ�, �����̻���ļ�����ӦSFD
            if (found_in_other_user == false) {
                char read_block[BLOCK_SIZE]{0};
                disk_read(read_block, (int) user_mem[cur_user].cur_dir->addr[0]);
                struct SFD cur_dir_SFD[DIR_NUM];
                memcpy(cur_dir_SFD, read_block, sizeof(cur_dir_SFD));

                // ���ļ�������Ŀ¼inode��Ӧblock�д洢��SFD, �ҵ�����ļ���dinode_num
                for (auto &cur_dir_SFD_iter : cur_dir_SFD) {
                    if (cur_dir_SFD_iter.file_name == dest_addr_split_items_iter) {
                        cur_dir_open_dinode_num = cur_dir_SFD_iter.di_number;
                        break;
                    }
                }
                inode_num = dinode_read(cur_dir_open_dinode_num);
            }
            struct INode &inode = sys_open_file[inode_num];
            //δ��SFD���ҵ�Ҫ�򿪵��ļ��ľ���·��
            if (found_in_other_user == false && cur_dir_open_dinode_num == 0) {
                cout << "file: " << new_dir << " not found" << endl;
                string s = "file: " + new_dir + " not found";
                display(s.c_str());
                if (s_link_flag) {
                    instruct_cd(dir_before);
                }
                return;
            }
            //��cd�򿪵��Ǹ��ļ�
            if (inode.file_type != 1) {
                cout << "not a directory" << endl;
                display("not a directory");
                if (s_link_flag) {
                    instruct_cd(dir_before);
                }
                //��ǰĿ¼ͣ���ļ����ڵ��ļ�����
                break;
            } else if (if_can_x(cur_dir_open_dinode_num) == false) {
                cout << "access denied" << endl;
                display("access denied");
                if (s_link_flag) {
                    instruct_cd(dir_before);
                }
                //��ǰĿ¼ͣ���ļ����ڵ��ļ�����
                break;
            } else {
                // ���ĵ�ǰ����Ŀ¼, ���ĵ�ǰ����Ŀ¼���ڴ�inodeָ��,
                // ��������ļ��ж�Ӧ�û����ļ���OFD��, �û����ļ�����++, inode���ü���++
                inode.access_count++;
                strcpy(user_mem[cur_user].cwd, new_dir.c_str());
                user_mem[cur_user].cur_dir = &inode;
                user_mem[cur_user].file_count++;
                for (auto &curr_OFD_iter : user_mem[cur_user].OFD) {
                    if (curr_OFD_iter.flag == 0) {
                        curr_OFD_iter.flag = 1;
                        strcpy(curr_OFD_iter.file_dir, new_dir.c_str());
                        curr_OFD_iter.inode_number = inode_num;
                        break;
                    }
                }
            }
        }
    } else if (dest_addr_split_items.size() == 1 && dest_addr_split_items[0] == "~") {
        //dest_addr == "~/"
        //new_dir == "/home/user_name/"
        string new_dir{};
        new_dir += "/home/";
        new_dir += user[cur_user].user_name;
        new_dir += "/";
        instruct_cd(new_dir);

    } else if (dest_addr_split_items.size() == 1) {
        //dest_addr == "folder_name"
        //new_dir == "./folder_name/"
        string new_dir{};
        new_dir += "./";
        new_dir += dest_addr_split_items[0];
        new_dir += "/";
        instruct_cd(new_dir);
    }
}

/*!
 * ɾ����ǰĿ¼��ָ�����ֵ���ͨ�ļ�
 * @param filename
 */
void rm(string filename) {
    // �ҵ���ǰĿ¼��SFD
    char block[BLOCK_SIZE] = {0};
    disk_read(block, (int) user_mem[cur_user].cur_dir->addr[0]);
    // ����SFD�ṹ��
    struct SFD SFD[DIR_NUM];
    memcpy(SFD, block, sizeof(SFD));
    // Ѱ������ļ���
    for (auto &i:SFD) {
        if (i.di_number != 0)
            if (strcmp(i.file_name, filename.c_str()) == 0) {
                // �ж�Ȩ��
                if (if_can_w(i.di_number)) {
                    // �ж�����
                    int id = dinode_read(i.di_number);
                    if (sys_open_file[id].file_type == 0 or sys_open_file[id].file_type == 2) {
                        delete_file(i.di_number);
                        // �ڵ�ǰĿ¼SFD��ɾ����¼
                        memset(&i, 0, sizeof(i));
                        // д��SFD
                        memcpy(block, SFD, sizeof(SFD));
                        disk_write(block, (int) user_mem[cur_user].cur_dir->addr[0]);
                        user_mem[cur_user].cur_dir->state = 'w';
                        user_mem[cur_user].cur_dir->last_time = time((time_t *) nullptr);
                        user_mem[cur_user].cur_dir->file_size -= 24;
                        return;
                    } else {
                        dinode_write(i.di_number);
                        cout << "ɾ��ʧ�ܣ��޷�ֱ��ɾ��Ŀ¼���볢��ʹ�� -r ����" << endl;
                        display("ɾ��ʧ�ܣ��޷�ֱ��ɾ��Ŀ¼���볢��ʹ�� -r ����");
                        return;
                    }
                } else {
                    cout << "ɾ��ʧ�ܣ���Ȩ��" << endl;
                    display("ɾ��ʧ�ܣ���Ȩ��");
                    return;
                }
            }
    }
    cout << "ɾ��ʧ�ܣ�û�д��ļ�" << endl;
    display("ɾ��ʧ�ܣ�û�д��ļ�");
}

/*!
 * ֱ�ӵݹ�ɾ���ļ���Ŀ¼
 * @param filename
 */
void rm_r(string filename) {
    // �ҵ���ǰĿ¼��SFD
    char block[BLOCK_SIZE] = {0};
    disk_read(block, (int) user_mem[cur_user].cur_dir->addr[0]);
    // ����SFD�ṹ��
    struct SFD SFD[DIR_NUM];
    memcpy(SFD, block, sizeof(SFD));
    // Ѱ������ļ���
    for (auto &i:SFD) {
        if (i.di_number != 0)
            if (strcmp(i.file_name, filename.c_str()) == 0) {
                // �ж�Ȩ��
                if (if_can_w(i.di_number)) {
                    delete_file(i.di_number);
                    // �ڵ�ǰĿ¼SFD��ɾ����¼
                    memset(&i, 0, sizeof(i));
                    // д��SFD
                    memcpy(block, SFD, sizeof(SFD));
                    disk_write(block, (int) user_mem[cur_user].cur_dir->addr[0]);
                    user_mem[cur_user].cur_dir->state = 'w';
                    user_mem[cur_user].cur_dir->last_time = time((time_t *) nullptr);
                    user_mem[cur_user].cur_dir->file_size -= 24;
                    return;
                } else {
                    cout << "ɾ��ʧ�ܣ���Ȩ��" << endl;
                    display("ɾ��ʧ�ܣ���Ȩ��");
                    return;
                }
            }
    }
    cout << "ɾ��ʧ�ܣ�û�д��ļ�" << endl;
    display("ɾ��ʧ�ܣ�û�д��ļ�");
}

/*!
 * ɾ���յ�Ŀ¼
 * @param filename
 */
void rmdir(string filename) {
    // �ҵ���ǰĿ¼��SFD
    char block[BLOCK_SIZE] = {0};
    disk_read(block, (int) user_mem[cur_user].cur_dir->addr[0]);
    // ����SFD�ṹ��
    struct SFD SFD[DIR_NUM];
    memcpy(SFD, block, sizeof(SFD));
    // Ѱ������ļ���
    for (auto &i:SFD) {
        if (i.di_number != 0)
            if (strcmp(i.file_name, filename.c_str()) == 0) {
                // �ж�Ȩ��
                if (if_can_w(i.di_number)) {
                    // �жϿղ���
                    int id = dinode_read(i.di_number);
                    if (sys_open_file[id].file_type == 1) {
                        if (sys_open_file[id].file_size == 0) {
                            delete_file(i.di_number);
                            // �ڵ�ǰĿ¼SFD��ɾ����¼
                            memset(&i, 0, sizeof(i));
                            // д��SFD
                            memcpy(block, SFD, sizeof(SFD));
                            disk_write(block, (int) user_mem[cur_user].cur_dir->addr[0]);
                            // �޸ĵ�ǰ����Ŀ¼��i�ڵ�
                            user_mem[cur_user].cur_dir->state = 'w';
                            user_mem[cur_user].cur_dir->last_time = time((time_t *) nullptr);
                            user_mem[cur_user].cur_dir->file_size -= 24;
                            return;
                        } else {
                            cout << "ɾ��ʧ�ܣ�Ŀ¼�ǿ�" << endl;
                            display("ɾ��ʧ�ܣ�Ŀ¼�ǿ�");
                            return;
                        }
                    } else {
                        cout << "ɾ��ʧ�ܣ�����Ŀ¼" << endl;
                        display("ɾ��ʧ�ܣ�����Ŀ¼");
                        return;
                    }

                } else {
                    cout << "ɾ��ʧ�ܣ���Ȩ��" << endl;
                    display("ɾ��ʧ�ܣ���Ȩ��");
                    return;
                }
            }
    }
    cout << "ɾ��ʧ�ܣ�û�д��ļ�" << endl;
    display("ɾ��ʧ�ܣ�û�д��ļ�");
}

/*!
 * ��ʾ��ǰ·��
 */
void pwd() {
    cout << user_mem[cur_user].cwd << endl;
    display(user_mem[cur_user].cwd);
}

/*!
 * �ı��ļ���
 * @param filename Ҫ�������ļ�
 * @param filename_after ������
 */
void mv(string filename, string filename_after) {
    // �ҵ���ǰĿ¼��SFD
    char block[BLOCK_SIZE] = {0};
    disk_read(block, (int) user_mem[cur_user].cur_dir->addr[0]);
    // ����SFD�ṹ��
    struct SFD SFD[DIR_NUM];
    memcpy(SFD, block, sizeof(SFD));
    // Ѱ������ļ���
    for (auto &i:SFD) {
        if (i.di_number != 0)
            if (strcmp(i.file_name, filename.c_str()) == 0) {
                // �ж�Ȩ��
                if (if_can_w(i.di_number)) {
                    // �ж�Ҫ�ĵ���������û
                    for (auto &j:SFD) {
                        if (j.di_number != 0)
                            if (strcmp(j.file_name, filename_after.c_str()) == 0) {
                                cout << "����ʧ�ܣ���������" << endl;
                                display("����ʧ�ܣ���������");
                            }
                    }
                    // ��ϵͳ�ļ��򿪱�������
                    int inode_number = 0;
                    for (int s = 0; s < SYS_OPEN_FILE; s++)
                        if (sys_open_file[s].di_number == i.di_number) {
                            inode_number = s;
                            break;
                        }
                    // ���ϵͳ�򿪱����У����û��ļ��򿪱����޸�
                    if (inode_number != 0) {
                        for (int u = 0; u < user_count; u++)
                            for (auto &o : user_mem[u].OFD)
                                if (o.inode_number == inode_number) {
                                    string new_dir = user_mem[cur_user].cwd;
                                    new_dir += filename_after;
                                    strcpy(o.file_dir, new_dir.c_str());
                                }
                    }

                    // �ڵ�ǰĿ¼SFD���޸ļ�¼
                    strcpy(i.file_name, filename_after.c_str());
                    // д��SFD
                    memcpy(block, SFD, sizeof(SFD));
                    disk_write(block, (int) user_mem[cur_user].cur_dir->addr[0]);
                    user_mem[cur_user].cur_dir->state = 'w';
                    user_mem[cur_user].cur_dir->last_time = time((time_t *) nullptr);
                    return;
                } else {
                    cout << "����ʧ�ܣ���Ȩ��" << endl;
                    display("����ʧ�ܣ���Ȩ��");
                    return;
                }
            }
    }
    cout << "����ʧ�ܣ�û�д��ļ�" << endl;
    display("����ʧ�ܣ�û�д��ļ�");
}

/*!
 * �ڵ�ǰĿ¼����Ӳ����
 * @param filename
 * @param filename_after
 */
void ln(string filename, string filename_after) {
    // Ҳ����ֻ��SFD��дһ�����i�ڵ㣬���Ӽ���++
    // �ҵ���ǰĿ¼��SFD
    char block[BLOCK_SIZE] = {0};
    disk_read(block, (int) user_mem[cur_user].cur_dir->addr[0]);
    // ����SFD�ṹ��
    struct SFD SFD[DIR_NUM];
    memcpy(SFD, block, sizeof(SFD));
    // Ѱ������ļ���
    for (auto &i:SFD) {
        if (i.di_number != 0)
            if (strcmp(i.file_name, filename.c_str()) == 0) {
                // �ж�Ȩ��
                if (if_can_r(i.di_number)) {
                    // �ж�����
                    // ����ϵͳ�򿪱���
                    int id = dinode_read(i.di_number);
                    if (sys_open_file[id].file_type == 0) {
                        // �ж�Ҫ��������������û
                        for (auto &j:SFD) {
                            if (j.di_number != 0)
                                if (strcmp(j.file_name, filename_after.c_str()) == 0) {
                                    cout << "����ʧ�ܣ���������" << endl;
                                    display("����ʧ�ܣ���������");
                                    dinode_write(i.di_number);
                                    return;
                                }
                        }
                        for (int si = 0; si < DIR_NUM; si++) {
                            // �ҵ�����SFD��
                            if (SFD[si].di_number == 0) {
                                // �ڵ�ǰĿ¼SFD���޸ļ�¼
                                strcpy(SFD[si].file_name, filename_after.c_str());
                                SFD[si].di_number = i.di_number;
                                // д��SFD
                                memcpy(block, SFD, sizeof(SFD));
                                disk_write(block, (int) user_mem[cur_user].cur_dir->addr[0]);
                                // ��ϵͳ�򿪱����޸�,��д��
                                sys_open_file[id].link_count++;
                                sys_open_file[id].state = 'w';
                                user_mem[cur_user].cur_dir->state = 'w';
                                user_mem[cur_user].cur_dir->last_time = time((time_t *) nullptr);
                                user_mem[cur_user].cur_dir->file_size += 24;
                                dinode_write(i.di_number);
                                return;
                            }
                        }
                        dinode_write(i.di_number);
                        cout << "����ʧ�ܣ�Ŀ¼��" << endl;
                        display("����ʧ�ܣ�Ŀ¼��");
                        return;
                    } else {
                        dinode_write(i.di_number);
                        cout << "����ʧ�ܣ��޷���Ŀ¼Ӳ����" << endl;
                        display("����ʧ�ܣ��޷���Ŀ¼Ӳ����");
                    }
                } else {
                    cout << "����ʧ�ܣ���Ȩ��" << endl;
                    display("����ʧ�ܣ���Ȩ��");
                    return;
                }
            }
    }
    cout << "����ʧ�ܣ�û�д��ļ�" << endl;
    display("����ʧ�ܣ�û�д��ļ�");
}

/*!
 * �ڵ�ǰĿ¼����������
 * @param filename
 * @param filename_after
 */
void ln_s(string filename, string filename_after) {
    // �½�һ��i�ڵ㣬����Ϊ2�������洢�����ļ��ľ���·�����ļ���������i�ڵ��
    // �ҵ���ǰĿ¼��SFD
    char block[BLOCK_SIZE] = {0};
    disk_read(block, (int) user_mem[cur_user].cur_dir->addr[0]);
    // ����SFD�ṹ��
    struct SFD SFD[DIR_NUM];
    memcpy(SFD, block, sizeof(SFD));
    // Ѱ������ļ���
    for (auto &i:SFD) {
        if (i.di_number != 0)
            if (strcmp(i.file_name, filename.c_str()) == 0) {
                // �ж�Ȩ��
                if (if_can_r(i.di_number)) {
                    // �ж�Ҫ��������������û
                    for (auto &j:SFD) {
                        if (j.di_number != 0)
                            if (strcmp(j.file_name, filename_after.c_str()) == 0) {
                                cout << "����ʧ�ܣ���������" << endl;
                                display("����ʧ�ܣ���������");
                                dinode_write(i.di_number);
                                return;
                            }
                    }
                    for (int si = 0; si < DIR_NUM; si++) {
                        // �ҵ�����SFD��
                        if (SFD[si].di_number == 0) {
                            // �����������ݣ������µ�i�ڵ�
                            struct s_link s_data{};
                            strcpy(s_data.file_dir, user_mem[cur_user].cwd);
                            strcpy(s_data.file_name, filename.c_str());
                            unsigned short umod_d = umod + 111;
                            struct DINode new_dinode = {
                                    .owner = cur_user,
                                    .group = 0,
                                    .file_type = 2,
                                    .mode = umod_d,
                                    .addr = {disk.allocate_block()},
                                    .block_num = 1,
                                    .file_size = sizeof(s_data),
                                    .link_count = 0,
                                    .last_time = time((time_t *) nullptr)
                            };
                            // д���������ݵ���i�ڵ���������ݿ�
                            char data_block[BLOCK_SIZE] = {0};
                            memcpy(data_block, &s_data, sizeof(s_data));
                            disk_write(data_block, new_dinode.addr[0]);
                            // �ҵ�����i�ڵ�
                            int index_i;
                            for (index_i = 0; index_i < DINODE_COUNT; index_i++)
                                if (bitmap[index_i] == 0) {
                                    // �����µĴ���i�ڵ�
                                    dinode_create(new_dinode, index_i);
                                    bitmap[index_i] = true;
                                    break;
                                }
                            // �ڵ�ǰĿ¼SFD���޸ļ�¼
                            strcpy(SFD[si].file_name, filename_after.c_str());
                            SFD[si].di_number = index_i;
                            // д��SFD
                            memcpy(block, SFD, sizeof(SFD));
                            disk_write(block, (int) user_mem[cur_user].cur_dir->addr[0]);
                            // ��ϵͳ�򿪱����޸�,��д��
                            user_mem[cur_user].cur_dir->state = 'w';
                            user_mem[cur_user].cur_dir->last_time = time((time_t *) nullptr);
                            user_mem[cur_user].cur_dir->file_size += 24;
                            return;
                        }
                    }
                    dinode_write(i.di_number);
                    cout << "����ʧ�ܣ�Ŀ¼��" << endl;
                    display("����ʧ�ܣ�Ŀ¼��");
                    return;

                } else {
                    cout << "����ʧ�ܣ���Ȩ��" << endl;
                    display("����ʧ�ܣ���Ȩ��");
                    return;
                }
            }
    }
    cout << "����ʧ�ܣ�û�д��ļ�" << endl;
    display("����ʧ�ܣ�û�д��ļ�");
}

/*!
 * �޸�ָ���ļ���Ȩ��
 * @param filename �ļ���
 * @param new_mode Ȩ����(000-777)
 */
void chmod(string filename, unsigned short new_mode) {
    // �ҵ���ǰĿ¼��SFD
    if (new_mode < 000 or new_mode > 777) {
        cout << "����Ȩ��ʧ�ܣ�Ȩ�������(000-777)" << endl;
        display("����Ȩ��ʧ�ܣ�Ȩ��������(000-777)");
        return;
    }
    char block[BLOCK_SIZE] = {0};
    disk_read(block, (int) user_mem[cur_user].cur_dir->addr[0]);
    // ����SFD�ṹ��
    struct SFD SFD[DIR_NUM];
    memcpy(SFD, block, sizeof(SFD));
    // Ѱ������ļ���
    for (auto &i:SFD) {
        if (i.di_number != 0)
            if (strcmp(i.file_name, filename.c_str()) == 0) {
                // �ж�Ȩ��
                if (if_can_w(i.di_number)) {
                    // ����ϵͳ�򿪱���
                    int id = dinode_read(i.di_number);
                    sys_open_file[id].mode = new_mode;
                    sys_open_file[id].state = 'w';
                    sys_open_file[id].last_time = time((time_t *) nullptr);
                    dinode_write(i.di_number);
                    return;
                } else {
                    cout << "����Ȩ��ʧ�ܣ���Ȩ�޸���" << endl;
                    display("����Ȩ��ʧ�ܣ���Ȩ�޸���");
                    return;
                }
            }
    }
    cout << "����Ȩ��ʧ�ܣ�û�д��ļ�" << endl;
    display("����Ȩ��ʧ�ܣ�û�д��ļ�");
}

/*!
 * �ı��ļ�ӵ����
 * @param filename
 * @param user_name
 */
void chown(string filename, string user_name) {
    // �ж���û������û�
    int user_id = checkuser(user_name);
    if (user_id == -1) {
        cout << "����ӵ����ʧ�ܣ�û�и��û�" << endl;
        display("����ӵ����ʧ�ܣ�û�и��û�");
        return;
    }
    // �ҵ���ǰĿ¼��SFD
    char block[BLOCK_SIZE] = {0};
    disk_read(block, (int) user_mem[cur_user].cur_dir->addr[0]);
    // ����SFD�ṹ��
    struct SFD SFD[DIR_NUM];
    memcpy(SFD, block, sizeof(SFD));
    // Ѱ������ļ���
    for (auto &i:SFD) {
        if (i.di_number != 0)
            if (strcmp(i.file_name, filename.c_str()) == 0) {
                // �ж�Ȩ��
                if (if_can_w(i.di_number)) {
                    // ����ϵͳ�򿪱���
                    int id = dinode_read(i.di_number);
                    sys_open_file[id].owner = user_id;
                    sys_open_file[id].state = 'w';
                    sys_open_file[id].last_time = time((time_t *) nullptr);
                    dinode_write(i.di_number);
                    return;
                } else {
                    cout << "����ӵ����ʧ�ܣ���Ȩ�޸���" << endl;
                    display("����ӵ����ʧ�ܣ���Ȩ�޸���");
                    return;
                }
            }
    }
    cout << "����ӵ����ʧ�ܣ�û�д��ļ�" << endl;
    display("����ӵ����ʧ�ܣ�û�д��ļ�");
}

/*!
 * �ı��ļ�������
 * @param filename
 * @param group_id
 */
void chgrp(string filename, int group_id) {
    if (group_id < 0 or group_id > GROUP_NUM - 1) {
        cout << "�����û���ʧ�ܣ���Ŵ���" << endl;
        display("�����û���ʧ�ܣ���Ŵ���");
        return;
    }
    // �ҵ���ǰĿ¼��SFD
    char block[BLOCK_SIZE] = {0};
    disk_read(block, (int) user_mem[cur_user].cur_dir->addr[0]);
    // ����SFD�ṹ��
    struct SFD SFD[DIR_NUM];
    memcpy(SFD, block, sizeof(SFD));
    // Ѱ������ļ���
    for (auto &i:SFD) {
        if (i.di_number != 0)
            if (strcmp(i.file_name, filename.c_str()) == 0) {
                // �ж�Ȩ��
                if (if_can_w(i.di_number)) {
                    // ����ϵͳ�򿪱���
                    int id = dinode_read(i.di_number);
                    sys_open_file[id].group = group_id;
                    sys_open_file[id].state = 'w';
                    sys_open_file[id].last_time = time((time_t *) nullptr);
                    dinode_write(i.di_number);
                    return;
                } else {
                    cout << "�����û���ʧ�ܣ���Ȩ�޸���" << endl;
                    display("�����û���ʧ�ܣ���Ȩ�޸���");
                    return;
                }
            }
    }
    cout << "�����û���ʧ�ܣ�û�д��ļ�" << endl;
    display("�����û���ʧ�ܣ�û�д��ļ�");
}

/*!
 * �����ļ�����Ȩ���룬���ܳ���1/3/5/7
 * @param umod_new
 */
void ins_umod(unsigned short umod_new) {
    if (umod_new < 0 or umod_new > 777) {
        cout << "���Ĵ���Ȩ����ʧ�ܣ�����Ȩ����" << endl;
        display("���Ĵ���Ȩ����ʧ�ܣ�����Ȩ����");
        return;
    }
    umod = umod_new;
}

/*!
 * ����һ���µ��ļ�
 * @param fname
 * @return ���ļ��Ĵ���i�ڵ��
 */
int touch(string fname) {
    if (bitmap.all()) {//����i�ڵ㶼������
        cout << "ʧ�ܣ�����i�ڵ㶼������" << endl;
        return 0;
    }
    char block[BLOCK_SIZE] = {0};
    disk_read(block, (int) user_mem[cur_user].cur_dir->addr[0]);
    struct SFD SFD[DIR_NUM];
    memcpy(SFD, block, sizeof(SFD)); //���ص�ǰĿ¼
    int count = 0;
    for (auto &index : SFD) {
        if (index.di_number == 0) {
            break;
        }
        if (strcmp(index.file_name, fname.c_str()) == STR_EQUL) {
            cout << "ʧ�ܣ��ļ������ظ�" << endl;
            return 0;
        }
        count++;
    }
    if (count == DIR_NUM) {
        cout << "ʧ�ܣ�Ŀ¼�������޷������ļ�" << endl;
        return 0;
    }
    //��ô��������ų���ʧ�������Ϊ���������ļ�����iNode������iNodeλʾͼ�������SFD����µ�ǰĿ¼INode��file_size
    //��������Ŀ¼�ļ�(SFD��)ֻ��һ���飬�����������block_num
    user_mem[cur_user].cur_dir->file_size += 24;
    struct DINode new_file = {
            .owner = cur_user,
            .group = 0,
            .file_type = 0,
            .mode = umod,
            .addr = {},
            .block_num = 0,  //�˴����ڸս����Ŀ��ļ�����0�����ݿ�
            .file_size = 0,
            .link_count = 0,
            .last_time = time((time_t *) nullptr)
    };
    for (auto &i:new_file.addr)
        i = 0;

    //count->di_number==0 ��ʾcountָ���SFD��һ����Ŀ¼��
    int index_i;
    for (index_i = 0; index_i < DINODE_COUNT; index_i++)
        // �ҵ�����i�ڵ�
        if (bitmap[index_i] == 0) {
            strcpy(SFD[count].file_name, fname.c_str());  //дSFD���ļ���
            SFD[count].di_number = index_i; //дSFD��iNode��
            bitmap[index_i] = true; //iNodeλʾͼ��λ
            break;
        }
    // д����i�ڵ�
    dinode_create(new_file, index_i);
    // SFDд�ش���
    memcpy(block, SFD, sizeof(SFD));
    disk_write(block, (int) user_mem[cur_user].cur_dir->addr[0]);

    return index_i;
}

/*!
 * ��ȡ��ǰĿ¼��ָ�����ֵ���ͨ�ļ�
 * @param filename
 */
void read(string filename) {
    // ������׼������
    string dir_before, dir_after, real_file_name;
    bool s_link_flag = FALSE;
    // ������������ļ�
    if (check_s_link(filename.c_str(), dir_before, dir_after, real_file_name) == 2) {
        // ��ת��ָ��Ŀ¼
        instruct_cd(dir_after);
        filename = real_file_name;
        s_link_flag = TRUE;
    }

    // �ҵ���ǰĿ¼��SFD
    char block[BLOCK_SIZE] = {0};
    disk_read(block, (int) user_mem[cur_user].cur_dir->addr[0]);
    // ����SFD�ṹ��
    struct SFD SFD[DIR_NUM];
    memcpy(SFD, block, sizeof(SFD));
    // Ѱ������ļ���
    string str;
    for (auto &i:SFD) {
        if (i.di_number != 0)
            if (strcmp(i.file_name, filename.c_str()) == 0) {
                // �ж�Ȩ��
                if (if_can_r(i.di_number)) {
                    //�ж��û��Ƿ���ļ�
                    for (auto &j:user_mem[cur_user].OFD) {
                        if (j.flag == 1 and i.di_number == sys_open_file[j.inode_number].di_number) {  // �����иĶ� !!!!!!!!!!!!!!!!!!!!!!!
                            for (auto &k:sys_open_file) {
                                if (k.di_number == i.di_number) {
                                    if (k.block_num == 0) {
                                        cout << endl;
                                        return;
                                    }
                                    char block_1[BLOCK_SIZE] = {0};
                                    disk_read(block_1, (int) k.addr[0]);
                                    for (int c = 0; c < strlen(block_1); c++) {
                                        cout << block_1[c];
                                    }
                                    cout << endl;
                                    if (s_link_flag) {
                                        instruct_cd(dir_before);
                                    }
                                    return;
                                }
                            }
                        }
                    }
                    cout << "�û�δ�򿪸��ļ�,�޷���ȡ" << endl;
                    display("�û�δ�򿪸��ļ�,�޷���ȡ");
                    if (s_link_flag) {
                        instruct_cd(dir_before);
                    }
                    return;

                } else {
                    cout << "��ǰ�û��޶����ļ�Ȩ��" << endl;
                    display("��ǰ�û��޶����ļ�Ȩ��");
                    if (s_link_flag) {
                        instruct_cd(dir_before);
                    }
                    return;
                }
            }
    }
    cout << "��ǰĿ¼��û��" << filename << "�ļ�" << endl;
    str = "��ǰĿ¼��û��";
    str += filename;
    str += "�ļ�";
    display(str.c_str());
    if (s_link_flag) {
        instruct_cd(dir_before);
    }
}

void write(string filename) {
    // ������׼������
    string dir_before, dir_after, real_file_name;
    bool s_link_flag = FALSE;
    // ������������ļ�
    if (check_s_link(filename.c_str(), dir_before, dir_after, real_file_name) == 2) {
        // ��ת��ָ��Ŀ¼
        instruct_cd(dir_after);
        filename = real_file_name;
        s_link_flag = TRUE;
    }

    // �ҵ���ǰĿ¼��SFD
    char block[BLOCK_SIZE] = {0};
    disk_read(block, (int) user_mem[cur_user].cur_dir->addr[0]);
    // ����SFD�ṹ��
    struct SFD SFD[DIR_NUM];
    memcpy(SFD, block, sizeof(SFD));
    // Ѱ������ļ���
    string str;
    for (auto &i:SFD) {
        if (i.di_number != 0)
            if (strcmp(i.file_name, filename.c_str()) == 0) {
                // �ж�Ȩ��
                if (if_can_w(i.di_number)) {
                    //�ж��û��Ƿ���ļ�
                    for (auto &j:user_mem[cur_user].OFD) {
                        if (j.flag == 1 and i.di_number == sys_open_file[j.inode_number].di_number) {  // �����иĶ� !!!!!!!!!!!!!!!!!!!!!!!
                            for (auto &k:sys_open_file) {
                                if (k.di_number == i.di_number) {
                                    char c = '0';
                                    string s;
                                    while (c != ':') {
                                        cin.get(c);
                                        s += c;
                                    }
                                    s.replace(s.length() - 1, 1, "");
                                    char block_1[BLOCK_SIZE] = {0};
                                    strcpy(block_1, s.c_str());
                                    j.rw_point = s.length();
                                    k.file_size = s.length();
                                    if (k.block_num == 0) {//û�з����
                                        k.addr[0] = disk.allocate_block();
                                        k.block_num = 1;
                                    }
                                    disk_write(block_1, (int) k.addr[0]);
                                    if (s_link_flag) {
                                        instruct_cd(dir_before);
                                    }
                                    return;
                                }
                            }
                        }
                    }
                    cout << "�û�δ�򿪸��ļ�,�޷���ȡ" << endl;
                    display("�û�δ�򿪸��ļ�,�޷���ȡ");
                    if (s_link_flag) {
                        instruct_cd(dir_before);
                    }
                    return;

                } else {
                    cout << "��ǰ�û���д���ļ�Ȩ��" << endl;
                    display("��ǰ�û���д���ļ�Ȩ��");
                    if (s_link_flag) {
                        instruct_cd(dir_before);
                    }
                    return;
                }
            }
    }
    cout << "��ǰĿ¼��û��" << filename << "�ļ�";
    str = "��ǰĿ¼��û��";
    str += filename;
    str += "�ļ�";
    display(str.c_str());
    if (s_link_flag) {
        instruct_cd(dir_before);
    }
}

void write_a(string filename) {
    // ������׼������
    string dir_before, dir_after, real_file_name;
    bool s_link_flag = FALSE;
    // ������������ļ�
    if (check_s_link(filename.c_str(), dir_before, dir_after, real_file_name) == 2) {
        // ��ת��ָ��Ŀ¼
        instruct_cd(dir_after);
        filename = real_file_name;
        s_link_flag = TRUE;
    }

    // �ҵ���ǰĿ¼��SFD
    char block[BLOCK_SIZE] = {0};
    disk_read(block, (int) user_mem[cur_user].cur_dir->addr[0]);
    // ����SFD�ṹ��
    struct SFD SFD[DIR_NUM];
    memcpy(SFD, block, sizeof(SFD));
    // Ѱ������ļ���
    string str;
    for (auto &i:SFD) {
        if (i.di_number != 0)
            if (strcmp(i.file_name, filename.c_str()) == 0) {
                // �ж�Ȩ��
                if (if_can_w(i.di_number)) {
                    //�ж��û��Ƿ���ļ�
                    for (auto &j:user_mem[cur_user].OFD) {
                        if (j.flag == 1 and i.di_number == sys_open_file[j.inode_number].di_number) {  // �����иĶ� !!!!!!!!!!!!!!!!!!!!!!!
                            for (auto &k:sys_open_file) {
                                if (k.di_number == i.di_number) {
                                    char c = '0';
                                    string s;
                                    while (c != ':') {
                                        cin.get(c);
                                        s += c;
                                    }
                                    s.replace(s.length() - 1, 1, "");
                                    char block_1[BLOCK_SIZE] = {0};
                                    disk_read(block_1, (int) k.addr[0]);
                                    char *p = block_1 + strlen(block_1);
                                    strcpy(p, s.c_str());
//                                    j.rw_point = strlen(block_1);
                                    k.file_size += s.length();
                                    disk_write(block_1, (int) k.addr[0]);
                                    if (s_link_flag) {
                                        instruct_cd(dir_before);
                                    }
                                    return;
                                }
                            }
                        }
                    }
                    cout << "�û�δ�򿪸��ļ�,�޷���ȡ" << endl;
                    display("�û�δ�򿪸��ļ�,�޷���ȡ");
                    if (s_link_flag) {
                        instruct_cd(dir_before);
                    }
                    return;

                } else {
                    cout << "��ǰ�û���д���ļ�Ȩ��" << endl;
                    display("��ǰ�û���д���ļ�Ȩ��");
                    if (s_link_flag) {
                        instruct_cd(dir_before);
                    }
                    return;
                }
            }
    }
    cout << "��ǰĿ¼��û��" << filename << "�ļ�" << endl;
    str = "��ǰĿ¼��û��";
    str += filename;
    str += "�ļ�";
    display(str.c_str());
    if (s_link_flag) {
        instruct_cd(dir_before);
    }
}

void show_iNode(struct DINode *diNode) {
    cout << "-DINode Info as follows" << endl;
    cout << "block_num:" << diNode->block_num << endl;
    cout << diNode->file_size << " " << diNode->file_type << " " << diNode->owner << endl;
    for (int i = 0; i < 8; i++) {
        cout << diNode->addr[i] << " ";
    }
    cout << endl;

    char block[BLOCK_SIZE] = {0}; //������
    unsigned int first_block_num[BLOCK_SIZE / 4]; //һ��������
    unsigned int second_block_num[BLOCK_SIZE / 4]; //��������������
    //�ȵõ�һ��������,������first_block_num
    disk_read(block, diNode->addr[8]); //addr[8]�Ƕ��μ�ַ
    memcpy(first_block_num, block, sizeof(block));
    int block_num = diNode->block_num;
    int first_bn = (block_num - 1) / 256 + 1; //һ����������
    int second_bn = block_num % 256; //��ɢ�Ķ�����������
    cout << "first thread table are as follows:" << endl;
    for (int i = 0; i < first_bn; i++) {
        cout << first_block_num[i] << " ";
    }
    cout << endl;
    cout << "find twice block num as follows:" << endl;
    for (int i = 0; i < first_bn - 1; i++) {
        disk_read(block, first_block_num[i]); //��ȡһ�������������
        memcpy(second_block_num, block, sizeof(block));
        for (int j = 0; j < 256; j++) { //��Ҫд�Ŀ���
            cout << second_block_num[j] << " ";
        }
    }
    disk_read(block, first_block_num[first_bn - 1]); //��ȡһ�������������
    memcpy(second_block_num, block, sizeof(block));
    for (int j = 0; j < second_bn; j++) { //��Ҫд�Ŀ���
        cout << second_block_num[j] << " ";
    }
    cout << endl;
}

bool delete_bigfile(int first_table_bn, int block_num) {
    unsigned int first_block_num[BLOCK_SIZE / 4]; //һ��������
    unsigned int second_block_num[BLOCK_SIZE / 4]; //��������������
    char block[BLOCK_SIZE] = {0}; //������
    disk_read(block, first_table_bn); //addr[8]�Ƕ��μ�ַ
    memcpy(first_block_num, block, sizeof(block));
    int first_bn = (block_num - 1) / 256 + 1; //һ����������
    int second_bn = block_num % 256; //��ɢ�Ķ�����������
    for (int i = 0; i < first_bn - 1; i++) {
        disk_read(block, first_block_num[i]); //��ȡһ�������������
        memcpy(second_block_num, block, sizeof(block));
        for (int j = 0; j < 256; j++) { //��Ҫд�Ŀ���
            disk.free_block(second_block_num[j]);
        }
        disk.free_block(first_block_num[i]);
    }
    disk_read(block, first_block_num[first_bn - 1]); //��ȡһ�������������
    memcpy(second_block_num, block, sizeof(block));
    for (int j = 0; j < second_bn; j++) { //��Ҫд�Ŀ���
        disk.free_block(second_block_num[j]);
    }
    disk.free_block(first_block_num[first_bn - 1]);
    disk.free_block(first_table_bn);
    return true;
}

/*!
 * ʵ�ְ�ĳ��windowsϵͳ�е����д��ļ�д�뵽һ���½����ļ�����
 * �������д���ļ���size>263KB,С����vimд
 * @param name will be writen into
 * @param file_name will be read from
*/
bool write_bigfile(const char *name, const char *file_name) {
    FILE *p = fopen(file_name, "rb");
    fseek(p, 0, SEEK_END);
    unsigned int size = ftell(p);
    fseek(p, 0, SEEK_SET); //��ָ���Ƶ��ļ���ʼ�������߶�д
    unsigned int blocknum = (size - 1) / BLOCK_SIZE + 1; //��Ҫ�Ŀ���
    if (blocknum <= 263) {
        cout << "too small to write" << endl;
        return false;
    }
    if (blocknum > 64 * 1024) {
        cout << "too large to write" << endl;
        return false;
    }

    char block[BLOCK_SIZE] = {0}; //������
    disk_read(block, (int) user_mem[cur_user].cur_dir->addr[0]); //��Ŀ¼�鵽������
    //Ŀ¼����������SFD
    struct SFD SFD[DIR_NUM];
    memcpy(SFD, block, sizeof(SFD));
    int count = 0;
    for (auto &index : SFD) {
        if (strcmp(index.file_name, name) == 0) {
            cout << "found " << SFD[count].file_name << endl;
            break;
        }
        count++;
    }
    if (count == DIR_NUM) {
        cout << name << " not found" << endl;
        return false;
    }
    int DInode_num = SFD[count].di_number; //���ļ���Ӧ�Ĵ���i�ڵ��
    int DINode_block_num = (DInode_num) / (BLOCK_SIZE / DINODE_SIZE) + 10; //����i�ڵ�Ŷ�Ӧ�Ŀ��
    int offset = DInode_num % (BLOCK_SIZE / DINODE_SIZE); //offset
    disk_read(block, DINode_block_num); //��DINode�鵽������
    struct DINode diNode{};
    memcpy(&diNode, block + offset * DINODE_SIZE, DINODE_SIZE);

    int block_num = diNode.block_num;
    if (block_num > 64 * 1024) {
        cout << "file to be writen is too large" << endl;
        show_iNode(&diNode);
        return false;
    }
    //���������
    unsigned int first_block_num[BLOCK_SIZE / 4]; //һ��������
    unsigned int second_block_num[BLOCK_SIZE / 4]; //��������������
    //�ȵõ�һ��������,������first_block_num
    //���block_num==0,��ô����һ�����ļ�����Ҫ��������һ��һ���������
    if (block_num == 0) {
        cout << "created a first thread table block" << endl;
        diNode.addr[8] = disk.allocate_block();
        if (diNode.addr[8] == 0) {
            show_iNode(&diNode);
            return false;
        }
        cout << "created a second thread table block" << endl;
        disk_read(block, diNode.addr[8]); //addr[8]�Ƕ��μ�ַ
        memcpy(first_block_num, block, sizeof(block));
        first_block_num[0] = disk.allocate_block();
        if (first_block_num[0] == 0) {
            show_iNode(&diNode);
            return false;
        }
    } else { //�ǿգ���ôһ���Ѿ�����һ��������������һ�������������
        disk_read(block, diNode.addr[8]); //addr[8]�Ƕ��μ�ַ
        memcpy(first_block_num, block, sizeof(block));
    }

    int first_bn = (block_num - 1) / 256 + 1; //һ����������
    int second_bn = block_num % 256; //��ɢ�Ķ�����������
    if (block_num < blocknum) { //Ŀǰ�еĿ�С������ģ��Ǿ��ٷ���һЩ��
        //2021-6-1
        /*
         * ע��������Ҫ���μ�ַ���ڶ���Ŀ¼����֮���һ��Ŀ¼���ټ�һ��
         * ��vimһ�μ�ַ��������
         * ÿд��һ��Ҫ�����д�ش���
         */
        //����һ�������������������������Ȼ�������Ҫ���ӵĿ���
        cout << "block_num<blocknum" << endl;
        int extra = blocknum - block_num; //��Ҫ���ӵĿ���
        cout << "number of blocks need to be add:" << extra << endl;
        //������֮ǰ���ڵ����һ��һ���������Ӷ�����
        //ָ�����������
        disk_read(block, first_block_num[first_bn - 1]);
        memcpy(second_block_num, block, sizeof(block));
        int right = second_bn + extra <= 256 ? second_bn + extra : 256;
        for (int i = second_bn; i < right; i++) {
            second_block_num[i] = disk.allocate_block();
            if (second_block_num[i] == 0) {
                show_iNode(&diNode);
                return false;
            }
        }
        //д���˶�������������һ��д�ش���
        memcpy(block, second_block_num, sizeof(block));

        disk_write(block, first_block_num[first_bn - 1]);

        //�������һ���������-1�飨���һ����һ���ٷ֣�����ֵĶ������飩��������ָ��Ķ������������飬ͬʱ������������ָ���256������������̿�
        right = (extra - (256 - second_bn) - 1) / 256 + 1; //��Ҫ��������һ������
        for (int i = first_bn; i < first_bn + right - 1; i++) {
            //���ȶ���һ����������飬Ȼ����������һ�������д�����
            //ͬʱ�����Ĵ��̺�д��һ��������
            for (int j = 0; j < 256; j++) {
                second_block_num[j] = disk.allocate_block();
                if (second_block_num[j] == 0) {
                    show_iNode(&diNode);
                    return false;
                }
            }
            memcpy(block, second_block_num, sizeof(block));
            int k = disk.allocate_block();
            if (k == 0) {
                show_iNode(&diNode);
                return false;
            }
            disk_write(block, k);
            first_block_num[i] = k;
        }

        //����д���һ����ɢ�Ķ�����
        int rr = (extra - (256 - second_bn)) % 256; //��Ҫ�����һ��һ�������Ķ�������
        if (rr > 0) { //��Ҫ���������ɢ��
            for (int j = 0; j < rr; j++) {
                second_block_num[j] = disk.allocate_block();
                if (second_block_num[j] == 0) {
                    show_iNode(&diNode);
                    return false;
                }
            }
            memcpy(block, second_block_num, sizeof(block));
            int k = disk.allocate_block();
            if (k == 0) {
                show_iNode(&diNode);
                return false;
            }
            disk_write(block, k);
            first_block_num[first_bn + right - 1] = k;
        }
        //��first_block_num tableд�ش��̿�
        memcpy(block, first_block_num, sizeof(block));
        disk_write(block, diNode.addr[8]);
        all_write_back();
    } else if (blocknum < block_num) { //Ŀǰ�еĿ��������ģ��Ǿ��ͷ�һЩ
        int extra = block_num - blocknum;
        //����Ŀǰ�еĿ���������������һ�������ͷ����һ��һ�����Ӧ�ĵ���ɢ������
        disk_read(block, first_block_num[first_bn - 1]); //��ȡ���һ�������������
        memcpy(second_block_num, block, sizeof(block));
        int left = second_bn - extra < 0 ? 0 : second_bn - extra; //second_bn-left�����һ��������������Ҫ�ͷŵ��������
        for (int i = left; i < second_bn; i++) {
            disk.free_block(second_block_num[i]); //�ͷŶ���������Ӧ�������
        }
        if (left == 0) { //��0�鶼�ͷ��ˣ���ô���һ����ҲҪ�ͷ�
            disk.free_block(first_block_num[first_bn - 1]);
        }
        //���濴�Ƿ���Ҫ�ͷ����������һ����
        left = (extra - second_bn) / 256; //��Ҫ�ͷŵ�����һ������
        for (int i = first_bn - 1 - left; i < first_bn - 1; i++) {
            //��ÿ��һ�����Ӧ�����ж�������Ӧ��������ͷţ����ͷŶ�Ӧ�Ķ������
            disk_read(block, first_block_num[i]); //��ȡһ�������������
            memcpy(second_block_num, block, sizeof(block));
            for (int j = 0; j < 256; j++) {
                disk.free_block(second_block_num[j]);
            }
            disk.free_block(first_block_num[i]); //�ͷŶ������
        }
        //���Ƿ���Ҫ�ͷ�һЩ��ɢ��
        //���left<=0,��ô������Ǹ���ɢ��͹��ͷ���,����Ͳ��ó����ͷ�С����ɢ����
        if (left > 0) { //����֮ǰ�Ĵ����ɢ���ͷ����ˣ������ͷ���һЩ����
            disk_read(block, first_block_num[first_bn - 1 - left - 1]); //��ȡһ�������������
            memcpy(second_block_num, block, sizeof(block));
            left = (extra - second_bn) % 256; //����Ҫ�ͷŵ���ɢ����
            for (int j = 256 - left; j < 256; j++) {
                disk.free_block(second_block_num[j]);
            }
            //����ֻ���ͷ��˿飬��������������Ϣδ���ӣ�ͨ���µ�block_num����֪����˱߽磬���Զ��������鲻�ø���д�ش���
        }
        //��first_block_num tableд�ش��̿�
        memcpy(block, first_block_num, sizeof(block));
        disk_write(block, diNode.addr[8]);
    }
    //���̿��������
    diNode.block_num = blocknum; //���¿���
    disk_read(block, DINode_block_num);
    memcpy(block + offset * DINODE_SIZE, &diNode, DINODE_SIZE);
    disk_write(block, DINode_block_num); //�Ѹ��¹��Ĵ���INodeд�ش���
    show_iNode(&diNode); //�������I�ڵ���Ϣ


    //���濪ʼд���ݿ�

    cout << "start to write data blocks" << endl;
    //����ֻ��һ��Ѱַ��ʽ�����μ�ַ
    first_bn = (blocknum - 1) / 256 + 1;
    second_bn = blocknum % 256;
    for (int i = 0; i < first_bn - 1; i++) { //����
        disk_read(block, first_block_num[i]); //��ȡһ�������������
        memcpy(second_block_num, block, sizeof(block));
        for (int j = 0; j < 256; j++) { //��Ҫд�Ŀ���
            fseek(p, (i * 256 + j) * BLOCK_SIZE, 0);
            fread(block, BLOCK_SIZE, 1, p);
            disk_write(block, second_block_num[j]);
        }
    }
    disk_read(block, first_block_num[first_bn - 1]); //��ȡһ�������������
    memcpy(second_block_num, block, sizeof(block));
    for (int j = 0; j < second_bn; j++) { //��Ҫд�Ŀ���
        fseek(p, (first_bn - 1 * 256 + j) * BLOCK_SIZE, 0);
        fread(block, BLOCK_SIZE, 1, p);
        disk_write(block, second_block_num[j]);
    }
    cout << "write_bigfile ended" << endl;

    fclose(p); //�رմ򿪵�filename�ļ�ָ��
    return true;
}

//�Ѵ��ļ��Ĵ��̿��������������Ӧ������
bool read_bigfile(const char *name) {
    char block[BLOCK_SIZE] = {0}; //������
    disk_read(block, (int) user_mem[cur_user].cur_dir->addr[0]); //��Ŀ¼�鵽������
    //Ŀ¼����������SFD
    struct SFD SFD[DIR_NUM];
    memcpy(SFD, block, sizeof(SFD));
    int count = 0;
    for (auto &index : SFD) {
        if (strcmp(index.file_name, name) == 0) {
            cout << "found " << SFD[count].file_name << endl;
            break;
        }
        count++;
    }
    if (count == DIR_NUM) {
        cout << name << " not found" << endl;
        return false;
    }
    int DInode_num = SFD[count].di_number; //���ļ���Ӧ�Ĵ���i�ڵ��
    cout << DInode_num << endl;
    int DINode_block_num = (DInode_num) / (BLOCK_SIZE / DINODE_SIZE) + 10; //����i�ڵ�Ŷ�Ӧ�Ŀ��
    int offset = DInode_num % (BLOCK_SIZE / DINODE_SIZE); //offset
    struct DINode diNode{};
    disk_read(block, DINode_block_num); //��DINode�鵽������
    memcpy(&diNode, block + offset * DINODE_SIZE, DINODE_SIZE);
    show_iNode(&diNode);
    return true;
}

void ins_vim(char *name) {
    // ������׼������
    string dir_before, dir_after, real_file_name;
    bool s_link_flag = FALSE;
    // ������������ļ�
    if (check_s_link(name, dir_before, dir_after, real_file_name) == 2) {
        // ��ת��ָ��Ŀ¼
        instruct_cd(dir_after);
        strcpy(name, real_file_name.c_str());
        s_link_flag = TRUE;
    }

    char block[BLOCK_SIZE] = {0}; //������

    disk_read(block, (int) user_mem[cur_user].cur_dir->addr[0]); //��Ŀ¼�鵽������
    //Ŀ¼����������SFD
    struct SFD SFD[DIR_NUM];
    memcpy(SFD, block, sizeof(SFD));
    int count = 0;
    for (auto &index : SFD) {
        if (strcmp(index.file_name, name) == 0) {
            cout << "found " << SFD[count].file_name << endl;
            break;
        }
        count++;
    }
    if (count == DIR_NUM) {
        cout << name << " not found" << endl;
        if (s_link_flag) {
            instruct_cd(dir_before);
        }
        return;
    }
    //countָ����ļ���Ӧ��SFD
    int DInode_num = SFD[count].di_number; //���ļ���Ӧ�Ĵ���i�ڵ��
//    cout<<"DINode_num:"<<DInode_num<<endl;
    int DINode_block_num = (DInode_num) / (BLOCK_SIZE / DINODE_SIZE) + 10; //����i�ڵ�Ŷ�Ӧ�Ŀ��
    int offset = DInode_num % (BLOCK_SIZE / DINODE_SIZE); //offset
//    cout<<"DINode Info:"<<DInode_num<<" "<<DINode_block_num<<" "<<offset<<endl;
    disk_read(block, DINode_block_num); //��DINode�鵽������
//    cout<<"block info:"<<endl;
//    for(char c:block)
//        cout<<c;
//    cout<<endl;
    struct DINode DiNode[BLOCK_SIZE / DINODE_SIZE];
//    p+=DINODE_SIZE*offset;
    memcpy(DiNode, block, sizeof(block));
    struct DINode *diNode = &DiNode[offset];
    int block_num = diNode->block_num;
//    cout<<"has read diNode of "<<name<<endl;
//    show_iNode(diNode);

    if (block_num > 263) {
        cout << "too large to open" << endl;
        if (s_link_flag) {
            instruct_cd(dir_before);
        }
        return;
    }
    Vim vi;
//    cout<<"has created vim"<<endl;

    if (block_num <= 7) {
        for (int i = 0; i < block_num; i++) {
            disk_read(block, diNode->addr[i]);
            vi.load(block);
        }
    } else {
        disk_read(block, diNode->addr[7]); //addr[7]��һ��������
        unsigned int second_block_num[BLOCK_SIZE / 4];
        memcpy(second_block_num, block, sizeof(block));
        char block1[BLOCK_SIZE] = {0}; //������
        for (int i = 0; i < block_num - 7; i++) {
            disk_read(block1, second_block_num[i]); //�������������е�i���Ӧ�������
            vi.load(block1);
        }
    }
//    cout<<"load file succed"<<endl;

//    vi.flus();
//    cout<<"start to loop"<<endl;
    int k = vi.loop();
//    cout<<"loop ended"<<endl;

    if (k == 0) { //k����0����Ҫ���б���
        int len = vi.txt.size();
//        cout<<"txt size:"<<len<<endl;

        if (len == 0) {
            if (s_link_flag) {
                instruct_cd(dir_before);
            }
            return;
        }
        int blocknum = (len - 1) / 16 + 1;
//        cout<<"block_num:"<<block_num<<endl;
//        cout<<"blocknum:"<<blocknum<<endl;
        if (blocknum > block_num) { //��ʾ֮ǰ�Ŀ鲻����
            //��Ϊ��Ҫ�ټ�һЩ�飬�����ȿ��ܷ���һ����������Ͽ飬������ܾ����ˣ����ж���������
            if (block_num < 7) { //��ʾ֮ǰ�Ѿ��õ�һ��������<7�����п���һ��������
                int max = blocknum <= 7 ? blocknum : 7; //��һ�����������д7��
                for (int i = block_num; i < max; i++) {
                    diNode->addr[i] = disk.allocate_block();
                }
            }
            if (blocknum > 7) { //�����Ƿ���Ҫ�ڶ����������еĿ� һ��������֧��7�飬����7�鲻��Ҫ��һ�������������꣬ͬʱ��ҪҪ����������
                if (block_num <= 7) { //����֮ǰ���ļ�ֻ�õ���һ�������飬û�õ����������飬������Ҫ�����յĶ����������
                    unsigned int second_block_num[BLOCK_SIZE / 4];
                    for (int i = 0; i < BLOCK_SIZE / 4; i++) {
                        second_block_num[i] = -1;
                    }
                    memcpy(block, second_block_num, sizeof(block));
                    diNode->addr[7] = disk.allocate_block();
                    disk_write(block, diNode->addr[7]); //�Ѷ����������д����̿�
                }
                //����һ�����ж�����������
                disk_read(block, diNode->addr[7]); //addr[7]ָ�����������
                unsigned int second_block_num[BLOCK_SIZE / 4];
                memcpy(second_block_num, block, sizeof(block));
                int base = block_num > 7 ? block_num - 7 : 0; //baseָ��������������¸�Ҫд�Ŀ�
                for (int i = base; i < blocknum - 7; i++) { //i�Ƕ�����������
                    second_block_num[i] = disk.allocate_block(); //���������Ŀ�
                }
                memcpy(block, second_block_num, sizeof(block));
                disk_write(block, diNode->addr[7]); //����������д��
            }
        } else if (blocknum < block_num) { //����֮ǰ�Ŀ����
            //�ȿ��ڶ����������ܷ��ͷţ�Ȼ���ٿ�һ���������費��Ҫ�ͷ�
            if (block_num > 7) { //��ʾ֮ǰ���ļ��õ��˶��������еĿ飬��Ҫ�ͷŲ��ֻ�ȫ�����������еĿ�
                if (blocknum <= 7) { //��ʾĿǰ��Ҫ�Ŀ���һ��������Ϳ����㣬��ôֱ�ӰѶ����������ͷż���
                    disk.free_block(diNode->addr[7]);
                    diNode->addr[7] = 0;
                } else { //��ôĿǰ����Ҫ���������еĿ飬�����޸Ķ����������ɾ������Ҫ�Ķ���������
                    disk_read(block, diNode->addr[7]); //addr[7]ָ������������
                    unsigned int second_block_num[BLOCK_SIZE / 4];
                    memcpy(second_block_num, block, sizeof(block));
                    int base = blocknum > 7 ? blocknum - 7 : 0; //baseָ������������в���Ҫ�Ŀ�������
                    for (int i = base; i < block_num - 7; i++) { //i�Ƕ����������飬��base��block_num-7-1���ͷ�
                        disk.free_block(second_block_num[i]);
                        second_block_num[i] = -1;
                    }
                    memcpy(block, second_block_num, sizeof(block));
                    disk_write(block, diNode->addr[7]); //����������д��
                }
            }
            //��һ����������Ҫ�ͷŵľ�����blocknum��block_num->7| ֮��Ŀ�
            if (blocknum < 7) { //��ʾĿǰ��Ҫ�Ŀ���<7,��ô��Ҫ�ͷŲ��õ�һ��������
                int max = block_num <= 7 ? blocknum : 7;
                for (int i = blocknum; i < max; i++) { //�ͷŲ���Ҫ��һ��������
                    disk.free_block(diNode->addr[i]);
                    diNode->addr[i] = 0;
                }
            }
        }
        //Ŀǰ�Ѿ������˶�Ӧ������飬ʹ��Ŀǰ�е������������Ҫд�����������ͬ
        diNode->block_num = blocknum; //���¿���
//        show_iNode(diNode);
        //����Ҫ��vectorд���Ѿ������õ��������
        //��дһ��������
        int max = blocknum <= 7 ? blocknum : 7;
        vi.writep = 0; //��ʼ��vi��дָ��(int����)
//        cout<<"start to write vector"<<endl;
        for (int i = 0; i < max; i++) {
            vi.write(diNode->addr[i], i == blocknum - 1);
        }
//        cout<<"vector has been writen"<<endl;
        //���濴�Ƿ���Ҫд����������
        if (blocknum > 7) { //��Ҫд����������
            disk_read(block, diNode->addr[7]); //addr[7]ָ������������
            unsigned int second_block_num[BLOCK_SIZE / 4];
            memcpy(second_block_num, block, sizeof(block));

            for (int i = 0; i < blocknum - 8; i++) { //i��Ҫд�Ķ���������
                vi.write(second_block_num[i], false);
            }
            vi.write(second_block_num[blocknum - 8], true);
        }

        memcpy(block, DiNode, sizeof(block));
//        cout<<"write DiNode to block"<<DINode_block_num<<endl;
        disk_write(block, DINode_block_num);
//        all_write_back();
    }
//    cout<<"ended"<<endl;
    if (s_link_flag) {
        instruct_cd(dir_before);
    }
}

void show_sof() {
    cout << endl;
    for (int id = 1; id < SYS_OPEN_FILE; id++) {
        if (sys_open_file[id].di_number != 0) {
            cout << "i_number: " << id << endl;
            cout << "di_number: " << sys_open_file[id].di_number << endl;
            cout << "state: " << sys_open_file[id].state << endl;
            cout << "access_count: " << sys_open_file[id].access_count << endl;
            cout << "owner: " << sys_open_file[id].owner << endl;
            cout << "group: " << sys_open_file[id].group << endl;
            cout << "file_type: " << sys_open_file[id].file_type << endl;
            cout << "mode: " << sys_open_file[id].mode << endl;
            cout << "block_num: " << sys_open_file[id].block_num << endl;
            cout << "file_size: " << sys_open_file[id].file_size << endl;
            cout << "link_count: " << sys_open_file[id].link_count << endl;
            cout << endl;
        }
    }
}

void show_uof() {
    cout << endl;
    for (int id = 1; id < OPEN_NUM; id++) {
        if (strlen(user_mem[cur_user].OFD[id].file_dir) != 0) {
            cout << "No: " << id << endl;
            cout << "file_dir: " << user_mem[cur_user].OFD[id].file_dir << endl;
            cout << "flag: " << user_mem[cur_user].OFD[id].flag << endl;
            cout << "inode_number: " << user_mem[cur_user].OFD[id].inode_number << endl;
            cout << endl;
        }
    }
}

void show_file(string filename) {
    // �ҵ���ǰĿ¼��SFD
    char block[BLOCK_SIZE] = {0};
    disk_read(block, (int) user_mem[cur_user].cur_dir->addr[0]);
    // ����SFD�ṹ��
    struct SFD SFD[DIR_NUM];
    memcpy(SFD, block, sizeof(SFD));
    // Ѱ������ļ���
    string str;
    for (auto &i:SFD) {
        if (i.di_number != 0)
            if (strcmp(i.file_name, filename.c_str()) == 0) {
                cout << endl;
                int id = dinode_read(i.di_number);
                cout << "di_number: " << sys_open_file[id].di_number << endl;
                cout << "access_count: " << sys_open_file[id].access_count << endl;
                cout << "owner: " << sys_open_file[id].owner << endl;
                cout << "group: " << sys_open_file[id].group << endl;
                cout << "file_type: " << sys_open_file[id].file_type << endl;
                cout << "mode: " << sys_open_file[id].mode << endl;
                cout << "block_num: " << sys_open_file[id].block_num << endl;
                cout << "file_size: " << sys_open_file[id].file_size << endl;
                cout << "link_count: " << sys_open_file[id].link_count << endl;
                cout << "last_time: " << ctime(&sys_open_file[id].last_time) << endl;
                dinode_write(i.di_number);
                return;
            }
    }
}

void format() {
    if (cur_user != 0) {
        cout << "��Ȩ��" << endl;
        return;
    }
    string s_root = "/";
    instruct_cd(s_root);
    // ���ϵͳ�ļ��򿪱�
    memset(sys_open_file, 0, sizeof(sys_open_file));
    sys_open_file_count = 0;
    // ����û����ļ���
    memset(user_mem, 0, sizeof(user_mem));
    // ����û���
    memset(user, 0, sizeof(user));
    user_count = 0;
    // �½��û�admin
    strcpy(user[0].user_name, "admin");
    strcpy(user[0].password, "admin");
    user[0].user_id = 0;
    user_count++;

    char block[BLOCK_SIZE] = {0};
    // ����һ��128MB�Ŀռ���Ϊ����
    for (int i = 0; i < DISK_BLK - 1; i++)
        disk_write(block, i);
    fclose(fp);
    fp = fopen("disk", "rb+");
    // ��ʼ��������
    disk.free_all();
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
    // �û����ļ����һ����Զ�Ǹ�Ŀ¼
    strcpy(user_mem[0].OFD[0].file_dir, "/");
    user_mem[0].OFD[0].flag = 1;
    user_mem[0].OFD[0].rw_point = 0;
    user_mem[0].OFD[0].inode_number = 0;
    user_mem[0].file_count = 1;
    user_mem[0].cur_dir = &sys_open_file[0];
    strcpy(user_mem[0].cwd, "/");
    // i�ڵ�д�����
    bitmap[0] = true;
    dinode_create(root, 0);
    // ������Ŀ¼
    creat_directory((char *) "bin");
    creat_directory((char *) "dev");
    creat_directory((char *) "etc");
    creat_directory((char *) "home");
    creat_directory((char *) "lib");
    creat_directory((char *) "tmp");
    creat_directory((char *) "user");
    string s = "home";
    instruct_cd(s);
    creat_directory((char *) "admin");
    s = "admin";
    instruct_cd(s);
    int a_di = touch("a.txt");
    int a_i = (int) dinode_read(a_di);
    string txt = "Hello World!\n";
    sys_open_file[a_i].addr[0] = disk.allocate_block();
    sys_open_file[a_i].block_num = 1;
    sys_open_file[a_i].file_size = txt.length();
    memcpy(block, txt.c_str(), txt.length());
    disk_write(block, (int) sys_open_file[a_i].addr[0]);
    dinode_write(a_di);
    s = "/";
    instruct_cd(s);
    string s_ = "~";
    instruct_cd(s_);
}

/*!
 * ���Ƶ�ǰĿ¼��ָ�����ֵ���ͨ�ļ�
 * @param filename
 */
void copy(string filename) {
    // ������׼������
    string dir_before, dir_after, real_file_name;
    bool s_link_flag = FALSE;
    // ������������ļ�
    if (check_s_link(filename.c_str(), dir_before, dir_after, real_file_name) == 2) {
        // ��ת��ָ��Ŀ¼
        s_link_flag = TRUE;
    }

    // �ҵ���ǰĿ¼��SFD
    char block[BLOCK_SIZE] = {0};
    disk_read(block, (int) user_mem[cur_user].cur_dir->addr[0]);
    // ����SFD�ṹ��
    struct SFD SFD[DIR_NUM];
    memcpy(SFD, block, sizeof(SFD));
    // Ѱ������ļ���
    string str;
    for (auto &i:SFD) {
        if (i.di_number != 0)
            if (strcmp(i.file_name, filename.c_str()) == 0) {
                for (auto &p:sys_open_file)
                    //�ж���Ŀ¼�ļ����������ļ�
                    if (p.di_number == i.di_number) {
                        if (p.file_type == 1) {
                            cout << "Ҫ���Ƶ��ļ���Ŀ¼�ļ����޷����ƣ�" << endl;
                            return;
                        }
                        if (p.file_type == 0) {
                            //�ж�Ȩ��
                            if (if_can_r(i.di_number)) {
                                //�ж��û��Ƿ���ļ�
                                for (auto &j:user_mem[cur_user].OFD) {
                                    if (j.flag == 1 and i.di_number == sys_open_file[j.inode_number].di_number) {  // �����иĶ� !!!!!!!!!!!!!!!!!!!!!!!
                                        disk_read(clipboard, (int) p.addr[0]);
                                        get_Dinode(i_1, p);
//                                        is_clipboard_s_link = false;
                                        cout << "�ļ������Ѹ��Ƶ���������!" << endl;
                                        return;
                                    }
                                }
                            } else {
                                cout << "��ǰ�û��޶����ļ�Ȩ��" << endl;
                                display("��ǰ�û��޶����ļ�Ȩ��");
                                return;
                            }
                        }
                    }

                // ������������ļ�
                if (s_link_flag) {
                    int id_s = dinode_read(i.di_number);
                    disk_read(clipboard, (int) sys_open_file[id_s].addr[0]);
                    get_Dinode(i_1, sys_open_file[id_s]);
//                    is_clipboard_s_link = true;
                    cout << "�ļ������Ѹ��Ƶ���������!" << endl;
                    dinode_write(i.di_number);
                    return;
                }

                cout << "�û�δ�򿪸��ļ�,�޷���ȡ" << endl;
                display("�û�δ�򿪸��ļ�,�޷���ȡ");
                return;
            }
    }
    cout << "��ǰĿ¼��û��" << filename << "�ļ�" << endl;
    str = "��ǰĿ¼��û��";
    str += filename;
    str += "�ļ�";
    display(str.c_str());
}

void paste(string filename) {
    // �ҵ���ǰĿ¼��SFD
    char block[BLOCK_SIZE] = {0};
    disk_read(block, (int) user_mem[cur_user].cur_dir->addr[0]);
    // ����SFD�ṹ��
    struct SFD SFD[DIR_NUM];
    memcpy(SFD, block, sizeof(SFD));
    // Ѱ������ļ���
    for (auto &i:SFD) {
        if (i.di_number != 0)
            if (strcmp(i.file_name, filename.c_str()) == 0) {
                cout << "���󣬵�ǰĿ¼���������ļ���";
                return;
            }
    }
    int id = touch(filename);   //���ļ�����i�ڵ�
    dinode_create(i_1, id);
    disk_write(clipboard, (int) i_1.addr[0]);
    cout << "ճ���ɹ���" << endl;
}
