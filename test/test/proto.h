//#ifndef _PROTO_H
//#define _PROTO_H
//
//#include "pcap.h"
//
//void ethernet_protocol_packet_callback(u_char *argument, const struct pcap_pkthdr* packet_header, const u_char* packet_content);
//void ip_protool_packet_callback(u_char *argument, const struct pcap_pkthdr* packet_header, const u_char* packet_content);
//void tcp_protool_packet_callback(u_char *argument, const struct pcap_pkthdr* packet_header, const u_char* packet_content);
//void udp_protool_packet_callback(u_char *argument, const struct pcap_pkthdr* packet_header, const u_char* packet_content);
//
////以太网协议头
//struct ether_header
//{
//	u_int8_t ether_dhost[6]; //目的Mac地址
//	u_int8_t ether_shost[6]; //源Mac地址
//	u_int16_t ether_type;    //协议类型
//};
//
////IPv4协议头
//struct ip_header
//{
//#if defined(WORDS_BIENDIAN)
//	u_int8_t    ip_version : 4, ip_header_length : 4;
//#else
//	u_int8_t    ip_header_length :4 , ip_version :4;
//#endif
//	u_int8_t    ip_tos;
//	u_int16_t   ip_length;
//	u_int16_t   ip_id;
//	u_int16_t   ip_off;
//	u_int8_t    ip_ttl;
//	u_int8_t    ip_protocol;
//	u_int16_t   ip_checksum;
//	struct in_addr ip_souce_address;
//	struct in_addr ip_destination_address;
//};
//
////UDP协议头
//struct udphdr
//{
//	u_int16_t source_port; /*源地址端口*/
//	u_int16_t dest_port;    /*目的地址端口*/
//	u_int16_t len;     /*UDP长度*/
//	u_int16_t check;   /*UDP校验和*/
//};
//
////TCP协议头
//#define __LITTLE_ENDIAN_BITFIELD
//struct tcphdr
//{
//	u_int16_t   source_port;         /*源地址端口*/
//	u_int16_t   dest_port;           /*目的地址端口*/
//	u_int32_t   seq;            /*序列号*/
//	u_int32_t   ack_seq;        /*确认序列号*/
//#if defined(__LITTLE_ENDIAN_BITFIELD)
//	u_int16_t res1 : 4,   /*保留*/
//		doff : 4,				/*偏移*/
//		fin : 1,              /*关闭连接标志*/
//		syn : 1,              /*请求连接标志*/
//		rst : 1,              /*重置连接标志*/
//		psh : 1,              /*接收方尽快将数据放到应用层标志*/
//		ack : 1,              /*确认序号标志*/
//		urg : 1,              /*紧急指针标志*/
//		ece : 1,              /*拥塞标志位*/
//		cwr : 1;              /*拥塞标志位*/
//#elif defined(__BIG_ENDIAN_BITFIELD)
//	u_int16_t doff : 4,   /*偏移*/
//		res1 : 4,             /*保留*/
//		cwr : 1,              /*拥塞标志位*/
//		ece : 1,              /*拥塞标志位*/
//		urg : 1,              /*紧急指针标志*/
//		ack : 1,              /*确认序号标志*/
//		psh : 1,              /*接收方尽快将数据放到应用层标志*/
//		rst : 1,              /*重置连接标志*/
//		syn : 1,              /*请求连接标志*/
//		fin : 1;              /*关闭连接标志*/
//#else
//	u_int16_t	flag;
//#endif 
//	u_int16_t   window;         /*滑动窗口大小*/
//	u_int16_t   check;          /*校验和*/
//	u_int16_t   urg_ptr;        /*紧急字段指针*/
//};

