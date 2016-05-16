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
	u_short EType; //����֡����
};
struct ArpH_ {
	u_short HardwareType; //Ӳ������
	u_short ProtoType; //�ϲ�Э������
	u_char MAClen; //mac��ַ����
	u_char IPlen; //ip��ַ����
	u_short Flag; //��������
	u_char SrcMAC[6];
	u_char SrcIP[4];
	u_char DstMAC[6];
	u_char DstIP[4];
	//u_char CRC[4];
};
struct IpH_ {
	union {
		u_char Version;  //�汾��
		u_char HdrLen;	//��ͷ����
	};
	u_char ServiceType; //��������
	u_short TotalLen;
	u_short Id;  //��ʶ
	union {
		u_short Flags; //��־
		u_short Fragoff; //�ֶ�ƫ��
	};
	u_char TimeToLive;
	u_char Protocol;
	u_short HdrChkSum;
	u_long SrcIp;
	u_long DstIp;
	//ѡ��ʡ��
};
struct IcmpH_ {
	u_char Type;  //������
	u_char Code; //����
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
	u_char Flag;    //4λ�ײ����Ⱥ�6λ������,6λ��־
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
	EtherH EthernetHeader; //����֡ͷ
	ArpH ArpHeader; //Arp֡
	u_char Data[18];  //�������
};