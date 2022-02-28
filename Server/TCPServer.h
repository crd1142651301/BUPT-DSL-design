#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <thread>
#include <algorithm>
#include <cctype>
#include <mutex>
#include <time.h>
#include "script.h"
#include "getdata.h"

using namespace std;

/* 定义缓冲区最大长度和能够接收的最大客户数量 */
#define MAXPACKETSIZE 1024
#define MAX_CLIENT 5

/* 描述每一个端口的信息 */
struct descript_socket {
	int socket     = -1;
	string ip      = "";
	int id         = -1; 
};

/* 运行时每个用户的环境变量 */
class Exec_environment {
public:
	string number;
    Build_Script::stepID curstepID;  
};

/* 封装TCP服务 */
class TCPServer {
public:
	/* 初始化TCP服务以及机器人所需数据 */
	int setup(int port, Build_Script::State state, DATA data, vector<int> opts = vector<int>());  // 设置端口服务端端口
	/* 负责新客户端的接入 */
	void accepted();                               

private:
	int sockfd;								// 服务端端口号
	struct sockaddr_in serverAddress;		// 服务端套接子地址
	struct sockaddr_in clientAddress;		// 客户端套接子地址
	pthread_t serverThread[MAX_CLIENT];     // 服务端最多能接收MAX_CLIENT个客户端

	static vector<descript_socket*> newsockfd;	// 存储客户端端口信息数组
	static Build_Script::State _state;		// 语法树
	static DATA _data;						// 用户数据
	static int num_client;					// 客户端数量
	static pthread_mutex_t mutex;			// 用来互斥各个线程间的写
	static void* Task(void * argv);			// 机器人完成服务客户端的任务
	/* 在规定时间内从desc->sock端口接收数据 */
	static int recieve(char* recvbuf, int time, descript_socket* desc);
	/* 向desc->sock发送数据 */
	static int Send(std::vector<Build_Script::Item> speak, descript_socket* desc, Exec_environment& exe);
	/* KMP算法匹配字符串 */
	static bool KMPcompare(string s, string p);
	/* 匹配branch并改变当前用户环境的状态 */
	static bool HandleStepID(char* recvbuf, Exec_environment& exe, std::unordered_map<std::string, Build_Script::stepID>& branchs);
};

#endif