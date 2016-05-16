#include<winsock2.h>
#include<windows.h>
#include<iostream>
#include<io.h>

using namespace std;
#pragma comment(lib, "ws2_32.lib")

#define BUFFSIZE 1024
#define LEN 256

enum{NO,LOGIN,CLOSE,QUIT,POS,MODEPORT,MODEPASV,DIR,ABOUT,GET,RESUME,PUT,LS,MKDIR,RMDIR,CD,DEL};
enum{PORT, PASV};

char ServerIp[LEN] = { 0 };      //服务器IP
int PortPasv = PASV;              //0-PORT,1-PASV
SOCKET CmdSocket;					    //控制连接
SOCKET DataSocket;					//数据连接
char CmdBuf[BUFFSIZE]   = {0};		//控制缓冲区
char DataBuf[BUFFSIZE]  = {0};      //命令缓冲区

int parseCmd(char* str);                     //解析命令
void getInput(char *input, int len);           //得到用户的输入，并去掉首部和尾部的空白符
char* lspace(char * input);					   //去首部空白
char* rspace(char * input);                     //去尾部空白

int CreateCmdSocket();                           //创建控制套接字
int MyRecv(SOCKET s, char *buf, int len);    //接收,-2表示套接字上不存在数据，-1表示SOCKET_ERROR，0表示连接断开,大于0表示接收的字节数
int MySend(SOCKET s, char *buf, int len);     //发送,-2表示不能向套接字上写数据，-1表示SOCKET_ERROR，大于0表示发送的字节数
int  DataConnect();                     //建立数据连接

void login();					               //登陆
void close();					               //断开
void about();                                   //帮助
void get();									   //下载模式
void put();                                    //上传模式
void pos();						   //当前目录
void mkdir(char*);                             //建立目录
void rmdir(char*);							   //删除目录
void cd(char*);                         //改变当前目录
void del(char*);                           //删除指定文件
void dir(char*);                           //列出目录中文件
void ls(char*);                           //列出服务器当前目录的文件

void download(char*, char*);                   //下载
void upload(char*, char*);                     //上传


DWORD WINAPI ListenFun(LPVOID lp)
{
	int ret;
	SOCKET listenSock;
	SOCKADDR_IN addrFrom;
	int len = sizeof(SOCKADDR);

	SOCKET* s = (SOCKET*)lp;
	SOCKET DataSocket = *s;
	ret = listen(DataSocket, 1);
	if(ret == SOCKET_ERROR)
	{
		cout << "数据监听失败" <<endl;
		return -1;
	}
	listenSock = accept(DataSocket, (sockaddr*)&addrFrom, &len);
	if(listenSock == INVALID_SOCKET)
	{
		cout << "接受连接请求失败" <<endl;
		return -1;
	}
	*s = listenSock;            //将listenSock赋给DataSocket
	return 0;
}

int main()
{
	int ret;
	WSADATA wsaData;
	char input[BUFFSIZE] ={0};

	if(_access("downloads",0)==-1)				//判断存在downloads目录
	{
		CreateDirectory("downloads",NULL);
	}

	ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if(ret != 0)
	{
		cout << "加载套接字库失败" <<endl;
		return -1;
	}

	if(LOBYTE(wsaData.wVersion) != 2 ||
		HIBYTE(wsaData.wVersion) != 2)
	{
		cout << "套接字库版本不一致" <<endl;
		WSACleanup();
		return -1;
	}
	if(CreateCmdSocket() == 0)
		return -1;
	about();
	while(1)
	{
		cout << "Cmd>> ";
		getInput(input, BUFFSIZE);   //得到输入的字符串并去掉首尾空白符
		ret = parseCmd(input);   //从输入的命令中返回相应的命令数字
		switch(ret)
		{
		case NO:
			break;
		case LOGIN:
			login();
			break;
		case CLOSE:
			close();
			break;
		case ABOUT:
			about();
			break;
		case GET:
			get();
			break;
		case PUT:
			put();
			break;
		case POS:
			pos();
			break;
		case MKDIR:
			mkdir(input);
			break;
		case RMDIR:
			rmdir(input);
			break;
		case CD:
			cd(input);
			break;
		case DEL:
			del(input);
			break;
		case DIR:
			dir(input);
			break;
		case LS:
			ls(input);
			break;

		case QUIT:
			cout << "已退出系统"<<endl;
			closesocket(CmdSocket);
			WSACleanup();
			return 0;
		case MODEPORT:
			PortPasv = PORT;
			cout<<"已转换至PORT模式"<<endl;
			break;
		case MODEPASV:
			PortPasv = PASV;
			cout<<"已转换至PASV模式"<<endl;
			break;
		default:
			cout << "命令错误" <<endl;
			break;
		}
	}
	closesocket(CmdSocket);
	WSACleanup();
	return 0;
}

