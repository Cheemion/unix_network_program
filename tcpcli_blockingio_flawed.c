#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include "lib/wrap.h"
#include <stdio.h>
#include <string.h>
#define	LISTENQ		1024	/* 2nd argument to listen() */
#define	SERV_PORT	9877			/* TCP and UDP */
#define	MAXLINE		4096	/* max text line length */

void str_cli(FILE* fp, int sockfd) {
    // here we have two flaws
    // one is when Fgets is ended with EOF,  we are not be able to read from sockets
    // another is when the server is terminated , we still block in Fgets
    char sendline[MAXLINE], recvline[MAXLINE];
    while(Fgets(sendline, MAXLINE, fp) != NULL) { 
        Writen(sockfd, sendline, strlen(sendline)); 
        if(Readline(sockfd, recvline, MAXLINE) == 0) 
            err_sys("str_cli:server terminated permaturely");
        Fputs(recvline, stdout);
    }
}

int main(int argc, char** argv) {
    int sockfd;
    struct sockaddr_in servaddr;
    if(argc != 2) 
        err_sys("usage: tcpcli <IPaddress>");
    sockfd = Socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
    Connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
    str_cli(stdin, sockfd);
    exit(0);
}