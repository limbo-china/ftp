#include "pcap.h"

typedef struct EtherH_ *EtherH;
typedef struct ArpH_ *ArpH;
typedef struct IpH_ *IpH;
typedef struct IcmpH_ *IcmpH;
typedef struct TcpH_ *TcpH;
typedef struct psdTcpH_ *psdTcpH;
typedef struct ArpPacket_ *ArpPacket;

struct EtherH_ {
	u_char DstMAC[6];
	u_char SrcMAC[6];
	u_short EType; //物理帧类型
};
struct ArpH_ {
	u_short HardwareType; //硬件类型
	u_short ProtoType; //上层协议类型
	u_char MAClen; //mac地址长度
	u_char IPlen; //ip地址长度
	u_short Flag; //操作类型
	u_char SrcMAC[6];
	u_char SrcIP[4];
	u_char DstMAC[6];
	u_char DstIP[4];
	//u_char CRC[4];
};
struct IpH_ {
	union {
		u_char Version;  //版本号
		u_char HdrLen;	//报头长度
	};
	u_char ServiceType; //服务类型
	u_short TotalLen;
	u_short Id;  //标识
	union {
		u_short Flags; //标志
		u_short Fragoff; //分段偏移
	};
	u_char TimeToLive;
	u_char Protocol;
	u_short HdrChkSum;
	u_long SrcIp;
	u_long DstIp;
	//选项省略
};
struct IcmpH_ {
	u_char Type;  //类型码
	u_char Code; //代码
	u_short Checksum;
	u_short Id;
	u_short Seq;
};
struct TcpH_ {
	u_short SPort;
	u_short DPort;
	u_long Seq;
	u_long Ack;
	u_char LenRes; 
	u_char Flag;    //4位首部长度和6位保留字,6位标志
	u_short Win;
	u_short	ChkSum;
	u_short Urp;
};
struct psdTcpH_ {
	u_long SIp;
	u_long DIp;
	u_char zero;
	u_char protocol;
	u_short TcpLen;
};
struct ArpPacket_ {
	EtherH EthernetHeader; //物理帧头
	ArpH ArpHeader; //Arp帧
	u_char Data[18];  //数据填充
};