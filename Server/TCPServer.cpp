#include "TCPServer.h" 

/* 类中静态变量重新声明 */
int TCPServer::num_client;
vector<descript_socket*> TCPServer::newsockfd;
Build_Script::State TCPServer::_state;
DATA TCPServer::_data;
pthread_mutex_t TCPServer::mutex;


void* TCPServer::Task(void *arg)
{
	/* 子线程分离出主线程，互不干涉 */
    pthread_detach(pthread_self());

	struct descript_socket *desc = (struct descript_socket*) arg;

    cerr << "open client[ id:"<< desc->id <<" ip:"<< desc->ip <<" socket:"<< desc->socket << "]" << endl;
	
	/* 建立客户信息表 */
	Exec_environment exe;
	char recvbuf[MAXPACKETSIZE];
	memset(recvbuf, 0x00, sizeof(recvbuf));
	recv(desc->socket, recvbuf, MAXPACKETSIZE, 0);

	// 初始化环境变量
	exe.number = recvbuf;
	// 在这里规定脚本文件第一个step为开始状态
	exe.curstepID = _state.init;
	bool exit = false;

	while(!exit)
	{
		Build_Script::Step step = _state._steps[exe.curstepID];
		for (auto command : step.commands) {
			if (strcmp(command.c_str(), "speak") == 0) 
			{
                //发送消息
				int ans = Send(step.speak, desc, exe);
				if (ans == -1)
				{
					exit = true;
					break;
				}
            }
            else if (strcmp(command.c_str(), "listen") == 0) 
			{
                // 等待接收消息，并完成切换状态
                int ans = recieve(recvbuf, step.listen, desc);
				if (ans == -1)
					exe.curstepID = step.silence;
				else if (ans == 0)
				{
					exit = true;
				}
				else
				{
					bool match = HandleStepID(recvbuf, exe, step.branchs);
					if (!match)    exe.curstepID = step._default;
				}
				break;
            }
			else if (strcmp(command.c_str(), "goto") == 0)
			{
				exe.curstepID = step._goto;
				break;
			}
			else if (strcmp(command.c_str(), "exit") == 0)
			{
				exit = true;
				break;
			}
			usleep(500);
		}
		if (exit)
		{
			sleep(1);
			string buf = "exit";
			int ans = send(desc->socket, buf.c_str(), buf.length(), 0);
			cout << "exit send state: " << ans << endl;
		}
    }

	close(desc->id);
	if(desc != NULL)
	{	
		delete desc;
		desc = NULL;
	}

	cerr << "exit thread: " << this_thread::get_id() << endl;
	pthread_exit(NULL);

	return 0;
}

int TCPServer::setup(int port, Build_Script::State state, DATA data, vector<int> opts)
{
	/* 建立绑定服务端套接字，并初始化机器人客服所需的各种数据 */
	/*
	返回值：
		return 0 成功绑定套接字
		其他 出现错误
	参数：
		port：端口号
		state：语法树和初始步骤
		data：数据集
		opts：套接字选项
	*/
	int opt = 1;
	sockfd = socket(AF_INET,SOCK_STREAM,0);
 	memset(&serverAddress,0,sizeof(serverAddress));

	for(unsigned int i = 0; i < opts.size(); i++) {
		if( (setsockopt(sockfd, SOL_SOCKET, opts.size(), (char *)&opt, sizeof(opt))) < 0 ) {
			cerr << "Errore setsockopt" << endl; 
      			return -1;
	      	}
	}

	serverAddress.sin_family      = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port        = htons(port);

	if((::bind(sockfd,(struct sockaddr *)&serverAddress, sizeof(serverAddress))) < 0){
		cerr << "Errore bind" << endl;
		return -1;
	}
	
 	if(listen(sockfd,5) < 0){
		cerr << "Errore listen" << endl;
		return -1;
	}

	_state = state;
	_data = data;
	num_client = 0;
	pthread_mutex_init(&mutex, NULL);
	return 0;
}

void TCPServer::accepted()
{
	/* 接收新的客户端并建立数据结构存储该客户端信息 */
	socklen_t sosize    = sizeof(clientAddress);
	descript_socket *so = new descript_socket;
	so->socket          = accept(sockfd, (struct sockaddr*)&clientAddress, &sosize);
	so->id              = num_client;
	so->ip              = inet_ntoa(clientAddress.sin_addr);
	newsockfd.push_back( so );
	cerr << "accept client[ id:" << newsockfd[num_client]->id << 
	                      " ip:" << newsockfd[num_client]->ip << 
		              " handle:" << newsockfd[num_client]->socket << " ]" << endl;
	pthread_create(&serverThread[num_client], NULL, &Task, (void *)newsockfd[num_client]);
	++num_client;
}

