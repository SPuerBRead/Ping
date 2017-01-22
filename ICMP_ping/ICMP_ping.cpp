// ICMP_ping.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "winsock2.h"
#include "stdint.h"
#include "time.h"
#define ICMP_ECHOREPLY 0
#define ICMP_ECHOREQ 8
#define ICMP_ECHO 8
#pragma comment(lib,"ws2_32.lib")

typedef uint8_t u_int8_t; //1字节
typedef uint16_t u_int16_t; //2
typedef uint32_t u_int32_t;  //3


unsigned short in_cksum(unsigned short *ptr, int nbytes);
struct icmp_header
{
	u_int8_t type;
	u_int8_t code;
	u_int16_t checksum;
	union 
	{
		struct
		{
			u_int16_t id;
			u_int16_t sequence;
		}echo;
		u_int32_t gateway;
		struct
		{
			u_int16_t __unused;
			u_int16_t mtu;
		}frag;
	}un;
};

int main()
{
	char *packet, *data = NULL;
	SOCKET sock;
	struct icmp_header *header = NULL;
	int data_size = 200, packet_size;

	WSADATA wsaData;
	DWORD dwError;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		fprintf(stderr, "WSAStartup() failed");
		exit(0);
	}

	sockaddr_in addr;

	addr.sin_family = AF_INET;
	char *ip = "119.29.62.29";
	addr.sin_addr.S_un.S_addr = inet_addr(ip);
	addr.sin_port = htons(0);

	if ((sock = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP)) == SOCKET_ERROR)
	{
		printf("socket fail");
		exit(0);
	}

	packet_size = sizeof(struct icmp_header) + data_size;
	packet = (char *)malloc(packet_size);
	memset(packet, 0, packet_size);

	header = (struct icmp_header *)packet;
	header->type = ICMP_ECHO;
	header->code = 0;
	header->un.echo.sequence = rand();
	header->un.echo.id = rand();

	data = packet + sizeof(struct icmp_header);
	memset(data, 'a', data_size);
	header->checksum = in_cksum((unsigned short *)header, packet_size);

	char recvbuffer[1024];
	sockaddr_in recvaddr;
	int addrlen = sizeof(recvaddr);
	int times[10] = { 0 };
	int count = 0;
	int recvbyte;
	while (count < 10)
	{
		clock_t startTime = clock();
		clock_t delta;
		if (sendto(sock, packet, packet_size, 0, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR)
		{
			printf("ERROR:", WSAGetLastError());
			break;
		}
		if ((recvbyte = recvfrom(sock, recvbuffer, sizeof(recvbuffer), 0, (struct sockaddr *)&recvaddr, &addrlen)) == SOCKET_ERROR)
		{
			printf("ERROR:", WSAGetLastError());
			break;
		}
		if (strcmp(inet_ntoa(recvaddr.sin_addr),ip))
		{
			printf("%s\r\n", inet_ntoa(recvaddr.sin_addr));
			printf("%s\r\n", ip);
			printf("result error\r\n");
			break;
		}
		else
		{
			delta = clock() - startTime;
			int rtt = delta * 1000 / CLOCKS_PER_SEC;
			times[count] = rtt;
			count++;
			printf("来自%s的回复：大小为%ld bytes，时间%dms\r\n", inet_ntoa(recvaddr.sin_addr), (long)recvbyte, rtt);
		}
	}
    return 0;
}


unsigned short in_cksum(unsigned short *ptr, int nbytes)
{
	register long sum;
	u_short oddbyte;
	register u_short answer;

	sum = 0;
	while (nbytes > 1) {
		sum += *ptr++;
		nbytes -= 2;
	}

	if (nbytes == 1) {
		oddbyte = 0;
		*((u_char *)& oddbyte) = *(u_char *)ptr;
		sum += oddbyte;
	}

	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	answer = ~sum;

	return (answer);
}