//以太网协议分析
//void ethernet_protocol_packet_callback(u_char *argument, const struct pcap_pkthdr* packet_header, const u_char* packet_content)
//{
//	u_short ethernet_type;
//	struct ether_header *ethernet_protocol;
//	u_char *mac_string;
//	static int packet_number = 1;
//	printf("----------------------------------------------\n");
//	printf("捕获第%d个网络数据包\n", packet_number);
//	printf("捕获时间:%d\n", packet_header->ts.tv_sec);
//	printf("数据包长度:%d\n", packet_header->len);
//	printf("---------以太网协议---------\n");
//	ethernet_protocol = (struct ether_header*)packet_content;//获得数据包内容
//	ethernet_type = ntohs(ethernet_protocol->ether_type);//获得以太网类型
//	printf("以太网类型:%04x\n", ethernet_type);
//	switch (ethernet_type)
//	{
//	case 0x0800: printf("上层协议是IPv4协议\n");break;
//	case 0x0806: printf("上层协议是ARP协议\n");break;
//	case 0x8035: printf("上层协议是RARP协议\n");break;
//	case 0x814C: printf("上层协议是简单网络管理协议SNMP\n");break;
//	case 0x8137: printf("上层协议是因特网包交换（IPX：Internet Packet Exchange）\n");break;
//	case 0x86DD: printf("上层协议是IPv6协议\n");break;
//	case 0x880B: printf("上层协议是点对点协议（PPP：Point-to-Point Protocol）\n");break;
//	default:break;
//	}
//
//	mac_string = ethernet_protocol->ether_shost;
//	printf("MAC帧源地址:%02x:%02x:%02x:%02x:%02x:%02x\n", *mac_string, *(mac_string + 1), *(mac_string + 2), *(mac_string + 3), *(mac_string + 4), *(mac_string + 5));
//	mac_string = ethernet_protocol->ether_dhost;
//	printf("MAC帧目的地址:%02x:%02x:%02x:%02x:%02x:%02x\n", *mac_string, *(mac_string + 1), *(mac_string + 2), *(mac_string + 3), *(mac_string + 4), *(mac_string + 5));
//	if (ethernet_type == 0x0800)//继续分析IP协议
//	{
//		ip_protool_packet_callback(argument, packet_header, packet_content + sizeof(ether_header));
//	}
//	printf("----------------------------------------------\n");
//	packet_number++;
//}
//
////IP协议分析
//void ip_protool_packet_callback(u_char *argument, const struct pcap_pkthdr* packet_header, const u_char* packet_content)
//{
//	struct ip_header *ip_protocol;
//	u_int header_length = 0;
//	u_int offset;
//	u_char tos;
//	u_int16_t checksum;
//
//	ip_protocol = (struct ip_header *)packet_content;
//	checksum = ntohs(ip_protocol->ip_checksum);
//	tos = ip_protocol->ip_tos;
//	offset = ntohs(ip_protocol->ip_off);
//	printf("---------IP协议---------\n");
//	printf("版本号:%d\n", ip_protocol->ip_version);
//	printf("首部长度:%d\n", header_length);
//	printf("服务质量:%d\n", tos);
//	printf("总长度:%d\n", ntohs(ip_protocol->ip_length));
//	printf("标识:%d\n", ntohs(ip_protocol->ip_id));
//	printf("偏移:%d\n", (offset & 0x1fff) * 8);
//	printf("生存时间:%d\n", ip_protocol->ip_ttl);
//	printf("协议类型:%d\n", ip_protocol->ip_protocol);
//
//	printf("检验和:%d\n", checksum);
//	printf("源IP地址:%s\n", inet_ntoa(ip_protocol->ip_souce_address));
//	printf("目的地址:%s\n", inet_ntoa(ip_protocol->ip_destination_address));
//
//	switch (ip_protocol->ip_protocol)
//	{
//	case 1: printf("上层协议是ICMP协议\n");break;
//	case 2: printf("上层协议是IGMP协议\n");break;
//	case 6:
//	{
//		//printf("上层协议是TCP协议\n");
//		tcp_protool_packet_callback(argument, packet_header, packet_content + sizeof(ip_header));
//	}
//	break;
//
//	case 17:
//	{
//		//printf("上层协议是UDP协议\n");
//		udp_protool_packet_callback(argument, packet_header, packet_content + sizeof(ip_header));
//	}
//	break;
//	default:break;
//	}
//
//}
//
////TCP协议分析
//void tcp_protool_packet_callback(u_char *argument, const struct pcap_pkthdr* packet_header, const u_char* packet_content)
//{
//	struct tcphdr *tcp_protocol;
//	u_int header_length = 0;
//	u_int offset;
//	u_char tos;
//	u_int16_t checksum;
//
//	tcp_protocol = (struct tcphdr *) packet_content;
//	checksum = ntohs(tcp_protocol->check);
//
//	printf("---------TCP协议---------\n");
//	printf("源端口:%d\n", ntohs(tcp_protocol->source_port));
//	printf("目的端口:%d\n", ntohs(tcp_protocol->dest_port));
//	printf("SEQ:%d\n", ntohl(tcp_protocol->seq));
//	printf("ACK SEQ:%d\n", ntohl(tcp_protocol->ack_seq));
//	printf("check:%d\n", checksum);
//
//
//	if (ntohs(tcp_protocol->source_port) == 80 || ntohs(tcp_protocol->dest_port) == 80)
//	{
//		//http协议
//		printf("http data:\n%s\n", packet_content + sizeof(tcphdr));
//	}
//
//}
//
////UDP协议分析
//void udp_protool_packet_callback(u_char *argument, const struct pcap_pkthdr* packet_header, const u_char* packet_content)
//{
//	struct udphdr *udp_protocol;
//	u_int header_length = 0;
//	u_int16_t checksum;
//
//	udp_protocol = (struct udphdr *) packet_content;
//	checksum = ntohs(udp_protocol->check);
//
//	u_int16_t source_port; /*源地址端口*/
//	u_int16_t dest_port;    /*目的地址端口*/
//	u_int16_t len;     /*UDP长度*/
//	u_int16_t check;   /*UDP校验和*/
//
//	printf("---------UDP协议---------\n");
//	printf("源端口:%d\n", udp_protocol->source_port);
//	printf("目的端口:%d\n", udp_protocol->dest_port);
//	printf("len:%d\n", udp_protocol->len);
//	printf("check:%d\n", checksum);
//
//}
#endif