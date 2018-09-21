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
    int filesize;
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
    while (1){
	    bzero(buf, BUFSIZE);
	    printf("Please enter msg: ");
	    fgets(buf, BUFSIZE, stdin);
	    serverlen = sizeof(serveraddr);
	    if (!strcmp(buf, "exit\n")){
		    n = sendto(sockfd, buf, strlen(buf), 0, &serveraddr, serverlen);
		    if (n < 0) error("ERROR in sendto");
		    n = recvfrom(sockfd, buf, 24, 0, &serveraddr, &serverlen);
		    if (n < 0) error("ERROR in recvfrom");
	    	else printf("%s\n", buf);
	  		printf("Client shutting down...\n");
	  		return 0;
	    }
	    else if (!strncmp(buf, "get", 3)){
	    	strncpy(fname, buf+4, 4);
	    	printf("INPUTTED FILENAME: %s\n", fname);
		    n = sendto(sockfd, buf, strlen(buf), 0, &serveraddr, serverlen);
		    if (n < 0) error("ERROR in sendto");
		    printf("Client requesting file...\n");
		    char tmpbfr[4096];
		    n = recvfrom(sockfd, tmpbfr, 4096, 0, &serveraddr, &serverlen); //File size
		    if (n < 0){
		    	error("ERROR in recvfrom");
	  			printf("Client shutting down...\n");
	  			return 0;
		    }
		    filesize = atoi(tmpbfr);
	    	//filesize=atoi(buf);
	    	if (filesize==-1){
	    		printf("Server could not locate file\n");
	    	}
	    	else{
		    	curfile = fopen(fname, "w");
		    	int received;
		    	received = recvfrom(sockfd, buf, filesize, 0, &serveraddr, &serverlen);
		    	fwrite(buf, 1, received, curfile);
		    	printf("FILE RECEIVED\n");
		    	fclose(curfile);
	    	}
	    }
        else if (!strncmp(buf, "put", 3)){
	        strncpy(fname, buf+4, 4);
	        printf("INPUTTED FILENAME: %s\n", fname);
 			curfile = fopen(fname, "r+");
 			if (curfile!=NULL){
	        	fseek(curfile, 0L, SEEK_END);
		        filesize = ftell(curfile);
		        rewind(curfile);
		        printf("FILE SIZE: %d\n", filesize);
		        char sizebuf[4096];
		        sprintf(sizebuf, "%d", filesize);
		        n = sendto(sockfd, sizebuf, 4096, 0, &serveraddr, serverlen);
		        char tempbuf[filesize];
		        fread(tempbuf, 1, filesize, curfile);
		        n = sendto(sockfd, tempbuf, strlen(tempbuf), 0, &serveraddr, serverlen);
	    	}
	    	else{
	    		printf("FAILED TO OPEN FILE\n");
	  			printf("Client shutting down...\n");
	  			return 0;
	    	}
	    }
	    /* send the message to the server 
	    n = sendto(sockfd, buf, strlen(buf), 0, &serveraddr, serverlen);
	    if (n < 0) 
	      error("ERROR in sendto");
	    
	     print the server's reply 
	    n = recvfrom(sockfd, buf, strlen(buf), 0, &serveraddr, &serverlen);
	    if (n < 0) 
	      error("ERROR in recvfrom");
	    printf("%s\n", buf);*/
	}
    return 0;
}
/*
server possible replies:
file
-1: no file matching
-2: file received
-3: file not received
-4: file deleted
-5: file not found to delete
list
*/