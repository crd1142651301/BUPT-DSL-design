#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netdb.h> 
#include <vector>

using namespace std;

class TCPClient {
private:
  int sock;
  std::string address;
  int port;
  struct sockaddr_in server;

public:
  TCPClient();
  int getSock();      // 得到客户端sock端口
  bool setup(string address, int port); // 设置ip地址和端口
  bool Send(string data);           // 向服务端发送data消息
  string receive(int size = 1024);  // 从服务端接收接收消息
  void exit();    // 退出端口
};

#endif