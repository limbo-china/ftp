
#include"pcap.h"
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data);
void InterpEthernet(const struct pcap_pkthdr* packet_header, const u_char* packet_content);
void InterpArp(const u_char* packet_content);
void InterpIp(const u_char* packet_content);
