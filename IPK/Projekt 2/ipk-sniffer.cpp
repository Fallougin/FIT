// Author: Artur Suvorkin  
// Login: xsuvor00
// IPK - Počítačové komunikace a sítě
// Projekt: Sniffer paketů (Varianta ZETA)

#include <iostream>             
#include <string>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <sstream>
#include <iomanip>              //RFC3339
#include <chrono>               //RFC3339
#include <ctime>                //RFC3339
#include <pcap.h>               //pcap funkcie
#include <net/ethernet.h>	    //struct ether_header
#include <netinet/ip.h>	        //struct iphdr
#include <netinet/ip_icmp.h>	//struct icmphdr
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet/ether.h>
#include <netinet/tcp.h>        //struct tcphdr
#include <netinet/udp.h>        //struct udphdr
#include <netinet/ip6.h>        //IPV6 packety

bool udp;
bool tcp;
bool icmp;

int port = -1;
int packetsCounter = 1;

std::string interface = "";
std::string filterStructure = "";

// Function lists all available interfaces and uses the pcap_freealldevs () function to release the list allocated by the pcap_findalldevs () function.
void interfacePrint(pcap_if_t *interfaceList)
{
    while (interfaceList->next != NULL) {
        printf("%s\n", interfaceList->name);
        interfaceList = interfaceList->next;
    }
    pcap_freealldevs(interfaceList);
    exit(0);
}

// Function reads all available interfaces using the pcap_findalldevs () function and returns a pointer to the first of them
pcap_if_t *infterfaceLoad()
{
    char errbuff[PCAP_ERRBUF_SIZE];
    pcap_if_t *interfaceList;
    if (pcap_findalldevs(&interfaceList, errbuff) == PCAP_ERROR) {
        fprintf(stderr, "ERROR: No interfaces available.\n");
        exit(1);
    }
    return interfaceList;
}

// Function checks whether the specified input interface is among the available interfaces
void interfaceCheck(std::string needle, pcap_if_t *interfaceList)
{
    bool correct = false;
    while (interfaceList->next != NULL) {
        if (strcmp(needle.c_str(), interfaceList->name) == 0) {
            correct = true;
        }
        interfaceList = interfaceList->next;
    }
    if (correct == false) {
        pcap_freealldevs(interfaceList);
        fprintf(stderr, "ERROR: Invalid interface.\n");
        exit(1);
    }
}

// List of interfaces on which packets can be captured. http://embeddedguruji.blogspot.com/2014/01/pcapfindalldevs-example.html
void printListOfInteface()
{
    char error[PCAP_ERRBUF_SIZE];
    pcap_if_t *interfaces,*temp;
    int i=0;
    if(pcap_findalldevs(&interfaces,error)==-1)
    {
        printf("ERROR: Invalid devices.\n");
    }
    for(temp=interfaces;temp;temp=temp->next)
    {
        printf("%d:  %s\n",i++,temp->name);
    }
    exit(1);
}

// Founction for argument parseing
void argumentParser(int argc, char *argv[])
{
    pcap_if_t *interfaceList = infterfaceLoad();  // List of available interfaces
    std::string arg;

    if (argc == 1) {
        interfacePrint(interfaceList);
    } 

    bool assigned;
    for (int i = 1; i < argc; i++) {
        arg = std::string(argv[i]);
        if (arg == "-i" || arg == "--interface") {
            assigned = true;
            try {
                interface = std::string(argv[i + 1]);
            } catch (std::exception const&) {
                interfacePrint(interfaceList);
            }
            interfaceCheck(interface, interfaceList);
        }
    }
    if (assigned == false) {
        interfacePrint(interfaceList);
    }

    // Passing other input arguments, such as protocol options, number of packets, which will be sniffed or an exclusive port for sniffing
    for (int i = 1; i < argc; i++) {
        arg = std::string(argv[i]);
        if (arg == "-i" || arg == "--interface") {
            i++;  // Skiping specified interface, this is loaded on the first transition
        } else if (arg == "-t" || arg == "--tcp") {
            tcp = true;
        } else if (arg == "-u" || arg == "--udp") {
            udp = true;
        } else if (arg == "--icmp") {
            icmp = true;
        } else if (arg == "-p") {
            try {
                port = std::stoi(std::string(argv[i + 1]));
                i++;
                if (port < 0) {
                    fprintf(stderr, "ERROR: Invalid port number.\n");
                    exit(1);
                }
            } catch (std::exception const&) {
                fprintf(stderr, "ERROR: Invalid port number.\n");
                exit(1);
            }
        } else if (arg == "-n") {
            try {
                packetsCounter = std::stoi(std::string(argv[i + 1]));
                i++;
                if (packetsCounter < 0) {
                    fprintf(stderr, "ERROR: Invalid packet number.\n");
                    exit(1);
                }
            } catch (std::exception const&) {
                fprintf(stderr, "ERROR: Invalid packet number.\n");
                exit(1);
            }
        } else {
            fprintf(stderr, "ERROR: Invalid entry argument.\n");
            exit(1);
        }
    }

    // If none of the possible sniffing protocols have been specified
    if (udp == false && tcp == false && icmp == false) {
        udp = true;
        tcp = true;
        icmp = true;
    }
}

