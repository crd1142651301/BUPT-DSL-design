#include <iostream>
#include "TCPServer.h"
#include "script.h"
#include "getdata.h"

#define SERVER_PORT 5050

/* 全局变量 */
TCPServer tcp;

int main(int argc, char* argv[]) 
{
    // 判断命令行输入是否符合规范
    if (argc != 3) 
    {
        printf("输入参数错误，正确格式应该为***.exe ***.txt ***.txt\n");
        return -1;
    }
    
    vector<int> opts = { SO_REUSEPORT, SO_REUSEADDR };
	// 解释脚本，建立语法分析树
    std::string filename1(argv[1]), filename2(argv[2]);
    Build_Script::Script script(filename1);
    UserData userData(filename2);

    /* 测试状查看读入是否正确 */
    script.show();
    userData.show();

	if(tcp.setup(SERVER_PORT, script.get_State(), userData.getUserData(), opts) == 0) {
		while(true) {
				tcp.accepted();
				cerr << "Accepted" << endl;
		}
	}
	else
		cerr << "Errore apertura socket" << endl;
    return 0;
}