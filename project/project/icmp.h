#define ICMP_ECHO 8
#define ICMP_REPLY 0
#define ICMP_MIN 8 //最小icmp报文长(报头长
#define MAX_PING_PACKET_SIZE (MAX_PACKET_SIZE+sizeof(struct IpH_))
#define DEF_PACKET_SIZE 32
#define MAX_PACKET_SIZE 1024

long ThreadNumCounter = 0, ThreadNumLimit = 30;
long *aa = &ThreadNumCounter;
DWORD WINAPI Ping(LPVOID IPAddr);

WSADATA wsadata;
SOCKET sockRaw;
int livenum = 0;
struct sockaddr_in from;
char icmp[MAX_PACKET_SIZE];
char recvbuf[MAX_PING_PACKET_SIZE];