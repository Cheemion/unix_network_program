#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "lib/wrap.h"
#include <errno.h>
#define	LISTENQ		1024	/* 2nd argument to listen() */
#define	SERV_PORT		 9877			/* TCP and UDP */
#define	MAXLINE		4096	/* max text line length */

/*
On Linux, select() may report a socket file descriptor as "ready
       for reading", while nevertheless a subsequent read blocks.  This
       could for example happen when data has arrived but upon
       examination has the wrong checksum and is discarded.  There may
       be other circumstances in which a file descriptor is spuriously
       reported as ready.  Thus it may be safer to use O_NONBLOCK on
       sockets that should not block.
*/

int main(int argc, char** argv)  {
    int i, maxi, maxfd,listenfd, connfd, sockfd;
    int nready, client[FD_SETSIZE];
    ssize_t n;
    fd_set rset, allset;
    socklen_t clilen;
    char buf[MAXLINE];
    struct sockaddr_in cliaddr, servaddr;
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);
    Bind(listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
    Listen(listenfd, LISTENQ);
    maxfd = listenfd;
    maxi = -1;
    for(i = 0; i < FD_SETSIZE; i++) {
        client[i] = -1;
    }
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    for( ; ; ) {
        rset = allset;
        nready = Select(maxfd + 1, &rset, NULL, NULL, NULL);
        if(FD_ISSET(listenfd, &rset)) {
            clilen = sizeof(cliaddr);
            connfd = Accept(listenfd, (struct sockaddr*) &cliaddr, &clilen);
            for(i = 0; i < FD_SETSIZE; i++) {
                if(client[i] < 0) {
                    client[i] = connfd;
                    break;
                }
            }
            if(i == FD_SETSIZE)
                err_sys("too many clients");
            FD_SET(connfd, &allset);
            maxfd = max(connfd, maxfd);
            maxi = max(maxi, i); // max index in client[] array
            if(--nready <= 0)  // end of ready fd
                continue;
        }
        for(i = 0; i <= maxi; i++) {
            if((sockfd = client[i]) < 0) continue;
            if(FD_ISSET(sockfd, &rset)) {
                if( (n = Read(sockfd, buf, MAXLINE)) == 0) {
                    //connect closed by client
                    Close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
                } else {
                    Writen(sockfd, buf, n);
                }
                if(--nready <= 0) break;
            }
        }
    }
}