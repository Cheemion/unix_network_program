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

void str_echo(int sockfd) {
    ssize_t n;
    char buf[MAXLINE];
    again:
        while((n = read(sockfd, buf, MAXLINE)) > 0)
            Writen(sockfd, buf, n);
        if(n < 0 && errno == EINTR) 
            goto again;
        else if (n < 0) 
            err_sys("str_echo:read error");
};

void sig_chld(int signo) {
    pid_t pid;
    int stat;
    while((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
        printf("child %d terminated \n", pid);
    }
    return;
}

int main(int argc, char** argv) {
    int listenfd, connfd;
    pid_t childpid;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);
    Bind(listenfd, (struct sockaddr* ) &servaddr, sizeof(servaddr));
    Listen(listenfd, LISTENQ);
    Signal(SIGCHLD, sig_chld);
    for(;;) {
       clilen = sizeof(cliaddr);
       if((connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen)) < 0) {
            if(errno == EINTR)
                continue;
            else
                err_sys("accept error");
       }
       if((childpid = Fork()) == 0) { // equal to 0 means child process
           Close(listenfd);
           str_echo(connfd);
           exit(0);
       }
       close(connfd);
    }
}