int TCPServer::recieve(char* recvbuf, int time, descript_socket* desc)
{
	/*
	函数功能：在规定时间内接收客户端信息
	返回值：
		return -1：超过时间都未受到消息
		return 0: 客户端直接断开
		return 其他：正常接收到的字符串长度
	参数：
		recvbuf:接收缓冲区
		time:接收规定时间
		desc:端口指针
	*/
	memset(recvbuf, 0x00, sizeof(recvbuf));
	int n = -1;
	int start_time = clock(), end_time = clock();

	while (end_time - start_time < time)
	{
		n = recv(desc->socket, recvbuf, MAXPACKETSIZE, 0);
		
		if(n != -1) 
		{
			/* socket返回值为0,表示断开连接 */
			if (n == 0)
			{
				cerr << "close client[ id:"<< desc->id <<" ip:"<< desc->ip <<" socket:"<< desc->socket<<" ]" << endl;
				close(desc->socket);

				/* 有客户端退出，将退出进程的socket移动到最后并删掉 */
				int id = desc->id;
				auto new_end = std::remove_if(newsockfd.begin(), newsockfd.end(),
											[id](descript_socket *device){ return device->id == id; });
				newsockfd.erase(new_end, newsockfd.end());
				
				if(num_client > 0) num_client--;
				break;
			}
			recvbuf[n] = '\0';
			string query(recvbuf);
			pthread_mutex_lock(&mutex);
			cout << "--------------------------" << endl;
			cout << "id:      " << desc->id << "\n"
				 << "ip:      " << desc->ip << "\n"
				 << "question: " << query << "\n"
				 << "socket:  " << desc->socket  << endl;
			cout << "--------------------------" << endl;
			pthread_mutex_unlock(&mutex);
			break;
		}
	}
	return n;
}

int TCPServer::Send(std::vector<Build_Script::Item> speak, descript_socket* desc, Exec_environment& exe)
{
	/*
	函数功能：发送答案
	无返回值
	参数说明：
		speak：答案数组
		desc: 端口指针
		exe: 用户环境变量
	*/
	int res = 0;
	string ans;
	for (auto item : speak)
	{
		if (item.first)
		{
			auto it = _data.find(exe.number);
			if (it != _data.end())
			{
				auto _it = it->second.find(item.second);
				if (_it != it->second.end())
					ans += _data[exe.number][item.second];
				else
				{
					ans = "无" + exe.number + "号码的" + _it->first + "数据";
					res = -1;
					break;
				}
			}
			else
			{
				ans = "无" + exe.number + "号码";
				res = -1;
				break;
			}
		}
		else
			ans += item.second;
	}
	send(desc->socket, ans.c_str(), ans.length(), 0);
	pthread_mutex_lock(&mutex);
	cout << "--------------------------" << endl;
	cout << "id:      " << desc->id << "\n"
	 	 << "ip:      " << desc->ip << "\n"
		 << "answer: " << ans << "\n"
	 	 << "socket:  " << desc->socket  << endl;
	cout << "--------------------------" << endl;
	pthread_mutex_unlock(&mutex);
	return res;
}

// KMP算法匹配字符串，匹配字符串p是否在s中
bool TCPServer::KMPcompare(string s, string p) {
	for (char& ch : s) {
		if (ch >= 'A' && ch <= 'Z') {
			ch = ch + 'a' - 'A';
		}
	}
	for (char& ch : p) {
		if (ch >= 'A' && ch <= 'Z') {
			ch = ch + 'a' - 'A';
		}
	}
	int n = s.size(), m = p.size();
	if (m == 0) return 0;

	s.insert(s.begin(), ' ');
	p.insert(p.begin(), ' ');
	vector<int> next(m + 1);

	for (int i = 2, j = 0; i <= m; i++) {
		while (j and p[i] != p[j + 1]) j = next[j];
		if (p[i] == p[j + 1]) j++;
		next[i] = j;
	}

	for (int i = 1, j = 0; i <= n; i++) {
		while (j and s[i] != p[j + 1]) j = next[j];
		if (s[i] == p[j + 1]) j++;
		if (j == m) return true;
	}
	return false;
}

bool TCPServer::HandleStepID(char* recvbuf, Exec_environment& exe, std::unordered_map<std::string, Build_Script::stepID>& branchs)
{
	/*
	函数功能： 匹配分支切换状态
	返回值说明：
		return true: 匹配成功
		return false: 匹配失败
	参数说明：
		recvbuf: 接收到的字符串指针
		exe: 用户环境变量
		branchs: 分支哈希
	*/
	string query(recvbuf);
	for (auto iter = branchs.begin(); iter != branchs.end() ; ++iter)
	{
		if (KMPcompare(query, iter->first))
		{
			exe.curstepID = iter->second;
			return true;
		}
	}
	return false;
}
