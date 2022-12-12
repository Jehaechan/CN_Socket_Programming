#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define BUFLEN	512

/* DNS */
typedef struct header {
	unsigned short id;			// transaction ID

	unsigned char rd : 1;		// recursion desired - default: 1
	unsigned char tc : 1;		// truncated message - if response msg > 512 Bytes, set 1
	unsigned char aa : 1;		// authoritative answer - used for DNS response msg
	unsigned char opcode : 4;	// 0: standard query, 1: Inverse Query, 2: Status, 3: Unassigned...
	unsigned char qr : 1;		// 0: DNS Query, 1: DNS Response

	unsigned char rcode : 4;	// response code - 0: No Error, 1: Format error, 2: ServFail, 3: Non-existent Domain...
	unsigned char reserved : 3;	// reserved - set 000
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

in_addr_t DNS(char* hostname) {
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
	packet.header.q_count = htons((unsigned short)1);
	packet.header.ans_count = 0x0000;
	packet.header.auth_count = 0x0000;
	packet.header.add_count = 0x0000;

	// converting hostname into DNS format
	int idx = 0, cnt = 0, changeidx = 0, packetlen;
	for (idx = 0; hostname[idx] != '\0'; idx++) {
		if (hostname[idx] == '.') {
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
	if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		fprintf(stderr, "Can't create a socket\n");
		exit(1);
	}

	bzero((char*)& dns_server, sizeof(struct sockaddr_in));
	dns_server.sin_family = AF_INET;
	dns_server.sin_port = htons(53);
	dns_server.sin_addr.s_addr = inet_addr("8.8.8.8");

	if (connect(sd, (struct sockaddr*) & dns_server, sizeof(dns_server)) == -1) {
		fprintf(stderr, "Can't connect to the server\n");
		exit(1);
	}

	for (int i = 0;i < packetlen; i++)
		printf("%02x ", *((unsigned char*)& packet + i));

	puts("");

	write(sd, (unsigned char*)& packet, packetlen);

	unsigned char* rbuf[65536];
	memset(rbuf, 0, sizeof(char) * 65536);
	read(sd, rbuf, 65536);

	in_addr_t* ipaddr_ptr = (in_addr_t*)((unsigned char*)rbuf + packetlen - 1 + sizeof(ANSWER));

	close(sd);

	return *ipaddr_ptr;
}
/* DNS fin */

/* Socket programming */
// read socket buffer and print stdout
void read_buf_and_print(int sd, char* bp, int bytes_to_read) {
	char* temp = bp;
	int n;
	bp[bytes_to_read] = '\0';
	while((n = read(sd, bp, bytes_to_read)) > 0) {
		bp += n;
		bytes_to_read -= n;
	}
	printf("%s",temp);
}

void ipc_with_server(char* ipc) {
	int n, bytes_to_read;
	int sd, new_sd, port, client_len;
	struct hostent *hp;
	struct sockaddr_in server, client;

	port = 50061;

	// create TCP socket
	if((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "Can't create a socket\n");
		exit(1);
	}
	// reuse port
	int option = 1;
	setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

	// set socket address
	bzero((char*)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(sd, (struct sockaddr*)&server, sizeof(server)) == -1) {
		fprintf(stderr, "Can't bind name to socket\n");
		exit(1);
	}

	listen(sd, 5);

	for(int i = 0; i < 5; i++){
		client_len = sizeof(client);
		if((new_sd = accept(sd, (struct sockaddr*)&client, &client_len)) == -1){
			fprintf(stderr, "Can't accept cilent\n");
			exit(1);
		}
		char* bp = ipc + 29 * i;	// for each socket, read 28 bytes and add ',' to ipc buffer
		bytes_to_read = 28;
		while((n = read(new_sd, bp, bytes_to_read)) > 0) {
			bp += n;
			bytes_to_read -= n;
		}
		close(new_sd);
		bp[0] =',';
	}
	close(sd);
}


// recv data size
int recv_data[14] = {10, 214, 12, 65, 12, 83, 12, 106, 12, 85, 12, 148, 13, 189};

int main(int argc, char* argv[]) {
	int		bytes_to_read;
	int		sd, port;
	struct	hostent *hp;
	struct	sockaddr_in server;
	char	*host, rbuf[BUFLEN], sbuf[BUFLEN];

	// set server address and #port
	host = "2022fcn.ddns.net";
	port = 50000;

	// create TCP socket
	if((sd = socket(AF_INET, SOCK_STREAM, 0)) == - 1) {
		fprintf(stderr, "Can't create a socket\n");
		exit(1);
	}

	// set socket address
	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port=htons(port);								// host to server(big endian)
	if ((hp = gethostbyname(host)) == NULL) {					// domain name -> hostent info
		fprintf(stderr, "Can't get server's address\n");
		exit(1);
	}
	bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);	// copy IP addresss

	// connecting to the server
	if(connect(sd,(struct sockaddr *)&server, sizeof(server)) == -1) {
		fprintf(stderr, "Can't connect to the server\n");
		exit(1);
	}
	
	int recv_idx = 0;
	for(int i = 0; i < 6; i++) {
		// recv data
		read_buf_and_print(sd, rbuf, recv_data[recv_idx++]);
		read_buf_and_print(sd, rbuf, recv_data[recv_idx++]);
		// send data
		int data_size = read(0, sbuf, BUFLEN);
		write(sd, sbuf, data_size);
	}
	read_buf_and_print(sd, rbuf, recv_data[recv_idx++]);
	
	// recv data from local server(IPC)
	char ipc_buf[145];
	ipc_with_server(ipc_buf);
	ipc_buf[144] = '\n';
	
	// send data to HIT server
	write(sd, ipc_buf, 145);

	// recv result from HIT server
	read_buf_and_print(sd, rbuf, recv_data[recv_idx]);

	close(sd);
	
	return 0;
}