int parseCmd(char* str)
{
	if(str[0] == '\0')
		return NO;
	else if(stricmp(str, "login") == 0)
		return LOGIN;
	else if(stricmp(str, "close") == 0)
		return CLOSE;
	else if (stricmp(str, "about") == 0)
		return ABOUT;
	else if (stricmp(str, "get") == 0)
		return GET;
	else if (stricmp(str, "put") == 0)
		return PUT;
	else if (stricmp(str, "pos") == 0)
		return POS;
	else if (stricmp(str, "mkdir") == 0 || strnicmp(str, "mkdir ", 6) == 0)
		return MKDIR;
	else if (stricmp(str, "rmdir") == 0 || strnicmp(str, "rmdir ", 6) == 0)
		return RMDIR;
	else if (stricmp(str, "cd") == 0 || strnicmp(str, "cd ", 3) == 0)
		return CD;
	else if (stricmp(str, "del") == 0 || strnicmp(str, "del ", 4) == 0)
		return DEL;
	else if (stricmp(str, "dir") == 0 || strnicmp(str, "dir ", 4) == 0)
		return DIR;
	else if (stricmp(str, "ls") == 0 || strnicmp(str, "ls ", 3) == 0)
		return LS;

	else if(stricmp(str, "quit") == 0)
		return QUIT;
	else if(stricmp(str, "port") == 0)
		return MODEPORT;
	else if(stricmp(str, "pasv") == 0)
		return MODEPASV;
	else
		return -1;
}
int CreateCmdSocket()
{
	CmdSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(CmdSocket == INVALID_SOCKET)
	{
		cout << "创建套接字失败" << endl;
		return 0;
	}
	return 1;
}

