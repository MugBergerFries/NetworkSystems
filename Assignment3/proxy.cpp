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

int main(int argc, char* argv[]){
	int port; //webproxy port
	int ssock, csock; //server and client socket
	struct sockaddr_in server, client;
	if (argc != 2) {
		cout<<"Usage: ./webproxy <port>"<<endl;
		exit(0);
	}
	port=(int*)argv[1];
	cout<<port<<endl;
	return 1;
}
