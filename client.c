#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define BUFLEN	512

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

//221.146.135.36

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

	close(sd);
	
	return 0;
}
