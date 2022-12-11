#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFLEN 512

const char ipaddr[] = "0.0.0.0";

int main(int argc, char **argv) {
	int bytes_to_read;
	int sd, new_sd, client_len, port;
	struct sockaddr_in server, client;
	char *bp, buf[BUFLEN];
	buf[31]='\0';
	pid_t pid;
	
	port = 50060;

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
	server.sin_addr.s_addr = inet_addr(ipaddr);
	if(bind(sd, (struct sockaddr*)&server, sizeof(server)) == -1) {
		fprintf(stderr, "Can't bind name to socket\n");
		exit(1);
	}

	listen(sd, 2);
	int loop = 0;

	while(loop < 5) {
		client_len = sizeof(client);
		if((new_sd = accept(sd, (struct sockaddr*)&client, &client_len))==-1){
			fprintf(stderr, "Can't accept client\n");
			exit(1);
		}

		if((pid = fork()) < 0){
			fprintf(stderr, "fork() error!\n");
			exit(1);
		}
		loop++;
		
		// child process
		if(pid == 0) {
			close(sd);
			read(new_sd, buf, 30);
			printf("%s",buf);
			close(new_sd);
			exit(0);
		}

		close(new_sd);
	}
	
	close(sd);
	
	return 0;
}
