#include "filesys.h"
#include <iostream>
#include <cstring>
#include <windows.h>

int flag_1[USER_NUM] = {0};

void help() {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    cout << "-----------��������˵���------------" << endl;
    cout << "1.ls                        ��ʾ�ļ�Ŀ¼" << endl;
    cout << "2.cd                        Ŀ¼ת��  " << endl;
    cout << "3.pwd                       ��ʾ��ǰ·��" << endl;
    cout << "4.open                      ���ļ�" << endl;
    cout << "5.close                     �ر��ļ�  " << endl;
    cout << "6.read                      ��ȡ�ļ�����" << endl;
    cout << "7.write                     д���ļ�         " << endl;
    cout << "8.touch                     ����һ���µ��ļ� / �ı��ȡʱ��" << endl;
    cout << "9.rm                        ɾ���ļ� " << endl;
    cout << "10.mkdir                    ����Ŀ¼ " << endl;
    cout << "11.rmdir                    ɾ��Ŀ¼   " << endl;
    cout << "12.cp                       �ļ�����" << endl;
    cout << "13.paste                    �ļ�ճ��" << endl;
    cout << "14.mv                       �ı��ļ���" << endl;
    cout << "15.ln                       �����ļ����� " << endl;
    cout << "16.chmod                    �ı��ļ�Ȩ�� " << endl;
    cout << "17.chown                    �ı��ļ�ӵ����" << endl;
    cout << "18.chgrp                    �ı��ļ������� " << endl;
    cout << "19.umod                     �ļ�����Ȩ���� " << endl;
    cout << "20.login                    �����û�       " << endl;
    cout << "21.sof                      չʾϵͳ���ļ���" << endl;
    cout << "22.uof                      չʾ�û����ļ���" << endl;
    cout << "23.show                     չʾ�ļ�������Ϣ   " << endl;
    cout << "24.wbig                     д���ļ�    " << endl;
    cout << "25.rbig                     �����ļ�       " << endl;
    cout << "26.vim                      �ı��༭��    " << endl;
    cout << "27.help                     ����       " << endl;
    cout << "28.format                   ��ʽ��       " << endl;
    cout << "29.cls                      ����      " << endl;
    cout << "30.exit                     �˳�       " << endl;
    display("-----------��������˵���------------");
    display("1.ls                        ��ʾ�ļ�Ŀ¼");
    display("2.cd                        Ŀ¼ת��  ");
    display("3.pwd                       ��ʾ��ǰ·��");
    display("4.open                      ���ļ�");
    display("5.close                     �ر��ļ�  ");
    display("6.read                      ��ȡ�ļ�����");
    display("7.write                     д���ļ�         ");
    display("8.touch                     ����һ���µ��ļ� / �ı��ȡʱ��");
    display("9.rm                        ɾ���ļ� ");
    display("10.mkdir                    ����Ŀ¼ ");
    display("11.rmdir                    ɾ��Ŀ¼   ");
    display("12.cp                       �ļ�����");
    display("13.paste                    �ļ�ճ��");
    display("14.mv                       �ƶ� / �ı��ļ���");
    display("15.ln                       �����ļ����� ");
    display("16.chmod                    �ı��ļ�Ȩ�� ");
    display("17.chown                    �ı��ļ�ӵ����");
    display("18.chgrp                    �ı��ļ������� ");
    display("19.umod                     �ļ�����Ȩ���� ");
    display("20.login                    �����û�       ");
    display("21.sof                      չʾϵͳ���ļ��� ");
    display("22.uof                      չʾ�û����ļ��� ");
    display("23.show                     չʾ�ļ�������Ϣ ");
    display("24.wbig                     д���ļ�        ");
    display("25.rbig                     �����ļ�        ");
    display("26.vim                      �ı��༭��     ");
    display("27.help                     ����       ");
    display("28.format                   ��ʽ��       ");
    display("29.cls                      ����      ");
    display("30.exit                     �˳�       ");
}

