// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
//#include "proto.h"
#include "packet.h"

#include "windows.h"

#pragma comment(lib,"wpcap.lib")
#pragma comment(lib,"packet.lib")
#pragma comment(lib,"ws2_32.lib")

const char * SOURCE_IP = "169.254.196.51";
const char * DESTI_IP = "169.254.62.136";

#define N 100 
const char ROUTER_MAC[6] = { 0xf0,0xb4,0x29,0x27,0xd6,0xfa };
const char BROAD_MAC[6] = { 0xff,0xff,0xff,0xff,0xff,0xff };
const char DESTI_MAC[6] = { 0x34, 0xde, 0x1a, 0x70, 0x9c, 0x5c };
const char SOURCE_MAC[6] = { 0x24,0xfd,0x52,0x6c,0x61,0x99 };
#define IP_TYPE 0x0008//ip
#define ARP_TYPE 0x0608 //arp
#define IP_ICMP 1 //icmp

u_short checksum(u_short *buffer, int size);

u_short checksum(u_short *buffer, int size) {
	unsigned int cksum = 0;
	while (size > 1) {
		cksum = cksum + *buffer++;
		size = size - sizeof(u_short);
	}
	if (size)
		cksum = cksum + *((u_char*)buffer);
	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum = cksum + (cksum >> 16);
	return (u_short)(~cksum);
}

void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data);

int main()
{
	pcap_if_t   *alldevs,*d;
	int    i = 0;
	char errbuf[PCAP_ERRBUF_SIZE];
	char adapname[10][50];
	u_char packet[N];
	if (pcap_findalldevs(&alldevs, errbuf) == -1)
	{
		fprintf(stderr, "Error in pcap_findalldevs: %s\n", errbuf);
		exit(1);
	}
	for (d = alldevs;d;d = d->next)
	{
		sprintf(adapname[i], "%s", d->name);
		printf("%d. \nName:\t%s\n", ++i, d->name);             
		if (d->description)
			printf("description:\t(%s)\n", d->description);
		else
			printf(" (No description available)\n");
		int j = 1;
		while (d->addresses) {
			printf("Interface%d\n\tIP:\t%s\n", j++, inet_ntoa(((SOCKADDR_IN*)d->addresses->addr)->sin_addr));
			printf("\tMASK:\t%s\n", inet_ntoa(((SOCKADDR_IN*)d->addresses->netmask)->sin_addr));
			printf("\tBROAD:\t%s\n", inet_ntoa(((SOCKADDR_IN*)d->addresses->broadaddr)->sin_addr));
			d->addresses = d->addresses->next;
		}
		printf("flags:\t%d\n\n",d->flags);
	}
	if (i == 0)
	{
		printf("\nNo interfaces found! \n");
		return 0;
	}

	pcap_t *adhandle = pcap_open_live(adapname[2],65535, 1, 1000, errbuf);


	//arp°ü
	memset(packet,0,sizeof(packet));
	EtherH eh = (EtherH)packet;
	ArpH arph = (ArpH)(packet + sizeof(struct EtherH_));
	int h = sizeof(struct EtherH_) + sizeof(struct ArpH_);
	memset(packet + h, 0, N - h);
	strcpy((char *)eh->DstMAC, DESTI_MAC);
	strcpy((char *)eh->SrcMAC, SOURCE_MAC);
	eh->EType = ARP_TYPE;
	arph->HardwareType = 0x0100;
	arph->ProtoType = IP_TYPE;
	arph->MAClen = 6;
	arph->IPlen = 4;
	arph->Flag = 0x0100;
	strcpy((char *)arph->SrcMAC, SOURCE_MAC);
	arph->SrcIP = inet_addr(SOURCE_IP);
	strcpy((char *)arph->DstMAC, DESTI_MAC);
	arph->DstIP = inet_addr(DESTI_IP);
	i = 0;
	while(i++<5)
		pcap_sendpacket(adhandle, packet, N /* size */);


	pcap_close(adhandle);

	pcap_freealldevs(alldevs);
    return 0;
}
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data)
{
	struct tm *ltime;
	char timestr[16];
	time_t local_tv_sec = header->ts.tv_sec;
	ltime = localtime(&local_tv_sec);
	strftime(timestr, sizeof(timestr), "%H:%M:%S", ltime);

	printf("%s,%.6ld len:%d\n", timestr, header->ts.tv_usec, header->len);

	for (int i = 0;i <header->len;i++)  
			printf("%02x",pkt_data[i]);
	printf("\n");

}