// Function that sets the filter on ​​the basis of input switches so that only those packets that are required are sniffed
void Filter()
{
    std::string portString;
    std::stringstream ss;
    ss << port;
    ss >> portString;

    if (udp) {
        if (port != -1){
            filterStructure += "udp port " + portString + " or ";
        } else {
            filterStructure += "udp or ";
        }
    }
    if (tcp) {
        if (port != -1){
            filterStructure += "tcp port " + portString + " or ";
        } else {
            filterStructure += "tcp or ";
        }
    }
    if (icmp) {
        filterStructure += "icmp or ";
    }

    filterStructure = filterStructure.substr(0, filterStructure.size() - 3); //orezanie "or"
    filterStructure = filterStructure + " or icmp6";  //prijimanie aj ipv6 packetov
}


// Packet content listing function
// https://www.programcreek.com/cpp/?code=mq1n%2FNoMercy%2FNoMercy-master%2FSource%2FClient%2FNM_Engine%2FINetworkScanner.cpp
static void packetPrint(const void *addr, int len) 
{
	int i;
	unsigned char buff[17];
	unsigned char *pc = (unsigned char*)addr;
	for (i = 0; i < len; i++) {
		if ((i % 16) == 0) {
			if (i != 0) {
				printf("  %s\n", buff);
            }
			printf("  %04x ", i);
		}
		printf(" %02x", pc[i]);
		if ((pc[i] < 0x20) || (pc[i] > 0x7e)) {
			buff[i % 16] = '.';
        } else {
			buff[i % 16] = pc[i];
        }
		buff[(i % 16) + 1] = '\0';
	}
	while ((i % 16) != 0) {
		printf("  ");
		++i;
	}
	printf(" %s\n", buff);
}

// Time recording function in required format (RFC3339) with nano seconds
// https://stackoverflow.com/questions/54325137/c-rfc3339-timestamp-with-milliseconds-using-stdchrono
void timePrint()
{
    std::stringstream ss;
    const auto now = std::chrono::system_clock::now();
    const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;
    const auto c_now = std::chrono::system_clock::to_time_t(now);
    ss << std::put_time(std::gmtime(&c_now), "%FT%T") <<  '.' << std::setfill('0') << std::setw(3) << millis << "+02:00 ";
    printf("%s", ss.str().c_str());
}


// IPv6 adress
// https://stackoverflow.com/questions/3727421/expand-an-ipv6-address-so-i-can-print-it-to-stdout 
void PrintIPv6Adress(const struct in6_addr * addr)
{
    printf("%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
    (int)addr->s6_addr[0], (int)addr->s6_addr[1],
    (int)addr->s6_addr[2], (int)addr->s6_addr[3],
    (int)addr->s6_addr[4], (int)addr->s6_addr[5],
    (int)addr->s6_addr[6], (int)addr->s6_addr[7],
    (int)addr->s6_addr[8], (int)addr->s6_addr[9],
    (int)addr->s6_addr[10], (int)addr->s6_addr[11], 
    (int)addr->s6_addr[12], (int)addr->s6_addr[13],
    (int)addr->s6_addr[14], (int)addr->s6_addr[15]);
}

