#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>


typedef struct header {
	unsigned short id;			// transaction ID

	unsigned char rd : 1;		// recursion desired - default: 1
	unsigned char tc : 1;		// truncated message - if response msg > 512 Bytes, set 1
	unsigned char aa : 1;		// authoritative answer - used for DNS response msg
	unsigned char opcode : 4;	// 0: standard query, 1: Inverse Query, 2: Status, 3: Unassigned...
	unsigned char qr : 1;		// 0: DNS Query, 1: DNS Response

	unsigned char rcode : 4;	// response code - 0: No Error, 1: Format error, 2: ServFail, 3: Non-existent Domain...
	unsigned char reserved: 3;	// reserved - set 000
	unsigned char ra : 1;		// recursion available;

	unsigned short q_count;		// number of question entries
	unsigned short ans_count;	// number of answer entries
	unsigned short auth_count;	// number of authority entries
	unsigned short add_count;	// number of resource entries
} DNS_HEADER;

typedef struct packet {
	DNS_HEADER header;
	unsigned char data[65524];
} PACKET;

typedef struct body {
	unsigned short type;
	unsigned short class;
} BODY;

#pragma pack(1)		// without padding
typedef struct answer {
	unsigned short name;
	unsigned short type;
	unsigned short class;
	unsigned int ttl;
	unsigned short len;
} ANSWER;
#pragma pack(8)

int main(int argc, char* argv[]) {
	if(argc != 2) {
		printf("Usage: ./dns.out <hostname>\n");
		return 1;
	}

	// set packet header
	PACKET packet;
	memset(&packet, 0, sizeof(PACKET));
	packet.header.id = (unsigned short)htons(getpid());
	packet.header.qr = 0;
	packet.header.opcode = 0;
	packet.header.aa = 0; 
	packet.header.tc = 0;
	packet.header.rd = 1;
	packet.header.ra = 0;
	packet.header.reserved = 0;
	packet.header.rcode = 0;
	packet.header.q_count = htons((unsigned short) 1);
	packet.header.ans_count = 0x0000;
	packet.header.auth_count = 0x0000;
	packet.header.add_count = 0x0000;

	// converting hostname into DNS format
	char* hostname = argv[1];
	int idx = 0, cnt = 0, changeidx=0, packetlen;
	for(idx = 0; hostname[idx] != '\0'; idx++) {
		if(hostname[idx]=='.') {
			packet.data[changeidx] = cnt;
			cnt = 0;
			changeidx = idx + 1;
		}
		else {
			cnt++;
			packet.data[idx + 1] = hostname[idx];
		}
	}
	idx += 2;
	packet.data[changeidx] = cnt;
	packet.data[idx++];
	BODY* tail = (BODY*)(packet.data + idx);
	tail->type = 1;					// Type: A
	tail->class = 1;				// Class: IN
	packetlen = sizeof(DNS_HEADER) + idx + sizeof(BODY);

	int sd, port;
	struct sockaddr_in dns_server;

	// create UDP socket
	if((sd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		fprintf(stderr, "Can't create a socket\n");
		exit(1);
	}
	
	bzero((char*)&dns_server, sizeof(struct sockaddr_in));
	dns_server.sin_family = AF_INET;
	dns_server.sin_port = htons(53);
	dns_server.sin_addr.s_addr = inet_addr("8.8.8.8");

	if(connect(sd, (struct sockaddr*)&dns_server, sizeof(dns_server)) == -1) {
		fprintf(stderr, "Can't connect to the server\n");
		exit(1);
	}
	
	for(int i = 0;i < packetlen; i++)
		printf("%02x ",*((unsigned char*)&packet + i));
	
	puts("");

	write(sd, (unsigned char*)&packet, packetlen);

	unsigned char* rbuf[65536];
	memset(rbuf, 0, sizeof(char)*65536);
	read(sd, rbuf, 65536);
	
	/*
	for(int i = 0;i < packetlen; i++)
		printf("%02x ", *((unsigned char*)rbuf + i));
	
	puts("");
	
	
	
	ANSWER* ans = (ANSWER*)((unsigned char*)rbuf + packetlen - 1);
	for(int i = 0; i < sizeof(ANSWER) + 50; i++){
		printf("%02x ",*((unsigned char*)ans + i));
	}
	
	puts("");
	
	in_addr_t* ipaddr_ptr = (in_addr_t*)((unsigned char*)rbuf + packetlen - 1 + sizeof(ANSWER));

	for(int i=0; i < 4; i++){
		printf("%02x ",*((unsigned char*)ipaddr_ptr + i));
	}
	*/

	close(sd);
	
	return 0;
}