void welcome() {
    int returnmenu = 0;
    int flag;
    int id;
    int codecorrect = 0;
    string a;
    string b;
    string c;
    cout << "$�ļ�����ϵͳ";
    getchar();
    init();
    while (returnmenu == 0) {
        cout << "FileSystem:$username:";
        cin >> a;
        id = checkuser(a);
        flag = 0;
        while (id == -1) {
            cout << "���������û���(y)orע��(n)" << endl;
            cin >> c;
            if (c == "y") {
                cout << "FileSystem:$username:";
                cin >> a;
                id = checkuser(a);
            }
            if (c == "n") {
                cout << "$��ע��" << endl;
                signup();
                id = user_count;
                flag = 1;
            }
        }
        //�ҵ��û����������¼
        if (flag == 0) {
            int m = 0;
            while (codecorrect == 0 && m < 3) {
                cout << "FileSystem:$password:";
                cin >> b;
                codecorrect = checkpassword(b, user[id].user_id);//���벻��ȷ��������
                m++;
            }
            codecorrect = 0;//ÿ�γɹ���½�Ժ�codecorrect��0�����µ�½��ʱ�����������������
            if (m < 3) {
                string str1;
                str1 = "userid=" + to_string(user[id].user_id);
                cur_user = user[id].user_id;
                user_mem[cur_user].cur_dir = sys_open_file;
                cout << str1.c_str();
                display(str1.c_str());
                flush();
                string s = "~";
                instruct_cd(s);
                returnmenu = menu();
            } else {
                cout << "�������β���ȷ,ǿ���˳�" << endl;
                system("pause");
                exit(0);
            }
        }
    }
}

