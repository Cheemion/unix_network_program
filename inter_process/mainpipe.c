#include "../lib/wrap.h"
void client(int readfd, int writefd) {
    size_t len;
    ssize_t n;
    char buff[MAXLINE];
    Fgets(buff, MAXLINE, stdin);
    len = strlen(buff);
    if(buff[len - 1] == '\n')
        len--;
    Write(writefd, buff, len);
    while((n = Read(readfd, buff, MAXLINE)) >0)
        Write(STDOUT_FILENO, buff, n);
}
void server(int readfd, int writefd) {
    int fd;
    ssize_t n;
    char buff[MAXLINE + 1];
    if((n = Read(readfd, buff, MAXLINE)) == 0)
        err_sys("end of file while reading pathname");
    buff[n] = '\0';
    if((fd = open(buff, O_RDONLY)) < 0){
        snprintf(buff + n, sizeof(buff) - n, ": can't open, %s \n", strerror(errno));
        n = strlen(buff);
        Write(writefd, buff, n);
    } else {
        while((n = Read(fd, buff, MAXLINE)) > 0)
            Write(writefd, buff, n);
        Close(fd);
    }
}

int main(int argc, char** argv) {
    int pipe1[2], pipe2[2];
    pid_t childpid;
    Pipe(pipe1);
    Pipe(pipe2);
    if((childpid = Fork()) == 0) { //child
        Close(pipe1[1]);
        Close(pipe2[0]);
        server(pipe1[0], pipe2[1]);
        exit(0);
    }
    Close(pipe1[0]);
    Close(pipe2[1]);
    client(pipe2[0], pipe1[1]);
    Waitpid(childpid, NULL, 0);
    exit(0);
}