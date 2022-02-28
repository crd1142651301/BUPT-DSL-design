#include "getdata.h"

/* 判断文件类型，及是否能够打开 */
UserData::UserData(const string& filename)
{
    int n = filename.length();
    if (filename.substr(n-4, n) == ".txt") {
        _input.open(filename);
        if (_input.fail())
            printf("文件打开失败\n");
        else 
        {
            storeData();
            printf("文件打开成功\n");
        }
    }else
        printf("脚本正确的文件后缀格式应该为.txt\n");
}

void UserData::show()
{
    cout << "每个用户的数据如下：" << endl;
    for (auto it = _data.begin() ; it != _data.end() ; ++it)
    {
        cout << "---------------------" << endl;
        cout << "number key: " << it->first << endl;
        for (auto _it = it->second.begin() ; _it != it->second.end() ; ++_it)
            cout << _it->first << ": " << _it->second << endl;
        cout << "---------------------" << endl;
    }
}

DATA UserData::getUserData()
{   
    return _data;
}

/* 从文件中读取用户数据 */
void UserData::storeData()
{
    string line;
    getline(_input, line);
    vector<string> header;
    istringstream record(line);
    string str;
    while (record >> str)
        header.emplace_back(str);
    while (getline(_input, line))
    {
        istringstream _record(line);
        string number;
        _record >> number;
        for (int i = 1; i < header.size() ; ++i)
        {
            string data;
            _record >> data;
            _data[number][header[i]] = data;
        }
    }
}