int menu() {
    getchar();      //ϴ�ַ�
    help();
    int flag = 0;
    int n;
    int s;
    int k;
    string order;       //�����ַ�
    string name1;       //����1
    string name2;       //����2
    string name3;       //����3
    cout << endl;
    while (flag == 0) {
        string str1 = user[cur_user].user_name;
        str1 += "@FileSystem";
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
        cout << str1;
        str1 += ":";
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        cout << ":";
        str1 += user_mem[cur_user].cwd;
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_BLUE);
        cout << user_mem[cur_user].cwd;
        str1 += "$ ";
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        cout << "$ ";
        display(str1.c_str());
//        flush();

        string str;
        char c;
        while ((c = cin.get()) != '\n') {
            str += c;
        }
        display(str.c_str());
        n = str.find(' ');
        if (n == -1) {
            order = str;
            name1 = "";
            name2 = "";
            name3 = "";
        } else {
            order = str.substr(0, n);
            string str2;
            str2 = str.substr(n + 1, str.length());
            s = str2.find(' ');
            if (s == -1) {
                name1 = str2;
                name2 = "";
                name3 = "";
            } else {
                name1 = str2.substr(0, s);
                string str3;
                str3 = str2.substr(s + 1, str2.length());
                k = str3.find(' ');
                if (k == -1) {
                    name2 = str3;
                    name3 = "";
                } else {
                    name2 = str3.substr(0, k);
                    name3 = str3.substr(k + 1, str3.length());
                }
            }
        }
        if (order == "ls") {     //1
            if (name1.empty())
                ls();
            else if (name1 == "-l") {
                ls_l();
            } else {
                cout << "����������ָ����ls -l?" << endl;
                display("����������ָ����ls -l?");
            }
        } else if (order == "cd") {     //2
            if (name1.empty())
                cout << "ָ���ʽ����!";
            else
                instruct_cd(name1);
        } else if (order == "pwd") {    //3
            pwd();
        } else if (order == "open") {    //4
            if (name1.empty())
                cout << "ָ���ʽ����!";
            else
                instruct_open(name1);
        } else if (order == "close") {     //5
            if (name1.empty())
                cout << "ָ���ʽ����!";
            else
                instruct_close(name1);
        } else if (order == "read") {       //6
            if (name1.empty())
                cout << "ָ���ʽ����!";
            else
                read(name1);
        } else if (order == "write") {      //7
            if (name2.empty()) {
                write(name1);
                getchar();
            } else if (name1 == "-a") {
                write_a(name2);
                getchar();
            } else {
                cout << "����������ָ����-a?" << endl;
                display("����������ָ����-a?");
            }
        } else if (order == "touch") {     //8
            if (name1.empty())
                cout << "ָ���ʽ����!";
            else
                touch(name1);
        } else if (order == "rm") {       //9
            if (name2.empty()) {
                rm(name1);
            } else if (name1 == "-r") {
                rm_r(name2);
            } else {
                cout << "����������ָ����-r?" << endl;
                display("����������ָ����-r?");
            }
        } else if (order == "mkdir") {      //10
            if (name1.empty())
                cout << "ָ���ʽ����!";
            else
                creat_directory(name1.c_str());
        } else if (order == "rmdir") {     //11
            if (name1.empty())
                cout << "ָ���ʽ����!";
            else
                rmdir(name1);
        } else if (order == "cp") {      //12
            if (name1.empty())
                cout << "ָ���ʽ����!";
            else
                copy(name1);
        } else if (order == "paste") {   //13
            if (name1.empty())
                cout << "ָ���ʽ����!";
            else
                paste(name1);
        } else if (order == "mv") {    //14
            if (name2.empty()) {
                cout << "���ܸ�Ϊ����" << endl;
                display("���ܸ�Ϊ����");
            } else {
                mv(name1, name2);
            }
        } else if (order == "ln") {      //15
            if (name3.empty()) {
                ln(name1, name2);
            } else if (name1 == "-s") {
                ln_s(name2, name3);
            } else {
                cout << "����������ָ����-s?" << endl;
                display("����������ָ����-s?");
            }
        } else if (order == "chmod") {     //16
            if (name2.empty())
                cout << "ָ���ʽ����!";
            else
                chmod(name1, stoi(name2));
        } else if (order == "chown") {      //17
            if (name2.empty())
                cout << "ָ���ʽ����!";
            else
                chown(name1, name2);
        } else if (order == "chgrp") {      //18
            if (name2.empty())
                cout << "ָ���ʽ����!";
            else
                chgrp(name1, stoi(name2));
        } else if (order == "umod") {       //19
            if (name1.empty())
                cout << "ָ���ʽ����!";
            else
                ins_umod(stoi(name1));
        } else if (order == "login") {   //20
            if (name1.empty())
                cout << "ָ���ʽ����!";
            else {
                unsigned short j;
                j = changeuser(name1);
                if (j == 999) {
                    cout << ">>�û��л�ʧ��" << endl;
                } else {
                    flag_1[cur_user] = 1;
                    cur_user = j;
                    if (flag_1[cur_user] == 0) {
                        strcpy(user_mem[cur_user].OFD[0].file_dir, "/");
                        user_mem[cur_user].OFD[0].flag = 1;
                        user_mem[cur_user].OFD[0].rw_point = 0;
                        user_mem[cur_user].OFD[0].inode_number = 0;
                        strcpy(user_mem[cur_user].cwd, "/");
                        user_mem[cur_user].cur_dir = &sys_open_file[0];
                        user_mem[cur_user].file_count = 1;
                        string s_ = "~";
                        instruct_cd(s_);
                        string str3;
                        str3 = "userid=" + to_string(user[cur_user].user_id);
                        cout << str3.c_str();
                        display(str3.c_str());
                        flag_1[cur_user] = 1;
                    }
                    flush();
                }
            }
        } else if (order == "sof") {     //21
            show_sof();
        } else if (order == "uof") {     //22
            show_uof();
        } else if (order == "show") {    //23
            if (name1.empty())
                cout << "ָ���ʽ����!";
            else
                show_file(name1);
        } else if (order == "wbig") {      //24
            if (name2.empty())
                cout << "ָ���ʽ����!";
            else
                write_bigfile(name1.c_str(), name2.c_str());
        } else if (order == "rbig") {       //25
            if (name1.empty())
                cout << "ָ���ʽ����!";
            else
                read_bigfile(name1.c_str());
        } else if (order == "vim") {        //26
            if (name1.empty())
                cout << "ָ���ʽ����!";
            else {
                char s_[100];
                strcpy(s_, name1.c_str());
                ins_vim(s_);
                flush();
            }
        } else if (order == "help") {      //27
            help();
        } else if (order == "format") {    //28
            format();
        } else if (order == "cls") {        //29
            clear();
            flush();
        } else if (order == "exit") {       //30
            return 2;
        } else {
            cout << ">>ָ�����,����������" << endl;
            display(">>ָ�����,����������");
            //flush();
        }
    }
}