void login()
{
	int ret;
	SOCKADDR_IN serverAddr;
	char ipaddr[LEN]   = {0};
	char username[LEN] = {0};
	char password[LEN] = {0};

	cout << "输入服务器IP地址: ";
	getInput(ipaddr, LEN);

	serverAddr.sin_addr.S_un.S_addr = inet_addr(ipaddr);
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(21);

	ret = connect(CmdSocket, (sockaddr*)&serverAddr, sizeof(SOCKADDR));

	if(ret == SOCKET_ERROR)
	{
		ret = WSAGetLastError();
		if(ret == WSAEISCONN)
		{
			cout << "已连接了" << ServerIp << ",请先断开连接" <<endl;
			return;
		}
		else if(ret == WSAENOTSOCK)
		{
			if(CreateCmdSocket() == 0)
				return;
			if(connect(CmdSocket, (sockaddr*)&serverAddr, sizeof(SOCKADDR)) == SOCKET_ERROR)
			{
				cout << "连接失败" <<  WSAGetLastError() << endl;
				return;
			}
		}
		else if (ret == WSAETIMEDOUT)
		{
			cout << "连接超时，请重新连接" <<  WSAGetLastError() << endl;
			return ;
		}
		else if(ret == WSAENETUNREACH)
		{
			cout << "目的地址不可达，请重新连接" << WSAGetLastError() << endl;
			return;
		}
		else {
			cout << "连接失败" << WSAGetLastError() << endl;
			return;
		}
	}

	ZeroMemory(CmdBuf, BUFFSIZE);
	ret = MyRecv(CmdSocket, CmdBuf, BUFFSIZE);

	if(ret <= 0)
	{
		closesocket(CmdSocket);
		return;
	}

	if(strncmp(CmdBuf, "220", 3) == 0)
	{
		cout << CmdBuf ;
		strcpy(ServerIp, ipaddr);
		cout << "连接成功\n请输入用户名:";
	}
	else
	{
		cout << CmdBuf ;
		cout << "连接失败" << endl;
		closesocket(CmdSocket);
		return ;
	}

	getInput(username, LEN);
	ZeroMemory(CmdBuf, BUFFSIZE);
	strcat(CmdBuf, "USER ");
	strcat(CmdBuf, username);
	strcat(CmdBuf, "\r\n");

	ret = MySend(CmdSocket, CmdBuf, strlen(CmdBuf));

	if(ret <= 0)
	{
		closesocket(CmdSocket);
		return;
	}

	ZeroMemory(CmdBuf, BUFFSIZE);
	ret = MyRecv(CmdSocket, CmdBuf, BUFFSIZE);

	if(ret <= 0)
	{
		closesocket(CmdSocket);
		return;
	}

	if(strncmp(CmdBuf, "331", 3) == 0)
	{
		cout << CmdBuf ;
		cout << "请输入密码:";
	}
	else
	{
		cout << CmdBuf ;
		return ;
	}

	ZeroMemory(CmdBuf, BUFFSIZE);
	getInput(password, LEN);
	strcat(CmdBuf, "PASS ");
	strcat(CmdBuf, password);
	strcat(CmdBuf, "\r\n");

	ret = MySend(CmdSocket, CmdBuf, strlen(CmdBuf));
	if(ret <= 0)
	{
		closesocket(CmdSocket);
		return;
	}

	ZeroMemory(CmdBuf, BUFFSIZE);
	ret = MyRecv(CmdSocket, CmdBuf, BUFFSIZE);

	if(ret <= 0)
	{
		cout << "密码错误"<<endl;
		closesocket(CmdSocket);
		return;
	}

	if(strncmp(CmdBuf, "230", 3) == 0)
	{
		cout << CmdBuf ;
		cout << "登入成功" <<endl;
	}
	else
	{
		cout << CmdBuf;
		return ;
	}

}
void close()
{

	int ret;
	ZeroMemory(CmdBuf, BUFFSIZE);
	strcat(CmdBuf, "QUIT");
	strcat(CmdBuf, "\r\n");
	ret = MySend(CmdSocket, CmdBuf, strlen(CmdBuf));

	if(ret <=  0)
	{
		closesocket(CmdSocket);
		return;
	}
	ZeroMemory(CmdBuf, BUFFSIZE);
	ret = MyRecv(CmdSocket, CmdBuf, BUFFSIZE);

	if(ret <= 0)
	{
		closesocket(CmdSocket);
		return;
	}

	if(strncmp(CmdBuf, "221", 3) == 0)
	{
		cout << CmdBuf;
		cout << "已断开连接" <<endl;
		closesocket(CmdSocket);
	}
	else
	{
		cout << CmdBuf;
		cout << "断开连接出错" << endl;
	}
}
void getInput(char *input, int len) //去掉首尾空白
{
	char* temp = new char[len];
	char* pstr = NULL;

	cin.getline(temp, len);
	pstr = lspace(temp);
	pstr = rspace(pstr);

	strcpy(input, pstr);
	delete[] temp;
	temp = NULL;

	if(!cin)               //清空缓冲区
	{
		cin.clear();
		while(cin.get() != '\n')
			continue;
	}
}

char* lspace(char * input)  //去掉首部空白
{
	char* pc = input;
	if(*input == '\0')
		return input;
	while(*pc)
	{
		if(*pc == ' ' || *pc == '\t')
		{
			pc++;
		}
		else
			break;
	}
	if( pc != input)
		input = pc;
	return input;
}

char* rspace(char* input) //去掉尾部空白
{
	char* pc = input;
	if(*input == '\0')
		return input;
	int len = strlen(input);
	pc = input + len -1;

	while(pc != input)
	{
		if(*pc == ' ' || *pc == '\t')
		{
			pc--;
		}
		else
			break;
	}

	if(pc != input + len -1)
		*(pc + 1) = '\0';
	return input;
}

void pos()
{
	int ret ;

	ZeroMemory(CmdBuf, BUFFSIZE);
	strcat(CmdBuf, "PWD\r\n");
	ret = MySend(CmdSocket, CmdBuf, strlen(CmdBuf));

	if(ret <=  0)
	{
		cout << "未连接至服务器主机" << endl;
		closesocket(CmdSocket);
		return;
	}

	ZeroMemory(CmdBuf, BUFFSIZE);
	while (MyRecv(CmdSocket, CmdBuf, BUFFSIZE) > 0)
	{
		cout << CmdBuf;
		ZeroMemory(CmdBuf, BUFFSIZE);
	}
}

