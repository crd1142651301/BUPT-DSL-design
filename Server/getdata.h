#ifndef GETDATA_H
#define GETDATA_H

#include <unordered_map>
#include <string>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>

using namespace std;

/* 存储用户信息的数据结构，是用户拥有一个主键字符串得到他的所有信息 */
typedef unordered_map<string, unordered_map<string, string>> DATA;


class UserData {
public:
    UserData(const string& filename);
    /* 向类外提供获取用户数据的接口 */
    DATA getUserData();
    /* 测试桩测试数据输出 */
    void show(void);
private:
    ifstream _input;
    DATA _data;
    void storeData();
};



#endif