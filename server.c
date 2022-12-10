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
//const char ipaddr[] = "127.31.117.199";

int main(int argc, char **argv) {
	int bytes_to_read;
	int sd, new_sd, client_len, port;
	struct sockaddr_in server, client;
	char *bp, buf[BUFLEN];
	pid_t pid;
	
	// change port num.
	port = 50060;
	if(argc == 2)
		port = atoi(argv[1]);

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
	//server.sin_addr.s_addr = htonl(INADDR_ANY);
	//inet_pton(AF_INET, ipaddr, &server.sin_addr);
	if(bind(sd, (struct sockaddr*)&server, sizeof(server)) == -1) {
		fprintf(stderr, "Can't bind name to socket\n");
		exit(1);
	}

	listen(sd, 5);

	/*
	while(1) {
		client_len = sizeof(client);
		if((new_sd = accept(sd, (struct sockaddr*)&client, &client_len))==-1){
			fprintf(stderr, "Can't accept client\n");
			exit(1);
		}

		if((pid = fork()) < 0){
			fprintf(stderr, "fork() error!\n");
			exit(1);
		}
		
		// child process
		if(pid == 0) {
			close(sd);
			memset(buf, '\0', sizeof(char)*BUFLEN);
			read(new_sd, buf, BUFLEN);
			printf("%s\n",buf);
			close(new_sd);
			exit(0);
		}

		close(new_sd);
	}
	*/
	
	while(1) {
		printf("loop\n");
		client_len = sizeof(client);
		if((new_sd = accept(sd, (struct sockaddr*)&client,&client_len))==-1){
			fprintf(stderr, "Can't accept client\n");
			exit(1);
		}
		printf("loop2\n");
		memset(buf, '\0', sizeof(char)*BUFLEN);
		read(new_sd, buf, BUFLEN);
		printf("%s\n",buf);
		close(new_sd);
	}
	close(sd);
	
	return 0;
}
