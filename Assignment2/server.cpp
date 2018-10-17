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
	char types[7][5] = {"html", "txt", "png", "gif", "jpg", "css", "js"};
	char corrosponding[7][23] = {"text/html", "text/plain", "image/png", "image/gif", "image/jpg", "text/css", "application/javascript"};
	struct sockaddr_in server, client;
	int sock, insize, port, socksize, filesize;
	char *messagein;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	port = strtol(argv[1], NULL, 10);
	server.sin_port = htons(port);
	FILE* curfile;
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
			char fileout[5000] = "HTTP/1.1 500 Internal Server Error";
			write(accepted, fileout, strlen(fileout));
			return 1;
		}
		puts("SUCCESS: Fork complete");
		if (pid>0){ //We're the parent process, pid is child's pid
			if (!!waitpid(-1, &wstatus, WNOHANG)){//Check if any process has finished, but don't hang
				puts("SUCCESS: A child returned");
			}
		}
		else{ //We're the child process
			messagein = (char*)malloc(5000);
			insize = recv(accepted, messagein, 5000, 0);
			cout<<messagein<<endl;
			char * med;
			med = strtok(messagein, " ");
			string method, path, vers;
			method = med;
			med = strtok(NULL, " ");
			path = med;
			med = strtok(NULL, " ");
			vers = med;
			if (method=="GET"){
				if (path=="/" || path=="/inside/"){
					curfile = fopen("./www/index.html", "rb");
					if (curfile!=NULL){//If file is open
						if (fseek(curfile, 0, SEEK_END) != 0) perror("ERROR: Fseek failed");
						filesize = ftello(curfile);//Seek to end of file and report position to get file size
						rewind(curfile);
						char* tempbuf = (char*)malloc(filesize);
						int temp;
						temp = fread(tempbuf, 1, filesize, curfile);//Read file into buffer
						char sizebuf[sizeof(int)];
						sprintf(sizebuf, "%d", filesize);
						char fileout[5000] = "HTTP/1.1 200 Document Follows\r\nContent-Type: text/html\r\nContent-Length: ";
						strcat(fileout, sizebuf);
						strcat(fileout, "\r\n\r\n");
						strcat(fileout, tempbuf);
						strcat(fileout, "\0");
						write(accepted, fileout, strlen(fileout));
						free(tempbuf);
					}
					else{
						perror("ERROR: fopen failed");
						char fileout[5000] = "HTTP/1.1 500 Internal Server Error";
						write(accepted, fileout, strlen(fileout));
						return 1;
					}
				}
				else{
					char* abspath = new char [path.length()+4];
					char* pathcstr = new char [path.length()+1];
					strcpy(pathcstr, path.c_str());
					strcpy(abspath, "www");
					strcat(abspath, pathcstr);
					cout<<"DEBUG: "<<abspath<<endl;
					curfile = fopen(abspath, "rb");
					if (curfile!=NULL){//If file is open
						if (fseek(curfile, 0, SEEK_END) != 0) perror("ERROR: Fseek failed");
						filesize = ftello(curfile);//Seek to end of file and report position to get file size
						rewind(curfile);
						char* tempbuf = (char*)malloc(filesize);
						int temp;
						temp = fread(tempbuf, 1, filesize, curfile);//Read file into buffer
						char sizebuf[sizeof(int)];
						sprintf(sizebuf, "%d", filesize);
						char* med2;
						char* pathtmp = new char [path.length()+1];
						strcpy(pathtmp, path.c_str());
						strtok(pathtmp, ".");
						char *med2tmp;
						while ((med2tmp=strtok(NULL, "."))!=NULL){
							med2 = med2tmp;
						}
						int ans=-1;
						for (int i=0;i<7;i++){
							if (!strcmp(med2, types[i])) ans=i;
						}
						if (ans==-1){
							cout<<"ERROR: file type not recognized: "<<med2<<endl;
							return 1;
						}
						char fileout[100000] = "HTTP/1.1 200 Document Follows\r\nContent-Type: ";
						strcat(fileout, corrosponding[ans]);
						strcat(fileout, "\r\nContent-Length: ");
						strcat(fileout, sizebuf);
						strcat(fileout, "\r\n\r\n");
						cout<<"test: "<<strlen(fileout)<<" - "<<sizeof(tempbuf)<<endl;
						strcat(fileout, tempbuf);
						cout<<"test2: "<<strlen(fileout)<<endl;
						strcat(fileout, "\0");
						cout<<"DEBUG2: "<<temp<<" - "<<sizeof(fileout)<<endl;
						write(accepted, fileout, sizeof(fileout)-1);
						free(tempbuf);
					}
					else{
						perror("ERROR: fopen failed");
					}
				}
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