#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>

int main(int argc, char *argv[]){
	struct sockaddr_in server, client;
	int sock, insize;
	char messagein[5000];
	sock = socket(AF_INET, SOCK_STREAM, 0);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(*argv[1]);
	if(bind(sock, (struct sockaddr*)&server, sizeof(server))==-1){
		perror("ERROR: Bind failed");
		return 1;
	}
	puts("SUCCESS: Bind complete");
	if (!!listen(sock,100)){
		perror("ERROR: Listen failed");
		return 1;
	}
	puts("SUCCESS: Listen complete");
	int pid = -1;
	int accepted, wstatus;
	while (true){
		accepted = accept(sock, (struct sockaddr*)&client, sizeof(client));
		if (accepted<0){
			perror("ERROR: Accept failed");
			return 1;
		}
		puts("SUCCESS: Accept complete");
		pid=fork();
		if (pid==-1){
			perror("ERROR: Fork failed");
			return 1;
		}
		puts("SUCCESS: Fork complete");
		if (pid>0){ //We're the parent process, pid is child's pid
			waitpid(-1, &wstatus, WNOHANG);//Check if any process has finished, but don't hang
		}
		else{ //We're the child process
			insize = recv(accepted, messagein, 5000, 0);
			printf("%s\n", messagein);
		}
	}
}