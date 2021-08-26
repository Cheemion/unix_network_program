#include "lib/wrap.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <poll.h>
#include <limits.h>

#define	LISTENQ		1024	/* 2nd argument to listen() */
#define	SERV_PORT		 9877			/* TCP and UDP */
#define	MAXLINE		4096	/* max text line length */
#define OPEN_MAX FOPEN_MAX

int main(int argc, char** argv) {
    int i, maxi, listenfd, connfd, sockfd;
    int nready;
    ssize_t n;
    char buf[MAXLINE];
    struct pollfd client[FOPEN_MAX];
    struct sockaddr_in cliaddr, servaddr;
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);
    Bind(listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
    Listen(listenfd, LISTENQ);
    client[0].fd = listenfd;
    client[0].events = POLLRDNORM;
    for(i = 1; i < FOPEN_MAX; i++) 
        client[i].fd = -1;
    maxi = 0; //index of max used
    socklen_t clilen;
    for( ; ; ) {
        nready = Poll(client, maxi + 1, INFTIM);
        if(client[0].revents & POLLRDNORM) {
            clilen = sizeof(cliaddr);
            connfd = Accept(listenfd, (struct sockaddr*) &cliaddr, &clilen);
            for(i = 1; i < OPEN_MAX; i++)
                if(client[i].fd < 0) {
                    client[i].fd = connfd;
                    break;
                }
            if(i == OPEN_MAX)
                err_sys("too many clients");
            client[i].events = POLLRDNORM;
            maxi = max(maxi, i);
            if(--nready <= 0)
                continue;
        }
        for(i = 1; i <= maxi; i++) {
            if( (sockfd = client[i].fd) < 0)
                continue;
            if(client[i].revents & (POLLRDNORM | POLLERR)) {
                if((n = read(sockfd, buf, MAXLINE)) < 0) {
                    if(errno == ECONNRESET) {
                        // RST by client
                        Close(sockfd);
                        client[i].fd = -1;
                    } else 
                        err_sys("read error");
                } else if(n == 0) {
                    // closed by client;
                    Close(sockfd);
                    client[i].fd = -1;
                } else 
                    Writen(sockfd, buf, n);
                if(--nready <= 0)
                    break;
            }
        }
    }
}