// In addition, it calls all the above auxiliary functions: Lists the time when the packet was sniffed,
// lists its contents encoded in a 16-bit system, if necessary ipv6 address. He can find out the address or port from the displayed packet.
void packetSniffer(u_char *args, const struct pcap_pkthdr *header,  const u_char *packet) 
{
	struct ip *ip;  // IP header
    const struct iphdr *ipHeader;  // IP header
    const struct ip6_hdr *ip6_hdr;  // IPv6 header
    const struct udphdr *udp_hdr;  // UDP packet structure
    const struct tcphdr *tcp_hdr;  // TCP packet structure
    const struct ether_header *ether_header;  // ethernet structure

    ip = (struct ip *)(packet + sizeof(struct ether_header));
    ipHeader = (struct iphdr *)(packet + sizeof(struct ethhdr));
    ip6_hdr = (struct ip6_hdr *)(packet + sizeof(struct ether_header));
    ether_header = (struct ether_header *) packet;

    int iplen = ip->ip_hl*4; // IPv4
    
    timePrint();

    switch(ntohs(ether_header->ether_type)) //htonl, htons, ntohl, ntohs convert values between host and network byte order
    {
        case ETHERTYPE_IP: //IPv4
            switch (ipHeader->protocol)
            {
                case 1: // ICMPv4
                    printf("%s > ", inet_ntoa(ip->ip_src));
                    printf("%s", inet_ntoa(ip->ip_dst));
                    printf(", length %d bytes\n", header->len);

                    packetPrint(packet, header->len);
                    break;
                case 6: // TCP
                    tcp_hdr = (struct tcphdr*)(packet + sizeof(struct ether_header) + iplen);

                    printf("%s : %d > ", inet_ntoa(ip->ip_src), ntohs(tcp_hdr->th_dport));
                    printf("%s : %d", inet_ntoa(ip->ip_dst), ntohs(tcp_hdr->th_sport));
                    printf(", length %d bytes\n", header->len);

                    packetPrint(packet, header->len);
                    break;
                case 17: // UDP
                    udp_hdr = (struct udphdr*)(packet + sizeof(struct ether_header) + iplen);

                    printf("%s : %d > ", inet_ntoa(ip->ip_src), ntohs(udp_hdr->uh_dport));
                    printf("%s : %d", inet_ntoa(ip->ip_dst), ntohs(udp_hdr->uh_sport));
                    printf(", length %d bytes\n", header->len);

                    packetPrint(packet, header->len);
                    break;
                default:
                    // another IPv4 protokol
                    break;
            }
            break;
        case ETHERTYPE_IPV6: // IPv6
            switch(ip6_hdr->ip6_ctlun.ip6_un1.ip6_un1_nxt)
            {
                case 6: // TCP
                    printf("TCP - IPv6\n");
                    break;
                case 17: // UDP
                    printf("UDP - IPv6\n");
                    break;
                case 58: // ICMPv6
                    PrintIPv6Adress(&ip6_hdr->ip6_src);
                    printf(" > ");
                    PrintIPv6Adress(&ip6_hdr->ip6_dst);
                    printf(" , length %d bytes\n", header->len);

                    packetPrint(packet, header->len);
                    break;
                default:
                    // another IPv6 protokol
                    break;
            }
            break;
    }
}

int main(int argc, char **argv)
{
    pcap_t *device;
    bpf_u_int32 ip;
    bpf_u_int32 mask;
    char errbuff[PCAP_ERRBUF_SIZE];
    struct bpf_program filterCompiled;

    argumentParser(argc, argv);
    Filter();

    if ((device = pcap_open_live(interface.c_str(), 65535, 1, 1000, errbuff)) == NULL ) {
        fprintf(stderr, "ERROR: Failure during opening device.\n");
        exit(1);
    }
    if (pcap_lookupnet(interface.c_str(), &ip, &mask, errbuff) == PCAP_ERROR) {
        fprintf(stderr, "ERROR: Can't find network.\n");
        exit(1);
    }

    pcap_close(device); // closing sniffing
    pcap_freecode(&filterCompiled); // free allocated memory
    return 0;
} 