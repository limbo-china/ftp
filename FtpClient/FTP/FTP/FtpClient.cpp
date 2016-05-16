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

char ServerIp[LEN] = { 0 };      //������IP
int PortPasv = PASV;              //0-PORT,1-PASV
SOCKET CmdSocket;					    //��������
SOCKET DataSocket;					//��������
char CmdBuf[BUFFSIZE]   = {0};		//���ƻ�����
char DataBuf[BUFFSIZE]  = {0};      //�������

int parseCmd(char* str);                     //��������
void getInput(char *input, int len);           //�õ��û������룬��ȥ���ײ���β���Ŀհ׷�
char* lspace(char * input);					   //ȥ�ײ��հ�
char* rspace(char * input);                     //ȥβ���հ�

int CreateCmdSocket();                           //���������׽���
int MyRecv(SOCKET s, char *buf, int len);    //����,-2��ʾ�׽����ϲ��������ݣ�-1��ʾSOCKET_ERROR��0��ʾ���ӶϿ�,����0��ʾ���յ��ֽ���
int MySend(SOCKET s, char *buf, int len);     //����,-2��ʾ�������׽�����д���ݣ�-1��ʾSOCKET_ERROR������0��ʾ���͵��ֽ���
int  DataConnect();                     //������������

void login();					               //��½
void close();					               //�Ͽ�
void about();                                   //����
void get();									   //����ģʽ
void put();                                    //�ϴ�ģʽ
void pos();						   //��ǰĿ¼
void mkdir(char*);                             //����Ŀ¼
void rmdir(char*);							   //ɾ��Ŀ¼
void cd(char*);                         //�ı䵱ǰĿ¼
void del(char*);                           //ɾ��ָ���ļ�
void dir(char*);                           //�г�Ŀ¼���ļ�
void ls(char*);                           //�г���������ǰĿ¼���ļ�

void download(char*, char*);                   //����
void upload(char*, char*);                     //�ϴ�


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
		cout << "���ݼ���ʧ��" <<endl;
		return -1;
	}
	listenSock = accept(DataSocket, (sockaddr*)&addrFrom, &len);
	if(listenSock == INVALID_SOCKET)
	{
		cout << "������������ʧ��" <<endl;
		return -1;
	}
	*s = listenSock;            //��listenSock����DataSocket
	return 0;
}

int main()
{
	int ret;
	WSADATA wsaData;
	char input[BUFFSIZE] ={0};

	if(_access("downloads",0)==-1)				//�жϴ���downloadsĿ¼
	{
		CreateDirectory("downloads",NULL);
	}

	ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if(ret != 0)
	{
		cout << "�����׽��ֿ�ʧ��" <<endl;
		return -1;
	}

	if(LOBYTE(wsaData.wVersion) != 2 ||
		HIBYTE(wsaData.wVersion) != 2)
	{
		cout << "�׽��ֿ�汾��һ��" <<endl;
		WSACleanup();
		return -1;
	}
	if(CreateCmdSocket() == 0)
		return -1;
	about();
	while(1)
	{
		cout << "Cmd>> ";
		getInput(input, BUFFSIZE);   //�õ�������ַ�����ȥ����β�հ׷�
		ret = parseCmd(input);   //������������з�����Ӧ����������
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
			cout << "���˳�ϵͳ"<<endl;
			closesocket(CmdSocket);
			WSACleanup();
			return 0;
		case MODEPORT:
			PortPasv = PORT;
			cout<<"��ת����PORTģʽ"<<endl;
			break;
		case MODEPASV:
			PortPasv = PASV;
			cout<<"��ת����PASVģʽ"<<endl;
			break;
		default:
			cout << "�������" <<endl;
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
		cout << "�����׽���ʧ��" << endl;
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

	cout << "���������IP��ַ: ";
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
			cout << "��������" << ServerIp << ",���ȶϿ�����" <<endl;
			return;
		}
		else if(ret == WSAENOTSOCK)
		{
			if(CreateCmdSocket() == 0)
				return;
			if(connect(CmdSocket, (sockaddr*)&serverAddr, sizeof(SOCKADDR)) == SOCKET_ERROR)
			{
				cout << "����ʧ��" <<  WSAGetLastError() << endl;
				return;
			}
		}
		else if (ret == WSAETIMEDOUT)
		{
			cout << "���ӳ�ʱ������������" <<  WSAGetLastError() << endl;
			return ;
		}
		else if(ret == WSAENETUNREACH)
		{
			cout << "Ŀ�ĵ�ַ���ɴ����������" << WSAGetLastError() << endl;
			return;
		}
		else {
			cout << "����ʧ��" << WSAGetLastError() << endl;
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
		cout << "���ӳɹ�\n�������û���:";
	}
	else
	{
		cout << CmdBuf ;
		cout << "����ʧ��" << endl;
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
		cout << "����������:";
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
		cout << "�������"<<endl;
		closesocket(CmdSocket);
		return;
	}

	if(strncmp(CmdBuf, "230", 3) == 0)
	{
		cout << CmdBuf ;
		cout << "����ɹ�" <<endl;
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
		cout << "�ѶϿ�����" <<endl;
		closesocket(CmdSocket);
	}
	else
	{
		cout << CmdBuf;
		cout << "�Ͽ����ӳ���" << endl;
	}
}
void getInput(char *input, int len) //ȥ����β�հ�
{
	char* temp = new char[len];
	char* pstr = NULL;

	cin.getline(temp, len);
	pstr = lspace(temp);
	pstr = rspace(pstr);

	strcpy(input, pstr);
	delete[] temp;
	temp = NULL;

	if(!cin)               //��ջ�����
	{
		cin.clear();
		while(cin.get() != '\n')
			continue;
	}
}

