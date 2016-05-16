#include "pcap.h"

typedef struct EtherH_ *EtherH;
typedef struct ArpH_ *ArpH;
typedef struct IpH_ *IpH;
typedef struct IcmpH_ *IcmpH;
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
	u_long SrcIP;
	u_char DstMAC[6];
	u_long DstIP;
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

struct ArpPacket_ {
	EtherH EthernetHeader; //����֡ͷ
	ArpH ArpHeader; //Arp֡
	u_char Data[18];  //�������
};
