#include "../lib/wrap.h"
#define FIFO1 "/tmp/fifo.1"
#define FIFO2 "/tmp/fifo.2"

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
    int readfd, writefd;
    pid_t childpid;
    if((mkfifo(FIFO1, FILE_MODE)) < 0 && (errno != EEXIST))
        err_sys("can't create ");
    if((mkfifo(FIFO2, FILE_MODE)) < 0 && (errno != EEXIST)) {
        unlink(FIFO1);
        err_sys("can't create ");
    }
    if((childpid = Fork()) == 0) { // child
        readfd = Open(FIFO1, O_RDONLY, 0);
        writefd = Open(FIFO2, O_WRONLY, 0);
        server(readfd, writefd);
    }
    writefd = Open(FIFO1, O_WRONLY, 0);
    readfd = Open(FIFO2, O_RDONLY, 0);
    client(readfd, writefd);
    Waitpid(childpid, NULL, 0);
    Close(readfd);
    Close(writefd);
    unlink(FIFO2);
    unlink(FIFO1);
    exit(0);
}