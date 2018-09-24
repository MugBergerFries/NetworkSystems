/* 
 * udpclient.c - A simple UDP client
 * usage: udpclient <host> <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <stdbool.h>
#include <errno.h>

#define BUFSIZE 1024

/* 
 * error - wrapper for perror
 */
void error(char *msg) {
	perror(msg);
	exit(0);
}

int main(int argc, char **argv) {
	int sockfd, portno, n;
	int serverlen;
	struct sockaddr_in serveraddr;
	struct hostent *server;
	char *hostname;
	char buf[BUFSIZE];
	long filesize;
	char fname[4];
	FILE* curfile;

	/* check command line arguments */
	if (argc != 3) {
	   fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
	   exit(0);
	}
	hostname = argv[1];
	portno = atoi(argv[2]);

	/* socket: create the socket */
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) 
		error("ERROR opening socket");

	/* gethostbyname: get the server's DNS entry */
	server = gethostbyname(hostname);
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host as %s\n", hostname);
		exit(0);
	}

	/* build the server's Internet address */
	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, 
	  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
	serveraddr.sin_port = htons(portno);

	/* get a message from the user */
	while (1){//Full loop
		bzero(buf, BUFSIZE);
		printf("Please enter msg: ");
		fgets(buf, BUFSIZE, stdin);
		serverlen = sizeof(serveraddr);
		if (!strcmp(buf, "exit\n")){//If 'exit'
			n = sendto(sockfd, buf, strlen(buf), 0, &serveraddr, serverlen);//Send 'exit'
			if (n < 0) error("ERROR in sendto");
			n = recvfrom(sockfd, buf, 24, 0, &serveraddr, &serverlen);//Receive reply
			if (n < 0) error("ERROR in recvfrom");
			else printf("%s\n", buf);//Print response
			printf("Client shutting down...\n");
			return 0;//Shut down
		}
		else if (!strncmp(buf, "get", 3)){//If 'get' then anything
			strncpy(fname, buf+4, 4);//Expects 4 character name
			n = sendto(sockfd, buf, strlen(buf), 0, &serveraddr, serverlen);
			if (n < 0) error("ERROR in sendto");
			printf("Client requesting file...\n");
			char *tmpbfr = malloc(4096);
			n = recvfrom(sockfd, tmpbfr, 4096, 0, &serveraddr, &serverlen); //File size
			printf("Received file size: %s\n", tmpbfr);
			if (n < 0){
				error("ERROR in recvfrom");
				printf("Client shutting down...\n");
				return 0;
			}
			filesize = atoi(tmpbfr);
			free(tmpbfr);
			if (filesize==-1){
				printf("Server could not locate file\n");
			}
			else{
				curfile = fopen(fname, "wb");//Open file in binary mode
				int received;
				char *tempbuf = malloc(2*filesize);
				received = recvfrom(sockfd, tempbuf, filesize, 0, &serveraddr, &serverlen);//Get file
				printf("Received: %d\n", received);
				fwrite(tempbuf, 1, received, curfile);//Write form buffer to file
				printf("FILE RECEIVED\n");
				fclose(curfile);
				free(tempbuf);
			}
		}
		else if (!strncmp(buf, "put", 3)){//If 'put' then anything
			strncpy(fname, buf+4, 4);//Expects 4 character filename
			printf("INPUTTED FILENAME: %s\n", fname);
			n = sendto(sockfd, buf, strlen(buf), 0, &serveraddr, serverlen);//Send input to server
			curfile = fopen(fname, "rb");//Open file as binary
			if (curfile!=NULL){//If file is open
				if (fseek(curfile, 0, SEEK_END) != 0) printf("ERROR IN FSEEK\n");
				filesize = ftello(curfile);//Seek to end of file and report position to get file size
				rewind(curfile);
				printf("FILE SIZE: %d\n", filesize);
				char sizebuf[4096];
				sprintf(sizebuf, "%d", filesize);
				n = sendto(sockfd, sizebuf, 4096, 0, &serveraddr, serverlen);//Send file size
				char* tempbuf = malloc(2*filesize);
				int temp;
				temp = fread(tempbuf, 1, filesize, curfile);//Read file into buffer
				printf("%d\n", temp);
				n = sendto(sockfd, tempbuf, temp, 0, &serveraddr, serverlen);//Send file
				free(tempbuf);
			}
			else{
				printf("FAILED TO OPEN FILE\n");
				printf("Client shutting down...\n");
				return 0;
			}
		}
		else if (!strncmp(buf, "ls", 2) || !strncmp(buf, "delete", 6)){//If 'ls' or 'delete'
			n = sendto(sockfd, buf, strlen(buf), 0, &serveraddr, serverlen);//Send input
			recvfrom(sockfd, buf, BUFSIZE, 0, &serveraddr, &serverlen);//Get response
			printf("%s\n", buf);
		}
		else{
			n = sendto(sockfd, buf, strlen(buf), 0, &serveraddr, serverlen);//If anything else
			recvfrom(sockfd, buf, BUFSIZE, 0, &serveraddr, &serverlen);//Get response
			printf("%s\n", buf);
		}
	}
	return 0;
}