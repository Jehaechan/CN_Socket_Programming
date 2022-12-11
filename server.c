#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>

#define BUFLEN 512

const char ipaddr[] = "0.0.0.0";

void ipc_with_client(char* buf) {
	int sd, port;
	struct hostent *hp;
	struct sockaddr_in server;

	port = 50061;

	if((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "Can't create a socket\n");
		exit(1);
	}

	bzero((char*)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port=htons(port);
	server.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(connect(sd, (struct sockaddr*)&server, sizeof(server)) == -1) {
		fprintf(stderr, "Can't connect to the server\n");
		exit(1);
	}
	buf[28] = '\0';
	printf("%s\n",buf);

	write(sd, buf, 28);
	close(sd);
}

int main(int argc, char **argv) {
	int bytes_to_read;
	int sd, new_sd, client_len, port;
	struct sockaddr_in server, client;
	char *bp, buf[BUFLEN];
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

	listen(sd, 3);
	int loop = 0;

	while(loop < 5) {
		client_len = sizeof(client);
		if((new_sd = accept(sd, (struct sockaddr*)&client, &client_len))==-1){
			fprintf(stderr, "Can't accept client\n");
			exit(1);
		}
		
		// concurrent server
		if((pid = fork()) < 0){
			fprintf(stderr, "fork() error!\n");
			exit(1);
		}

		// child process
		if(pid == 0) {
			close(sd);
			read(new_sd, buf, 30);
			// send data to local client(IPC)
			ipc_with_client(buf);
			close(new_sd);
			exit(0);
		}
		loop++;

		// parent process
		close(new_sd);
	}
	
	// manage child process
	for(int i = 0; i < 5; i++){
		if(waitpid(-1, NULL, 0) <= 0){
			fprintf(stderr, "wait() error!\n");
			exit(1);
		}
	}
	
	close(sd);
	
	return 0;
}
