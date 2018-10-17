#include<iostream>
#include<stdio.h>
#include<string.h>
#include<string>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<stdlib.h>

using namespace std;

int main(int argc, char *argv[]){
	struct sockaddr_in server, client;
	int sock, insize, port, socksize;
	char messagein[1000];
	sock = socket(AF_INET, SOCK_STREAM, 0);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	port = strtol(argv[1], NULL, 10);
	server.sin_port = htons(port);
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
	while (1){
		socksize = sizeof(client);
		accepted = accept(sock, (struct sockaddr*)&client, (socklen_t*)&socksize);
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
			if (!!waitpid(-1, &wstatus, WNOHANG)){//Check if any process has finished, but don't hang
				puts("SUCCESS: A child returned");
			}
		}
		else{ //We're the child process
			insize = recv(accepted, messagein, 5000, 0);
			cout<<"MESSAGEIN: "<<messagein<<endl;
			char * med;
			med = strtok(messagein, " ");
			string method, path, vers;
			method = med;
			med = strtok(NULL, " ");
			path = med;
			med = strtok(NULL, " ");
			vers = med;
			cout<<"METHOD, PATH: "<<method<<", "<<path<<endl;
			if (method=="GET"){
				printf("%s\n", "WE DID IT REDDIT\n");
				close(sock);
			}
			else if (method=="POST"){
				puts("ERROR: Post not supported");
				return 1;
			}
			else if (method=="HEAD"){
				puts("ERROR: Head not supported");
				return 1;
			}
			else{
				cout<<"ERROR: Unknown request method '"<<method<<"'\n";
				return 1;
			}
			return 0;
		}
	}
}