void mkdir(char* input)
{
	int ret ;
	char temp[BUFFSIZE] = {0};
	char* pstr =NULL;

	if(stricmp(input, "mkdir") == 0)
	{
		cout<<"输入新建的目录名:";
		getInput(temp, BUFFSIZE);
		if(temp[0] == '\0')
		{
			cout << "目录名不能为空"<<endl;
			return;
		}
		pstr = temp;
	}
	else
		pstr = lspace(input + 5);


	ZeroMemory(CmdBuf, BUFFSIZE);
	strcat(CmdBuf, "MKD ");
	strcat(CmdBuf, pstr);
	strcat(CmdBuf, "\r\n");

	ret = MySend(CmdSocket, CmdBuf, strlen(CmdBuf));
	if(ret <=  0)
	{
		cout << "未连接" << endl;
		closesocket(CmdSocket);
		return;
	}

	ZeroMemory(CmdBuf, BUFFSIZE);
	while (MyRecv(CmdSocket, CmdBuf, BUFFSIZE) > 0)
	{
		cout << CmdBuf;
		ZeroMemory(CmdBuf, BUFFSIZE);
	}
}

void rmdir(char* input)
{
	int ret ;
	char temp[BUFFSIZE] = {0};
	char* pstr =NULL;

	if(stricmp(input, "rmdir") == 0)
	{
		cout<<"输入要删除的目录名:";
		getInput(temp, BUFFSIZE);
		if(temp[0] == '\0')
		{
			cout << "目录名不能为空"<<endl;
			return;
		}
		pstr = temp;
	}
	else
		pstr = lspace(input + 5);


	ZeroMemory(CmdBuf, BUFFSIZE);
	strcat(CmdBuf, "RMD ");
	strcat(CmdBuf, pstr);
	strcat(CmdBuf, "\r\n");

	ret = MySend(CmdSocket, CmdBuf, strlen(CmdBuf));
	if(ret <=  0)
	{
		closesocket(CmdSocket);
		return;
	}

	ZeroMemory(CmdBuf, BUFFSIZE);
	while (MyRecv(CmdSocket, CmdBuf, BUFFSIZE) > 0)
	{
		cout << CmdBuf;
		ZeroMemory(CmdBuf, BUFFSIZE);
	}
}

void cd(char* input)
{
	int ret ;
	char temp[BUFFSIZE] = {0};
	char* pstr =NULL;

	if(stricmp(input, "cd") == 0)
	{
		cout<<"输入切换到的目录名:";
		getInput(temp, BUFFSIZE);
		if(temp[0] == '\0')
		{
			cout << "目录名不能为空"<<endl;
			return;
		}
		pstr = temp;
	}
	else
		pstr = lspace(input + 2);


	ZeroMemory(CmdBuf, BUFFSIZE);
	strcat(CmdBuf, "CWD ");
	strcat(CmdBuf, pstr);
	strcat(CmdBuf, "\r\n");

	ret = MySend(CmdSocket, CmdBuf, strlen(CmdBuf));
	if(ret <=  0)
	{
		closesocket(CmdSocket);
		return;
	}

	ZeroMemory(CmdBuf, BUFFSIZE);
	while (MyRecv(CmdSocket, CmdBuf, BUFFSIZE) > 0)
	{
		cout << CmdBuf;
		ZeroMemory(CmdBuf, BUFFSIZE);
	}
}

void del(char* input)
{
	int ret ;
	char temp[BUFFSIZE] = {0};
	char* pstr =NULL;

	if(stricmp(input, "del") == 0)
	{
		cout<<"输入需要删除的文件名:";
		getInput(temp, BUFFSIZE);
		if(temp[0] == '\0')
		{
			cout << "文件名不能为空."<<endl;
			return;
		}
		pstr = temp;
	}
	else
		pstr = lspace(input + 3);


	ZeroMemory(CmdBuf, BUFFSIZE);
	strcat(CmdBuf, "DELE ");
	strcat(CmdBuf, pstr);
	strcat(CmdBuf, "\r\n");

	ret = MySend(CmdSocket, CmdBuf, strlen(CmdBuf));
	if(ret <=  0)
	{
		closesocket(CmdSocket);
		return;
	}

	ZeroMemory(CmdBuf, BUFFSIZE);
	while (MyRecv(CmdSocket, CmdBuf, BUFFSIZE) > 0)
	{
		cout << CmdBuf;
		ZeroMemory(CmdBuf, BUFFSIZE);
	}
}