char* lspace(char * input)  //ȥ���ײ��հ�
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

char* rspace(char* input) //ȥ��β���հ�
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
		cout << "δ����������������" << endl;
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
		cout<<"�����½���Ŀ¼��:";
		getInput(temp, BUFFSIZE);
		if(temp[0] == '\0')
		{
			cout << "Ŀ¼������Ϊ��"<<endl;
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
		cout << "δ����" << endl;
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
		cout<<"����Ҫɾ����Ŀ¼��:";
		getInput(temp, BUFFSIZE);
		if(temp[0] == '\0')
		{
			cout << "Ŀ¼������Ϊ��"<<endl;
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
		cout<<"�����л�����Ŀ¼��:";
		getInput(temp, BUFFSIZE);
		if(temp[0] == '\0')
		{
			cout << "Ŀ¼������Ϊ��"<<endl;
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
		cout<<"������Ҫɾ�����ļ���:";
		getInput(temp, BUFFSIZE);
		if(temp[0] == '\0')
		{
			cout << "�ļ�������Ϊ��."<<endl;
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
			cout << "�����׽���ʧ��" << endl;
			return 0;
		}

		localAddr.sin_family = AF_INET;
		localAddr.sin_port = htons(0);                              //��ȡϵͳ�˿ں�
		localAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");    //���ص�ַ

		ret = bind(DataSocket, (sockaddr *)&localAddr, sizeof(SOCKADDR));
		if(ret == SOCKET_ERROR)
		{
			cout << "���׽���ʧ��" <<endl;
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
			cout << "�����׽���ʧ��" << WSAGetLastError() << endl;
			return 0;
		}

		ret = connect(DataSocket, (sockaddr*)&serverAddr, sizeof(SOCKADDR));

		if(ret == SOCKET_ERROR)
		{
			cout << "��������ʧ��" << WSAGetLastError() << endl;
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

int MyRecv(SOCKET s, char *buf, int len)     //�������������
{
	int ret = 0;
	timeval tm;
	tm.tv_sec = 1;
	tm.tv_usec = 0;

	fd_set fdread;
	FD_ZERO(&fdread);
	FD_SET(s, &fdread);

	ret = select(0, &fdread, NULL, NULL, &tm);  //��ֹ�׽��ִ�������ģʽ
	if(ret == 0)
		return -2;
	if(ret == SOCKET_ERROR)
		return -1;
	if(FD_ISSET(s, &fdread))
		ret = recv(s, buf, len, 0);
	return ret;
}

int MySend(SOCKET s, char *buf, int len)      //�������������
{
	int ret = 0;

	timeval tm;
	tm.tv_sec = 1;
	tm.tv_usec = 0;

	fd_set fdwrite;
	FD_ZERO(&fdwrite);
	FD_SET(s, &fdwrite);

	ret = select(0, NULL, &fdwrite, NULL, &tm); //��ֹ�׽��ִ�������ģʽ
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
	cout << "------------Ftp�ͻ���-----------" << endl;
	cout<<"֧����������:"<<endl;
	cout << "cd\t�ı䵱ǰĿ¼" << endl << "close\t�ر�����" << endl<<"del\tɾ��ָ���ļ�" << endl;
	cout <<"dir\t�г�ָ��Ŀ¼�ļ��б�"<<endl<<"get\t����ģʽ"<<endl<<"about\t����" << endl;
	cout << "pasv\t�л���pasvģʽ" << endl<<"login\t��½ģʽ"<<endl<<"ls\t�г���ǰĿ¼�б�" << endl;
	cout << "pos\t��ʾ��ǰĿ¼·��" << endl << "put\t�ϴ�ģʽ" << endl<<"quit\t�˳�ϵͳ" <<endl;
	cout <<"mkdir\t����Ŀ¼"<<endl<<"rmdir\tɾ��ָ��Ŀ¼" <<endl;
	cout << "-------------------------------" << endl;
}

void get()
{
	int ret;
	int done = 0;
	char* pStr = NULL;
	char temp[LEN] = {0};
	char remotefile[LEN] = {0};  //�������ļ���(���԰���·��)
	char localfile[LEN] = {0};  //�����ļ���(���԰���·��)

	cout << "�������ص��ļ���:";
	getInput(remotefile, LEN);
	if(remotefile[0] == '\0')
	{
			cout <<"�ļ�������Ϊ��"<<endl;
			return;
	}
	cout << "���뱾���ļ�·��(�磺download\\):";
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
		cout << "�Ҳ������ļ�" <<endl;
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
		cout <<"���ܴ򿪱����ļ�"<<localfile << endl;
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

	cout <<"��������..."<<endl;

	if(PortPasv == PORT)
		Sleep(1000);  //�ȴ��������Ӽ����̻߳��socket
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
		fflush(pFile);  //�����д������
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

	cout << "�����ϴ��ı����ļ�:";
	getInput(localfile, LEN);
	if(localfile[0] == '\0')
	{
		cout << "�����ļ�����Ϊ��" <<endl;
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

    cout<<"�����ϴ�����������·����Ĭ��Ϊ��ǰĿ¼��:";
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
		cout << "���ܴ򿪱����ļ�" <<endl;
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

	cout << "�����ϴ�..." << endl;

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
