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
	struct sockaddr_in server, client;//Endpoint addresses for client and server
	if (argc != 2) {//Make sure there are the right amount of inputs
		cout<<"Usage: ./webproxy <port>"<<endl;
		exit(0);
	}
	port=atoi(argv[1]); //does not validate input
	ssock=socket(AF_INET, SOCK_STREAM, 0);//Create an IPv4 TCP socket with no flags
	server.sin_family = AF_INET;//IPv4
	server.sin_addr.s_addr = INADDR_ANY;//Bind to all interfaces
	server.sin_port = htons(port);//Set server port as argv[1]
	if(bind(ssock, (struct sockaddr*)&server, sizeof(server))==-1){//Bind server to sock
		perror("ERROR: Bind failed");
		return 1;
	}
}
