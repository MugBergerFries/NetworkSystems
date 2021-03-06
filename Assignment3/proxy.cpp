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
#include<tr1/functional>
#include<algorithm>
#include<vector>
#include<ctime>
#include<netdb.h>
#include<fstream>

using namespace std;

vector<string> urlcache;

int main(int argc, char* argv[]){
	vector<string>* cacheaddr = &urlcache;
	hash<string> f;
	int port, socksize, filesize; //webproxy port
	int psock, csock; //proxy and client socket
	FILE* curfile;
	FILE* blocked;
	char* messagein;
	char* messagecpy;
	vector<string> blockhost;
	vector<string> blockip;
	struct sockaddr_in proxy, client;//Endpoint addresses for client and proxy
	if (argc != 3) {//Make sure there are the right amount of inputs
		cout<<"Usage: ./webproxy <port> <timeout>"<<endl;
		exit(0);
	}
	port=atoi(argv[1]); //does not validate input
	psock=socket(AF_INET, SOCK_STREAM, 0);//Create an IPv4 TCP socket with no flags
	proxy.sin_family = AF_INET;//IPv4
	proxy.sin_addr.s_addr = INADDR_ANY;//Bind to all interfaces
	proxy.sin_port = htons(port);//Set proxy port as argv[1]
	if(bind(psock, (struct sockaddr*)&proxy, sizeof(proxy))<0){//Bind proxy to sock
		perror("ERROR: Bind failed");
		close(psock);
		exit(0);
	}
	if (!!listen(psock,100)){//Set sock as a passive socket with a max queue of 100 requests
		perror("ERROR: Listen failed");
		close(psock);
		exit(0);
	}
	int pid = -1;//Process id for future children
	int wstatus;
	ifstream inFile;
	string tmp;
	inFile.open("blacklist.txt");
	while (getline(inFile, tmp)){
			blockhost.push_back(tmp);
			blockip.push_back(tmp);
	}
	cout<<blockhost[0]<<" "<<blockhost[1]<<endl;
	while (1){
		socksize = sizeof(client);
		csock = accept(psock, (struct sockaddr*)&client, (socklen_t*)&socksize);//Accept an incoming connetion (SYN, ACK)
		if (csock<0){
			perror("ERROR: Accept failed");
			return 1;
		}
		puts("SUCCESS: Accept complete");
		pid=fork();//Fork the process
		if (pid==-1){
			perror("ERROR: Fork failed");
			char fileout[5000] = "HTTP/1.1 500 Internal Server Error";
			write(csock, fileout, strlen(fileout));
			return 1;
		}
		if (pid>0){ //We're the parent process, pid is child's pid
			close(csock);//Close the child's connection
			if (!!waitpid(-1, &wstatus, WNOHANG)){//Check if any process has finished, but don't hang
				puts("SUCCESS: A child returned");
			}
		}
		else{ //We're the child process
			close(psock);//Close the parent's connection
			messagein = (char*)malloc(100000);//Allocate memory for the incoming message
			messagecpy = (char*)malloc(100000);//Allocate memory for the incoming message
			recv(csock, messagein, 100000, 0);//Receive the message
			cout<<"\nRECEIVED MESSAGE BEGIN\n"<<messagein<<"RECEIVED MESSAGE END\n\n";
			memcpy(messagecpy, messagein, 100000);
			char* med;//Middleman to parse message
			med = strtok(messagein, " ");//Get message until first " "
			string method, path, vers, host;
			method = med;//In this case, this will be GET, POST, HEAD
			med = strtok(NULL, " ");//Get after first space and before second
			path = med;//In this case, this will be the path requested
			med = strtok(NULL, " ");
			vers = med;//This will be the HTTP version
			med = strtok(NULL, "\n");
			host = med;
			vector<string>::iterator urlindex;
			string urlhash = to_string(f(path));
			if (method=="GET"){
				if(find(cacheaddr->begin(), cacheaddr->end(), urlhash) != cacheaddr->end()) {
					urlindex = find(cacheaddr->begin(), cacheaddr->end(), urlhash);
					rotate(cacheaddr->begin(), urlindex, urlindex+1);
					char filename[30];
					strcpy(filename, "cache/");
					strcat(filename, urlhash.c_str());
					printf("Found %s in the cache\n", filename);
					curfile = fopen(filename, "rb");
					if (fseek(curfile, 0, SEEK_END) != 0) perror("ERROR: Fseek failed");
					filesize = ftello(curfile);//Seek to end of file and report position to get file size
					rewind(curfile);//Reset file location pointer
					char* filedata = (char*)malloc(filesize);//Buffer to read file into
					int filebytes;//Bytes read in from file (should equal filesize)
					filebytes = fread(filedata, 1, filesize, curfile);//Read file into buffer
					char sizebuf[sizeof(int)];
					sprintf(sizebuf, "%d", filesize);//Turn filesize into char array
					/*char* med2;//Middleman to parse file type
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
						write(csock, fileout, strlen(fileout));
						close(csock);
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
					memcpy(send+headersize, filedata, filebytes);//Copy the file data to the location right after the header*/
					write(csock, filedata, filebytes);//Send the data
					free(filedata);
					//free(filehead);
					shutdown(csock, SHUT_WR);//Tell client we want to close the connection
					while (recv(csock, messagein, 100000, 0)!=0){}//Wait for them to be ready
					close(csock);//Close the connection
					free(messagein);
					return 1;
				}else{
					if (cacheaddr->size() == 10){
						cout<<"IT'S ALL FILLED UP CAP'N"<<endl;
						char oldname[30];
						strcpy(oldname, "cache/");
						strcat(oldname, cacheaddr->back().c_str());
						cacheaddr->pop_back();
						remove(oldname);
						printf("Deleted %s\n", oldname);
					}
					cacheaddr->insert(cacheaddr->begin(), urlhash);
					cout<<"ADDED "<<urlhash<<" TO THE CACHE; SIZE: "<<cacheaddr->size()<<endl;
					char filename[30];
					strcpy(filename, "cache/");
					strcat(filename, urlhash.c_str());
					printf("Created %s\n", filename);
					curfile = fopen(filename, "wb");
					int ssock, msgsize;
					struct sockaddr_in server;
					struct hostent *sent;
					memset(&server, 0, sizeof(server));
					server.sin_family = AF_INET;
					/* the following code block is from echoClient.c from Assignment1
				    /* Map port number (char string) to port number (int)*/
        			if ((server.sin_port=htons((unsigned short)80)) == 0){
                		printf("can't get \"%d\" port number\n", 80);
                		exit(1);
        			}

    				/* Map host name to IP address, allowing for dotted decimal */
        			if ( sent = gethostbyname(path.substr(7, path.substr(7).find("/", 0)).c_str()) ){
                		memcpy(&server.sin_addr, sent->h_addr, sent->h_length);
        			}
        			else if ( (server.sin_addr.s_addr = inet_addr(path.c_str())) == INADDR_NONE ){
                		printf("can't get %s host entry: %s\n", path.substr(7, path.substr(7).find("/", 0)).c_str(), strerror(errno));
                		exit(1);
        			}
        			char tester[INET_ADDRSTRLEN];
        			string ip;
        			inet_ntop(AF_INET, &(server.sin_addr.s_addr), tester, INET_ADDRSTRLEN);
        			ip.assign(tester, find(tester, tester+INET_ADDRSTRLEN, '\0'));
        			cout<<"TESTING: "<<ip<<endl;
        			if ((find(blockhost.begin(), blockhost.end(), path.substr(7, path.substr(7).find("/", 0))) != blockhost.end()) || (find(blockhost.begin(), blockhost.end(), ip) != blockhost.end())){
						perror("ERROR: page blacklisted");
						cout<<"Path in question (not absolute): "<<path<<endl<<endl;
						char fileout[5000] = "HTTP/1.1 403 Forbidden\r\n\r\n";
						write(csock, fileout, strlen(fileout));
						close(csock);
						free(messagein);
						return 1;
        			}
    				/* Allocate a socket */
        			ssock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        			if (ssock < 0){
                		printf("can't create socket: %s\n", strerror(errno));
                		exit(1);
        			}

    				/* Connect the socket */
        			if (connect(ssock, (struct sockaddr *)&server, sizeof(server)) < 0){
            			printf("can't connect to %s.80: %s\n", path.c_str(), strerror(errno));
            			exit(1);
        			}
        			int test;
            		string final = ("GET " + path.substr(path.substr(11).find("/", 0)+11) + " " + vers + " " + host + "\nConnection: keep-alive\r\n\r\n");
            		cout<<"SENDING: "<<final<<endl;
            		test = write(ssock, final.c_str(), strlen(final.c_str()));
            		while((msgsize = recv(ssock, messagecpy, 100000, 0)) > 0){
            			cout<<"Reply received: size = "<<msgsize<<endl;
						fwrite(messagecpy, 1, strlen(messagecpy), curfile);//Write form buffer to file
            			cout<<"Wrote to file"<<msgsize<<endl;
            			write(csock, messagecpy, strlen(messagecpy));
            			cout<<"Sent to client"<<msgsize<<endl;
        			}
					shutdown(ssock, SHUT_WR);//Tell client we want to close the connection
					while (recv(ssock, messagein, 100000, 0)!=0){}//Wait for them to be ready
					close(ssock);//Close the connection
					shutdown(csock, SHUT_WR);//Tell client we want to close the connection
					while (recv(csock, messagein, 100000, 0)!=0){}//Wait for them to be ready
					close(csock);//Close the connection
					free(messagein);
					free(messagecpy);
					return 1;
				}
			}
			else{
				//Send HTTP 400 Bad Request
			}
			free(messagein);
			return 0;
		}
	}
}
