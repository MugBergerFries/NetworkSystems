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
	char types[7][5] = {"html", "txt", "png", "gif", "jpg", "css", "js"};//Possible requested file types
	char corrosponding[7][23] = {"text/html", "text/plain", "image/png", "image/gif", "image/jpg", "text/css", "application/javascript"};//Corrosponding Content-Type
	struct sockaddr_in server, client;//Endpoint addresses for client and server
	int sock, port, socksize, filesize;//Server socket, server port, socket size, and file size
	char *messagein;//Buffer for messages
	sock = socket(AF_INET, SOCK_STREAM, 0);//Create an IPv4 TCP socket with no flags
	server.sin_family = AF_INET;//IPv4
	server.sin_addr.s_addr = INADDR_ANY;//Bind to all interfaces
	port = strtol(argv[1], NULL, 10);//Convert port to int
	server.sin_port = htons(port);//Set server port as argv[1]
	FILE* curfile;//File pointer for open files
	if(bind(sock, (struct sockaddr*)&server, sizeof(server))==-1){//Bind server to sock
		perror("ERROR: Bind failed");
		return 1;
	}
	puts("SUCCESS: Bind complete");
	if (!!listen(sock,100)){//Set sock as a passive socket with a max queue of 100 requests
		perror("ERROR: Listen failed");
		return 1;
	}
	puts("SUCCESS: Listen complete");
	int pid = -1;//Process id for future children
	int accepted, wstatus;
	while (1){
		socksize = sizeof(client);
		accepted = accept(sock, (struct sockaddr*)&client, (socklen_t*)&socksize);//Accept an incoming connetion (SYN, ACK)
		if (accepted<0){
			perror("ERROR: Accept failed");
			return 1;
		}
		puts("SUCCESS: Accept complete");
		pid=fork();//Fork the process
		if (pid==-1){
			perror("ERROR: Fork failed");
			char fileout[5000] = "HTTP/1.1 500 Internal Server Error";
			write(accepted, fileout, strlen(fileout));
			return 1;
		}
		if (pid>0){ //We're the parent process, pid is child's pid
			close(accepted);//Close the child's connection
			if (!!waitpid(-1, &wstatus, WNOHANG)){//Check if any process has finished, but don't hang
				puts("SUCCESS: A child returned");
			}
		}
		else{ //We're the child process
			close(sock);//Close the parent's connection
			messagein = (char*)malloc(100000);//Allocate memory for the incoming message
			recv(accepted, messagein, 100000, 0);//Receive the message
			cout<<"\nRECEIVED MESSAGE BEGIN\n\n"<<messagein<<"RECEIVED MESSAGE END\n\n";
			char * med;//Middleman to parse message
			med = strtok(messagein, " ");//Get message until first " "
			string method, path, vers;
			method = med;//In this case, this will be GET, POST, HEAD
			med = strtok(NULL, " ");//Get after first space and before second
			path = med;//In this case, this will be the path requested
			med = strtok(NULL, " ");
			vers = med;//This will be the HTTP version
			if (method=="GET"){
				if (path=="/" || path=="/inside/"){//If we need to send index.html
					curfile = fopen("./www/index.html", "rb");//Open index in read binary mode (for NULL characters)
					if (curfile!=NULL){//If file is open
						if (fseek(curfile, 0, SEEK_END) != 0) perror("ERROR: Fseek failed");
						filesize = ftello(curfile);//Seek to end of file and report position to get file size
						rewind(curfile);//Reset file location pointer
						char* filedata = (char*)malloc(filesize);//Allocate buffer to store the file
						fread(filedata, 1, filesize, curfile);//Read file into buffer
						char sizebuf[sizeof(int)];
						sprintf(sizebuf, "%d", filesize);//Convert file size in bytes to char array
						char fileout[5000] = "HTTP/1.1 200 Document Follows\r\nContent-Type: text/html\r\nContent-Length: ";
						strcat(fileout, sizebuf);
						strcat(fileout, "\r\n\r\n");
						strcat(fileout, filedata);//Combine header and file data
						write(accepted, fileout, strlen(fileout));//Send the message
						free(filedata);//deallocate 
						shutdown(accepted, SHUT_WR);//Tell client we want to close connection
						while (recv(accepted, messagein, 100000, 0)!=0){}//Wait for client to be ready
						close(accepted);//Close connection
						free(messagein);//Deallocate message buffer
						return 1;
					}
					else{
						perror("ERROR: index file", path);
						char fileout[5000] = "HTTP/1.1 500 Internal Server Error";
						write(accepted, fileout, strlen(fileout));
						close(accepted);
						free(messagein);
						return 1;
					}
				}
				else{
					char* abspath = new char [path.length()+4];//Absolute path to the requested file
					char* pathcstr = new char [path.length()+1];//char array conversion of the path
					strcpy(pathcstr, path.c_str());
					strcpy(abspath, "www");
					strcat(abspath, pathcstr);//Take incoming path, add www to the front
					curfile = fopen(abspath, "rb");//Open the requested file
					if (curfile!=NULL){//If file is open
						if (fseek(curfile, 0, SEEK_END) != 0) perror("ERROR: Fseek failed");
						filesize = ftello(curfile);//Seek to end of file and report position to get file size
						rewind(curfile);//Reset file location pointer
						char* filedata = (char*)malloc(filesize);//Buffer to read file into
						int filebytes;//Bytes read in from file (should equal filesize)
						filebytes = fread(filedata, 1, filesize, curfile);//Read file into buffer
						char sizebuf[sizeof(int)];
						sprintf(sizebuf, "%d", filesize);//Turn filesize into char array
						char* med2;//Middleman to parse file type
						char* pathtmp = new char [path.length()+1];
						strcpy(pathtmp, path.c_str());//Turn path into a char array
						strtok(pathtmp, ".");//Parse path delimited by .
						char *med2tmp;
						while ((med2tmp=strtok(NULL, "."))!=NULL){//Read until the last '.'
							med2 = med2tmp;//Get the file extension
						}
						int ans=-1;
						for (int i=0;i<7;i++){
							if (!strcmp(med2, types[i])) ans=i;//Find the file type from the hard-coded list on line 15
						}
						if (ans==-1){
							cout<<"ERROR: file type not recognized: "<<med2<<endl;
							char fileout[5000] = "HTTP/1.1 500 Internal Server Error";
							write(accepted, fileout, strlen(fileout));
							close(accepted);
							free(messagein);
							return 1;
						}
						char *filehead = (char*)malloc(500);//Malloc buffer for the file header
						strcpy(filehead, "HTTP/1.1 200 Document Follows\r\nContent-Type: ");
						strcat(filehead, corrosponding[ans]);//Set Content-Type to matching field from the list on line 16
						strcat(filehead, "\r\nContent-Length: ");
						strcat(filehead, sizebuf);
						strcat(filehead, "\r\n\r\n");
						int headersize = strlen(filehead); //Size in bytes of the header
						char send[filebytes+headersize]; //Final buffer for the message out
						memcpy(send, filehead, headersize);//Copy the header to the beginning of the buffer (this is to avoid NULL char issues with strcpy)
						memcpy(send+headersize, filedata, filebytes);//Copy the file data to the location right after the header
						write(accepted, send, filebytes+headersize);//Send the data
						free(filedata);
						free(filehead);
						shutdown(accepted, SHUT_WR);//Tell client we want to close the connection
						while (recv(accepted, messagein, 100000, 0)!=0){}//Wait for them to be ready
						close(accepted);//Close the connection
						free(messagein);
						return 1;
					}
					else{
						perror("ERROR: failed to open %s", abspath);
						char fileout[38] = "HTTP/1.1 500 Internal Server Error";
						write(accepted, fileout, 38);
						close(accepted);
						free(messagein);
						return 1;
					}
				}
			}
			else if (method=="POST"){
				puts("ERROR: Post not supported");
				free(messagein);
				close(accepted);
				return 1;
			}
			else if (method=="HEAD"){
				puts("ERROR: Head not supported");
				free(messagein);
				close(accepted);
				return 1;
			}
			else{
				cout<<"ERROR: Unknown request method '"<<method<<"'\n";
				free(messagein);
				close(accepted);
				return 1;
			}
			free(messagein);
			return 0;
		}
	}
}