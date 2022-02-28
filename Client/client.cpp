#include <stdio.h>
#include <iostream>
#include <pthread.h>
#include "TCPClient.h"

using namespace std;

/* 定义IP地址和端口 */
#define IP_ADDR "127.0.0.1"
#define CLIENT_PORT 5050
TCPClient tcp;

/* 读写线程函数 */
void* sendMsg(void * args);
void* recvMsg(void * args);

int main(int argc, char* argv[]) 
{

    if (argc != 2) 
    {
        printf("输入格式不正确，请按照***.exe number再次尝试！");
        return -1;
    }

    tcp.setup(IP_ADDR, CLIENT_PORT);

    /* 将号码发送过去 */
    tcp.Send(argv[1]);
    pthread_t recv;
    pthread_t send;

    /* 开辟读写线程 */
    pthread_create(&recv, NULL, recvMsg, NULL);
    pthread_create(&send, NULL, sendMsg, NULL);
    pthread_join(recv, 0);
    pthread_join(send, 0);
 
    tcp.exit();
    return 0;
}
void* sendMsg(void * args)
{
    while (true) {
        char msg[4096];
        memset(msg, 0, sizeof(msg));
        cin >> msg;
        size_t outputlength = strlen(msg);
        if (0 == outputlength)
            continue;
        ssize_t bytecount = send(tcp.getSock(), msg, outputlength, 0);//发送消息
        if (bytecount < 0) {
            cout << "send failed" << endl;
        }
        usleep(1);
    }
}
void* recvMsg(void * args)
{
    while (true) {
        char msg[4096];
        memset(msg, 0, sizeof(msg));
 
        ssize_t bytecount = recv(tcp.getSock(), msg, sizeof(msg), 0); //接收消息
        if (bytecount <= 0) {
            continue;
        } else {
            cout << "[客服]: " << msg << endl;
        }
    }
}