int DataConnect()
{
	HANDLE hThread;
	int ret, len;
	SOCKADDR_IN localAddr;
	SOCKADDR_IN serverAddr;
	unsigned short  hbPort, lbPort, nPort;
	char temp[8] = {0};
	char* pStr = NULL;
	char* pStr1 = NULL;
	char ipstr[20] = {0};
	int count;

	if(PortPasv == PORT)
	{
		DataSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if(DataSocket == INVALID_SOCKET)
		{
			cout << "创建套接字失败" << endl;
			return 0;
		}

		localAddr.sin_family = AF_INET;
		localAddr.sin_port = htons(0);                              //获取系统端口号
		localAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");    //本地地址

		ret = bind(DataSocket, (sockaddr *)&localAddr, sizeof(SOCKADDR));
		if(ret == SOCKET_ERROR)
		{
			cout << "绑定套接字失败" <<endl;
			closesocket(DataSocket);
			return 0;
		}

		len = sizeof(SOCKADDR);
		getsockname(DataSocket, (sockaddr *)&localAddr, &len);
		nPort = ntohs(localAddr.sin_port);
		hbPort = nPort / 256;
		lbPort = nPort % 256;

		strcpy(ipstr, inet_ntoa(localAddr.sin_addr));
		pStr = ipstr;

		while(*pStr)
		{
			if(*pStr == '.')
				*pStr = ',';
			pStr++;
		}

		ZeroMemory(CmdBuf, BUFFSIZE);
		strcat(CmdBuf, "PORT ");
		strcat(CmdBuf, ipstr);
		strcat(CmdBuf, ",");
		strcat(CmdBuf, itoa(hbPort, temp, 10));
		strcat(CmdBuf, ",");
		strcat(CmdBuf, itoa(lbPort, temp, 10));
		strcat(CmdBuf, "\r\n");

		ret = MySend(CmdSocket, CmdBuf, strlen(CmdBuf));

		if(ret <=  0)
		{
			closesocket(CmdSocket);
			return 0;
		}

		ZeroMemory(CmdBuf, BUFFSIZE);
		ret = MyRecv(CmdSocket, CmdBuf, BUFFSIZE);
		if(ret <= 0)
	    {
		    closesocket(CmdSocket);
			return 0;
		}

		if(strncmp(CmdBuf, "200", 3) == 0)
		{
			cout << CmdBuf;
			hThread = CreateThread(0,0,ListenFun,(LPVOID)&DataSocket,0,0);
			if(hThread)
				CloseHandle(hThread);
			return 1;
		}
		else
		{
			cout << CmdBuf;
			return 0;
		}

	}
	else
	{
		ZeroMemory(CmdBuf, BUFFSIZE);
		strcat(CmdBuf, "PASV \r\n");
		ret = MySend(CmdSocket, CmdBuf, strlen(CmdBuf));

		if(ret <=  0)
		{
			closesocket(CmdSocket);
			return 0;
		}

		ZeroMemory(CmdBuf, BUFFSIZE);
		ret = MyRecv(CmdSocket, CmdBuf, BUFFSIZE);

		if(ret <= 0)
	    {
			closesocket(CmdSocket);
			return 0;
		}

		if(strncmp(CmdBuf, "227", 3) == 0)
		{
			cout << CmdBuf;
		}
		else
		{
			cout << CmdBuf;
			return 0;
		}

		pStr = strchr(CmdBuf, '(');
		pStr1 = pStr ;
		count = 0;
		while(count < 4)
		{
			if(*pStr1 == ',')
			{
				*pStr1 = '.';
				count++;
			}
			pStr1++;
		}
		strncpy(ipstr, pStr + 1, pStr1 - pStr -2);

		pStr = pStr1;

		while(*pStr1 != ',')
			pStr1 ++;
		ZeroMemory(temp, 8);
		strncpy(temp, pStr, pStr1 - pStr);
		hbPort = atoi(temp);

		pStr = pStr1 + 1;
		while(*pStr1 != ')')
			pStr1++;
		ZeroMemory(temp, 8);
		strncpy(temp, pStr, pStr1 - pStr);
		lbPort = atoi(temp);

		nPort = 256 * hbPort + lbPort;
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(nPort);
		serverAddr.sin_addr.S_un.S_addr = inet_addr(ipstr);

		DataSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

		if(DataSocket == INVALID_SOCKET)
		{
			cout << "创建套接字失败" << WSAGetLastError() << endl;
			return 0;
		}

		ret = connect(DataSocket, (sockaddr*)&serverAddr, sizeof(SOCKADDR));

		if(ret == SOCKET_ERROR)
		{
			cout << "数据连接失败" << WSAGetLastError() << endl;
			closesocket(DataSocket);
			return 0;
		}
		return 1;
	}
}

