/* 
 * udpserver.c - A simple UDP echo server 
 * usage: udpserver <port>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <errno.h>

#define BUFSIZE 1024

/*
 * error - wrapper for perror
 */
void error(char *msg) {
  perror(msg);
  exit(1);
}

int main(int argc, char **argv) {
  int sockfd; /* socket */
  int portno; /* port to listen on */
  int clientlen; /* byte size of client's address */
  struct sockaddr_in serveraddr; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
  struct hostent *hostp; /* client host info */
  char buf[BUFSIZE]; /* message buf */
  char *hostaddrp; /* dotted decimal host addr string */
  int optval; /* flag value for setsockopt */
  int n; /* message byte size */
  long filesize;
  char fname[5];
  FILE* curfile;
  char storedfiles[3][5];
  int filecount=0;
  int fileindex;


  /* 
   * check command line arguments 
   */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  portno = atoi(argv[1]);

  /* 
   * socket: create the parent socket 
   */
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) 
    error("ERROR opening socket");

  /* setsockopt: Handy debugging trick that lets 
   * us rerun the server immediately after we kill it; 
   * otherwise we have to wait about 20 secs. 
   * Eliminates "ERROR on binding: Address already in use" error. 
   */
  optval = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 
	     (const void *)&optval , sizeof(int));

  /*
   * build the server's Internet address
   */
  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons((unsigned short)portno);

  /* 
   * bind: associate the parent socket with a port 
   */
  if (bind(sockfd, (struct sockaddr *) &serveraddr, 
	   sizeof(serveraddr)) < 0) 
    error("ERROR on binding");

  /* 
   * main loop: wait for a datagram, then echo it
   */
  clientlen = sizeof(clientaddr);
  while (1) {

    /*
     * recvfrom: receive a UDP datagram from a client
     */
    bzero(buf, BUFSIZE);
    n = recvfrom(sockfd, buf, BUFSIZE, 0,
		 (struct sockaddr *) &clientaddr, &clientlen);
    if (n < 0)
      error("ERROR in recvfrom");

    /* 
     * gethostbyaddr: determine who sent the datagram
     */
    hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, 
			  sizeof(clientaddr.sin_addr.s_addr), AF_INET);
    if (hostp == NULL)
      error("ERROR on gethostbyaddr");
    hostaddrp = inet_ntoa(clientaddr.sin_addr);
    if (hostaddrp == NULL)
      error("ERROR on inet_ntoa\n");
    printf("server received datagram from %s (%s)\n", 
	   hostp->h_name, hostaddrp);
    printf("server received %d/%d bytes: %s\n", strlen(buf), n, buf);
    if (!strcmp(buf, "exit\n")){
      n = sendto(sockfd, "Server shutting down...", strlen("Server shutting down..."), 0, 
         (struct sockaddr *) &clientaddr, clientlen);
      if (n < 0) error("ERROR in sendto");
      return 0;
    }
    else if (!strncmp(buf, "get", 3)){
      strncpy(fname, buf+4, 4);
      printf("INPUTTED FILENAME: %s\n", fname);
      curfile = fopen(fname, "rb");
      char sizebuf[4096];
      if (curfile != NULL){
        fseeko(curfile, 0, SEEK_END);
        filesize = ftello(curfile);
        rewind(curfile);
        sprintf(sizebuf, "%d", filesize);
        n = sendto(sockfd, sizebuf, 4096, 0, &clientaddr, clientlen);
        char *tempbuf = malloc(2*filesize);
        int temp;
        temp = fread(tempbuf, 1, filesize, curfile);
        n = sendto(sockfd, tempbuf, temp, 0, &clientaddr, clientlen);
      }
      else{
        sprintf(sizebuf, "%d", -1);
        n = sendto(sockfd, sizebuf, 4096, 0, &clientaddr, clientlen);
      }
    }
    else if (!strncmp(buf, "put", 3)){
      strncpy(fname, buf+4, 4);
      printf("INPUTTED FILENAME: %s\n", fname);
      char tmpbfr[4096];
      n = recvfrom(sockfd, tmpbfr, 4096, 0, &clientaddr, &clientlen); //File size
      printf("Client sending file...\n");
      if (n < 0){
        error("ERROR in recvfrom");
        printf("Server shutting down...\n");
        return 0;
      }
      filesize = atoi(tmpbfr);
      //filesize=atoi(buf);
      curfile = fopen(fname, "wb");
      if (curfile == NULL) printf("ERROR: FILE NULL\n");
      int received;
      received = recvfrom(sockfd, buf, filesize, 0, &clientaddr, &clientlen);
      if (errno==EFAULT){
        printf("SLIM SHADY\n");
      }
      else if (errno==EAGAIN){
        printf("ACHTUNG\n");
      }
      else printf("FACK\n");
      printf("RECEIVED: %d\n", received);
      fwrite(buf, 1, received, curfile);
      printf("FILE RECEIVED\n");
      fclose(curfile);
    } 
    /*if (buf=="exit"){
      off=1;   
      n = sendto(sockfd, "Server shutting down...", strlen("Server shutting down..."), 0, 
         (struct sockaddr *) &clientaddr, clientlen);
      if (n < 0) error("ERROR in sendto");
    }*/

  }
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