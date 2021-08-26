#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include "lib/wrap.h"
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#define	LISTENQ		1024	/* 2nd argument to listen() */
#define	SERV_PORT	9877			/* TCP and UDP */
#define	MAXLINE		4096	/* max text line length */

void str_cli_select_01(FILE* fp, int sockfd) {
    int maxfdp1;
    fd_set rset;
    char sendline[MAXLINE], recvline[MAXLINE];
    FD_ZERO(&rset);
    for(;;) {
        FD_SET(fileno(fp), &rset);
        FD_SET(sockfd, &rset);
        maxfdp1 = max(fileno(fp), sockfd) + 1;
        Select(maxfdp1, &rset, NULL, NULL, NULL);
        if (FD_ISSET(sockfd, &rset)) {
            // 这里会有问题， 有可能FD_ISSET 没有读到，但是其实已经有好几行在缓存了
            if(Readline(sockfd, recvline, MAXLINE) == 0) 
                err_sys("str_cli: server terminated prematurely");
            Fputs(recvline, stdout);
        }
        if(FD_ISSET(fileno(fp), &rset)) {
            if(Fgets(sendline, MAXLINE, fp) == NULL) return; //这里其实会有问题， 因为有可能没有东西发送，但是其实还在接受.
            Writen(sockfd, sendline, strlen(sendline));
        }
    }
}

void str_cli_select_02(FILE* fp, int sockfd) {
    fd_set rset;
    FD_ZERO(&rset);
    char buffer[MAXLINE];
    int stdineof = 0;
    int n;
    for(;;) {
        if (stdineof == 0) FD_SET(fileno(fp), &rset);
        FD_SET(sockfd, &rset);
        int maxfdp1 = max(fileno(fp), sockfd) + 1;
        Select(maxfdp1, &rset, NULL, NULL, NULL);
        if(FD_ISSET(sockfd, &rset)) { // socket 的检查
            if( (n = Read(sockfd, buffer, MAXLINE)) == 0) {
                if(stdineof == 1)  return; // normal termination ， 输入流结束了并且sock也没有数据了
                else err_sys("strcli: server terminated prematurely");
            }
            Write(fileno(stdout), buffer, n);
        }

        if(FD_ISSET(fileno(fp), &rset)) { //文件io的检查
            if((n = Read(fileno(fp), buffer, MAXLINE)) == 0) {
                stdineof = 1;
                Shutdown(sockfd, SHUT_WR);
                FD_CLR(fileno(fp), &rset);
                continue;
            }
            Writen(sockfd, buffer, n);
        }
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
    str_cli_select_02(stdin, sockfd);
    exit(0);
}