void dir(char* input)
{
	char* pstr = NULL;
	int ret;
	if(DataConnect() == 0)
		return;

	pstr = lspace(input + 3);
	ZeroMemory(CmdBuf, BUFFSIZE);
	strcat(CmdBuf,"LIST ");
	strcat(CmdBuf, pstr);
	strcat(CmdBuf, "\r\n");
	ret = MySend(CmdSocket, CmdBuf, strlen(CmdBuf));
	if(ret <= 0)
	{
		closesocket(CmdSocket);
		closesocket(DataSocket);
		return;
	}

	ZeroMemory(CmdBuf, BUFFSIZE);
	ret = MyRecv(CmdSocket, CmdBuf, BUFFSIZE);
	if(ret <= 0)
	{
		closesocket(CmdSocket);
		closesocket(DataSocket);
		return ;
	}
	if(strncmp(CmdBuf, "150", 3) != 0)
	{
		cout << CmdBuf;
		closesocket(DataSocket);
		return;
	}
	cout << CmdBuf;

	while(1)
	{
		ZeroMemory(DataBuf, BUFFSIZE);
		ret = MyRecv(DataSocket, DataBuf, BUFFSIZE);

		if(ret <=0)
		{
			closesocket(DataSocket);
			break;
		}
		cout << DataBuf;
	}

	ZeroMemory(CmdBuf, BUFFSIZE);
	while (MyRecv(CmdSocket, CmdBuf, BUFFSIZE) > 0)
	{
		cout << CmdBuf;
		ZeroMemory(CmdBuf, BUFFSIZE);
	}

}

void ls(char* input)
{
	char* pstr = NULL;
	int ret;
	if(DataConnect() == 0)
		return;

	pstr = lspace(input + 2);
	ZeroMemory(CmdBuf, BUFFSIZE);
	strcat(CmdBuf,"NLST ");
	strcat(CmdBuf, pstr);
	strcat(CmdBuf, "\r\n");
	ret = MySend(CmdSocket, CmdBuf, strlen(CmdBuf));
	if(ret <= 0)
	{
		closesocket(CmdSocket);
		closesocket(DataSocket);
		return;
	}

	ZeroMemory(CmdBuf, BUFFSIZE);
	ret = MyRecv(CmdSocket, CmdBuf, BUFFSIZE);
	if(ret <= 0)
	{
		closesocket(CmdSocket);
		closesocket(DataSocket);
		return ;
	}
	if(strncmp(CmdBuf, "150", 3) != 0)
	{
		cout << CmdBuf;
		closesocket(DataSocket);
		return;
	}
	cout << CmdBuf;

	while(1)
	{
		ZeroMemory(DataBuf, BUFFSIZE);
		ret = MyRecv(DataSocket, DataBuf, BUFFSIZE);

		if(ret <=0)
		{
			closesocket(DataSocket);
			break;
		}
		cout << DataBuf;
	}

	ZeroMemory(CmdBuf, BUFFSIZE);
	while(MyRecv(CmdSocket, CmdBuf, BUFFSIZE)> 0)
	{
		cout << CmdBuf;
		ZeroMemory(CmdBuf, BUFFSIZE);
	}
}

int MyRecv(SOCKET s, char *buf, int len)     //接收命令或数据
{
	int ret = 0;
	timeval tm;
	tm.tv_sec = 1;
	tm.tv_usec = 0;

	fd_set fdread;
	FD_ZERO(&fdread);
	FD_SET(s, &fdread);

	ret = select(0, &fdread, NULL, NULL, &tm);  //防止套接字处于锁定模式
	if(ret == 0)
		return -2;
	if(ret == SOCKET_ERROR)
		return -1;
	if(FD_ISSET(s, &fdread))
		ret = recv(s, buf, len, 0);
	return ret;
}