unsigned short changeuser(string name) {
    int codecorrect;
    string b;
    int id = checkuser(std::move(name));
    if (id == -1) {
        return 999;
    } else {
        cout << "FileSystem:$password:";
        cin >> b;
        getchar();
        codecorrect = checkpassword(b, id);
        if (codecorrect == 0) {
            return 999;
        } else return id;
    }
    //cur_user=id;
}

void signup()  //ע��
{
    string a, b, ans;
    int flag = 0;//�ܹ���������,���û��������ظ������
    int n;
    if (user_count >= USER_NUM - 1) {
        cout << "�û���������!" << endl;
        return;
    }
    while (flag == 0) {
        cout << "your username:" << endl;
        cin >> a;
        n = checkuser(a);
        if (n == -1) {
            cout << "your password:" << endl;
            cin >> b;
            cout << "ȷ��ע��(y/n)��" << endl;
            cin >> ans;
            if (ans == "y") {
                flag = 1;
                char *c = ChangeStrToChar(a);
                char *d = ChangeStrToChar(b);
                strcpy(user[user_count].user_name, c);
                strcpy(user[user_count].password, d);
                user[user_count].user_id = user_count;
                cur_user = user_count;
                strcpy(user_mem[cur_user].OFD[0].file_dir, "/");
                user_mem[cur_user].OFD[0].flag = 1;
                user_mem[cur_user].OFD[0].rw_point = 0;
                user_mem[cur_user].OFD[0].inode_number = 0;
                strcpy(user_mem[cur_user].cwd, "/");
                user_mem[cur_user].cur_dir = &sys_open_file[0];
                user_mem[cur_user].file_count = 1;
                string s_ = "~";
                instruct_cd(s_);
                // �����û�Ŀ¼
                string dir = "/home";
                instruct_cd(dir);
                creat_directory(c);
                dir = "/";
                instruct_cd(dir);
                cout << "ע��ɹ�!" << endl;
                cout << "userid=" << user[user_count].user_id << endl;
                user_count++;
            } else
                flag = 0;
        } else {
            cout << "�û����ظ�������������!" << endl;
        }
    }
}

int checkuser(string str)   //����û��Ƿ����
{
    int i, flag = 0, id = -1;
    char *c = ChangeStrToChar(std::move(str));
    for (i = 0; i < USER_NUM; i++) {
        if (strcmp(c, user[i].user_name) == 0) {
            flag = 1;
            id = i;
            break;
        }
    }
    if (flag == 0) {
        cout << "user not exists" << endl;
    }
    return id;
}

int checkpassword(string b, unsigned short n)//���û����ڣ���������Ƿ���ȷ
{
    int flag1 = 0;
    int codecorrect = 0;
    char *c = ChangeStrToChar(std::move(b));
    if (strcmp(c, user[n].password) == 0) {
        flag1 = 1;
//        cout << "password correct!" << endl;
//        getchar();
        codecorrect = 1;
    }
    if (flag1 == 0) {
        cout << "�������!" << endl;
    }
    return codecorrect;
}

char *ChangeStrToChar(string InputString)    //string����תchar����
{
    char *InputChar = new char[InputString.length()];
    int i;
    for (i = 0; i <= InputString.length(); i++)
        InputChar[i] = InputString[i];
    InputChar[i] = '\0';//�����һ���ַ������Ԫ���ÿգ�������ܳ�����ֵĴ���
    return InputChar;
}
