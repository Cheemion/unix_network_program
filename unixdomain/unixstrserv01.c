#include "../lib/wrap.h"

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
    struct sockaddr_un cliaddr, servaddr;
    listenfd = Socket(AF_LOCAL, SOCK_STREAM, 0);
    unlink(UNIXSTR_PATH);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sun_family = AF_LOCAL;
    strcpy(servaddr.sun_path, UNIXSTR_PATH);
    Bind(listenfd, (SA*) &servaddr, sizeof(servaddr));
    Listen(listenfd, LISTENQ);
    Signal(SIGCHLD, sig_chld);
    for(;;) {
        clilen = sizeof(cliaddr);
        if((connfd = accept(listenfd, (SA*) &cliaddr, &clilen)) < 0) {
            if(errno == EINTR) continue;
            else err_sys("accept error");
        }
        if((childpid == Fork()) == 0) {
            Close(listenfd);
            str_echo(connfd);
            exit(0);
        }
        Close(connfd);
    }
}