int MySend(SOCKET s, char *buf, int len)      //发送命令或数据
{
	int ret = 0;

	timeval tm;
	tm.tv_sec = 1;
	tm.tv_usec = 0;

	fd_set fdwrite;
	FD_ZERO(&fdwrite);
	FD_SET(s, &fdwrite);

	ret = select(0, NULL, &fdwrite, NULL, &tm); //防止套接字处于锁定模式
	if(ret == 0)
		return -2;
	if(ret == SOCKET_ERROR)
		return -1;
	if(FD_ISSET(s, &fdwrite))
		ret = send(s, buf, len, 0);
	return ret;
}

void about()
{
	cout << "------------Ftp客户端-----------" << endl;
	cout<<"支持以下命令:"<<endl;
	cout << "cd\t改变当前目录" << endl << "close\t关闭连接" << endl<<"del\t删除指定文件" << endl;
	cout <<"dir\t列出指定目录文件列表"<<endl<<"get\t下载模式"<<endl<<"about\t帮助" << endl;
	cout << "pasv\t切换到pasv模式" << endl<<"login\t登陆模式"<<endl<<"ls\t列出当前目录列表" << endl;
	cout << "pos\t显示当前目录路径" << endl << "put\t上传模式" << endl<<"quit\t退出系统" <<endl;
	cout <<"mkdir\t创建目录"<<endl<<"rmdir\t删除指定目录" <<endl;
	cout << "-------------------------------" << endl;
}

void get()
{
	int ret;
	int done = 0;
	char* pStr = NULL;
	char temp[LEN] = {0};
	char remotefile[LEN] = {0};  //服务器文件名(可以包含路径)
	char localfile[LEN] = {0};  //本地文件名(可以包含路径)

	cout << "输入下载的文件名:";
	getInput(remotefile, LEN);
	if(remotefile[0] == '\0')
	{
			cout <<"文件名不能为空"<<endl;
			return;
	}
	cout << "输入本地文件路径(如：download\\):";
	getInput(localfile, LEN);

	strcpy(temp, remotefile);
	/*pStr = temp;
	while(*pStr)
	{
		if(*pStr == ':' || *pStr == '\\' || *pStr == '/')
			*pStr = '_';
		pStr++;
	}*/
	if(localfile[0] == '\0')
	{
		strcat(localfile, "downloads\\");
		strcat(localfile, temp);
	}
	else
		strcat(localfile, temp);

	ZeroMemory(CmdBuf, BUFFSIZE);
	strcat(CmdBuf, "SIZE ");
	strcat(CmdBuf, remotefile);
	strcat(CmdBuf, "\r\n");
	ret = MySend(CmdSocket, CmdBuf, strlen(CmdBuf));

	if(ret <= 0)
	{
		closesocket(CmdSocket);
		return;
	}

	ZeroMemory(CmdBuf, BUFFSIZE);
	ret = MyRecv(CmdSocket, CmdBuf, BUFFSIZE);

	if(ret <= 0)
	{
		closesocket(CmdSocket);
		return ;
	}

	if(strncmp(CmdBuf, "213", 3) != 0)
	{
		cout << CmdBuf;
		cout << "找不到该文件" <<endl;
		return;
	}

	else
		download(localfile, remotefile);
}

