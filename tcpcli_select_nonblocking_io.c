#include "lib/wrap.h"
// not worth it

char* gf_time() {
    struct timeval tv;
    static char str[30];
    char* ptr;
    if((gettimeofday(&tv, NULL)) < 0)
        err_sys("gettimeofday error");
    ptr = ctime(&tv.tv_sec);
    strcpy(str, &ptr[11]);
    snprintf(str + 8, sizeof(str) - 8, ".%06ld", tv.tv_usec);
    return (str);
}

void str_cli(FILE*fp ,int sockfd) {
    int maxfdp1, val, stdineof;
    ssize_t n, nwritten;
    fd_set rset, wset;
    char to[MAXLINE], fr[MAXLINE];
    char *toiptr, *tooptr, *friptr, *froptr; 

    val = Fcntl(sockfd, F_GETFL, 0);
    Fcntl(sockfd, F_SETFL, val | O_NONBLOCK);
    val = Fcntl(STDIN_FILENO, F_GETFL, 0);
    Fcntl(sockfd, F_SETFL, val | O_NONBLOCK);

    toiptr = tooptr = to;
    friptr = froptr = fr;
    stdineof = 0;
    maxfdp1 = max(max(STDIN_FILENO, STDOUT_FILENO), sockfd) + 1;
    for( ; ;) {
        FD_ZERO(&rset);
        FD_ZERO(&rset);
        if(stdineof == 0 && toiptr < &to[MAXLINE])
            FD_SET(STDIN_FILENO, &rset);
        if(friptr < &fr[MAXLINE])
            FD_SET(sockfd, &rset);
        if(tooptr != toiptr)
            FD_SET(sockfd, &wset);
        if(froptr != friptr)
            FD_SET(STDOUT_FILENO, &wset);
        
        Select(maxfdp1, &rset, &wset, NULL, NULL);
        if(FD_ISSET(STDOUT_FILENO, &rset)) {
            if((n = read(STDIN_FILENO, toiptr, &to[MAXLINE] - toiptr)) < 0) {
                if(errno != EWOULDBLOCK)
                    err_sys("read error");
            } else if (n == 0) {
                fprintf(stderr, "%s: EFOT on stdin\n", gf_time());
                stdineof = 1;
                if(tooptr == toiptr)
                    Shutdown(sockfd, SHUT_WR);
            } else {
                fprintf(stderr, "%s: read %d bytes from stdin\n", gf_time(), n);
                toiptr += n;
                FD_SET(sockfd, &wset);
            }
        }

        if(FD_ISSET(sockfd, &rset)) {

        }
    }
}