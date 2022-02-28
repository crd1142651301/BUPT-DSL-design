#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <pthread.h>
#include "TCPClient.h"

using namespace std;

/* 定义IP地址和端口号 */
#define IP_ADDR "127.0.0.1"
#define CLIENT_PORT 5050

/* 读写线程函数 */
void* sendMsg(void * args);
void* recvMsg(void * args);

TCPClient tcp;  // 客户端tcp连接
ifstream in;   // 测试用输入文件

int main(int argc, char* argv[]) 
{

    if (argc != 2) 
    {
        printf("输入格式不正确，请按照***.exe in.txt out.txt再次尝试！");
        return -1;
    }

    tcp.setup(IP_ADDR, CLIENT_PORT);

    /* 将号码发送过去 */
    string filename1 = argv[1];
    in.open(filename1);

    pthread_t recv;
    pthread_t send;

    /* 开辟读写线程 */
    pthread_create(&send, NULL, sendMsg, NULL);
    pthread_create(&recv, NULL, recvMsg, NULL);
    pthread_join(send, 0);
    pthread_join(recv, 0);

    tcp.exit();
    return 0;
}

void* sendMsg(void * args)
{
    while (!in.eof()) {
        string msg;
        in >> msg;
        size_t outputlength = msg.length();
        if (0 == outputlength)
            continue;
        ssize_t bytecount = send(tcp.getSock(), msg.c_str(), outputlength, 0);//发送消息
        if (bytecount < 0) {
            cout << "send failed" << endl;
        }
        sleep(1);
    }
    pthread_exit(NULL);
    return nullptr;
}
void* recvMsg(void * args)
{
    bool exit = false;
    while (!exit) {
        char msg[4096];
        memset(msg, 0, sizeof(msg));
    
        ssize_t bytecount = recv(tcp.getSock(), msg, sizeof(msg), 0); //接收消息
        if (bytecount <= 0) 
            continue;
        else {
            string recv(msg);

            if (recv == "exit")
                exit = true;
            else
                cout << recv << endl;
        }
        usleep(100);
    }
    pthread_exit(NULL);
    return nullptr;
}