void download(char* localfile, char* remotefile)
{
	FILE* pFile = NULL;
	int ret;

	pFile = fopen(localfile, "wb+");
	if(pFile == NULL)
	{
		cout <<"不能打开本地文件"<<localfile << endl;
		return;
	}

	if(DataConnect() == 0)
		return;

	ZeroMemory(CmdBuf, BUFFSIZE);
	strcat(CmdBuf, "RETR ");
	strcat(CmdBuf, remotefile);
	strcat(CmdBuf, "\r\n");
	ret = MySend(CmdSocket, CmdBuf, strlen(CmdBuf));
	if(ret <= 0)
	{
		closesocket(CmdSocket);
		closesocket(DataSocket);
		return;
	}
	ZeroMemory(CmdBuf, BUFFSIZE);
	ret = MyRecv(CmdSocket, CmdBuf, BUFFSIZE);
	if(ret <= 0)
	{
		closesocket(CmdSocket);
		closesocket(DataSocket);
		return ;
	}
	if(strncmp(CmdBuf, "150", 3) != 0)
	{
		cout << CmdBuf;
		closesocket(DataSocket);
		return;
	}
	cout << CmdBuf;

	cout <<"正在下载..."<<endl;

	if(PortPasv == PORT)
		Sleep(1000);  //等待数据连接监听线程获得socket
	while(1)
	{
		ZeroMemory(DataBuf, BUFFSIZE);
		ret = MyRecv(DataSocket, DataBuf, BUFFSIZE);

		if(ret <= 0)
		{
			closesocket(DataSocket);
			fclose(pFile);
			pFile = NULL;
			break;
		}
		fwrite(DataBuf, ret, 1, pFile);
		fflush(pFile);  //清除读写缓冲区
	}

	ZeroMemory(CmdBuf, BUFFSIZE);
	while (MyRecv(CmdSocket, CmdBuf, BUFFSIZE) > 0)
	{
		cout << CmdBuf;
		ZeroMemory(CmdBuf, BUFFSIZE);
	}
}


void put()
{
	char localfile[LEN] = {0};
	char remotefile[LEN] = {0};
	char temp[LEN] = {0};
	char* pStr = NULL;

	cout << "输入上传的本地文件:";
	getInput(localfile, LEN);
	if(localfile[0] == '\0')
	{
		cout << "本地文件不能为空" <<endl;
		return;
	}

	strcpy(temp, localfile);
	pStr = temp;
	while(*pStr)
	{
		if(*pStr == ':' || *pStr == '\\' || *pStr == '/')
			*pStr = '_';
		pStr++;
	}

    cout<<"输入上传至服务器的路径（默认为当前目录）:";
	getInput(remotefile, LEN);

	if(remotefile[0] == '\0')
	{
		pStr = localfile + strlen(localfile) -1;
		while(pStr != localfile && *pStr != '\\')
			pStr--;
		strcpy(remotefile, pStr + 1);
	}
	else {
		pStr = localfile + strlen(localfile) - 1;
		while (pStr != localfile && *pStr != '\\')
			pStr--;
		strcat(remotefile, pStr + 1);
	}
	cout << localfile << endl << remotefile << endl;

	upload(localfile, remotefile);

}

void upload(char* localfile, char* remotefile)
{
	FILE* pFile = NULL;
	int ret;

	pFile = fopen(localfile, "rb");
	if(pFile == NULL)
	{
		cout << "不能打开本地文件" <<endl;
		return;
	}

    if(DataConnect() == 0)
		return;
	ZeroMemory(CmdBuf, BUFFSIZE);
	strcat(CmdBuf, "STOR ");
	strcat(CmdBuf, remotefile);
	strcat(CmdBuf, "\r\n");
	ret = MySend(CmdSocket, CmdBuf, strlen(CmdBuf));
	if(ret <= 0)
	{
		closesocket(CmdSocket);
		closesocket(DataSocket);
		return;
	}

	ZeroMemory(CmdBuf, BUFFSIZE);
	ret = MyRecv(CmdSocket, CmdBuf, BUFFSIZE);
	if(ret <= 0)
	{
		closesocket(CmdSocket);
		closesocket(DataSocket);
		return;
	}
	if(strncmp(CmdBuf, "150", 3) != 0)
	{
		cout << CmdBuf;
		closesocket(DataSocket);
		return;
	}
	cout << CmdBuf;

	cout << "正在上传..." << endl;

	while(1)
	{
		ZeroMemory(DataBuf, BUFFSIZE);
		ret = fread(DataBuf, 1, BUFFSIZE, pFile);
		ret =MySend(DataSocket, DataBuf, ret);
		if(ret <= 0)
		{
			closesocket(DataSocket);
			fclose(pFile);
			pFile = NULL;
			break;
		}

	}
	ZeroMemory(CmdBuf, BUFFSIZE);
	while (MyRecv(CmdSocket, CmdBuf, BUFFSIZE) > 0)
	{
		cout << CmdBuf;
		ZeroMemory(CmdBuf, BUFFSIZE);
	}
}
