#pragma once
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include<arpa/inet.h>
#define	MAXLINE		4096	/* max text line length */
void err_sys(char* chars) {
    printf("%s\n", chars);
    fflush(stdout);
    exit(1);
}
int Socket(int family, int type, int protocal) {
    int n;
    if((n = socket(family, type, protocal)) < 0) {
        printf("socket error\n");
        exit(1);
    }
    return n;
}

void Bind(int listenfd, struct sockaddr* servaddr, socklen_t len) {
    if(bind(listenfd, servaddr, len) < 0) {
        printf("bind error on fd %d \n", listenfd);
        exit(1);
    }
}
int Listen(int listenfd, int queue_size) {
    if(listen(listenfd, queue_size) < 0) {
        printf("listen error on fd %d \n", listenfd);
        exit(1);
    }
}

int Accept(int fd, struct sockaddr * address, socklen_t * address_len) {
    int n;
    if((n = accept(fd, address, address_len)) < 0) {
        printf("accept error on fd %d \n", fd);
        exit(1);
    }
    return n;
}

int Fork() {
    int n;
    if((n = fork()) < 0) {
        printf("fork error\n");
        exit(1);
    }
    return n;
}

void Close(int fd)  {
    if(close(fd) < 0) {
        printf("close fd %d result in error", fd);
        exit(1);
    }
}

ssize_t						/* Write "n" bytes to a descriptor. */
writen(int fd, const void *vptr, size_t n)
{
	size_t		nleft;
	ssize_t		nwritten;
	const char	*ptr;
	ptr = (const char*)vptr;
	nleft = n;
	while (nleft > 0) {
		if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
			if (nwritten < 0 && errno == EINTR)
				nwritten = 0;		/* and call write() again */
			else
				return(-1);			/* error */
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}
	return(n);
}

void Writen(int fd, void *ptr, size_t nbytes)
{
	if (writen(fd, ptr, nbytes) != nbytes)
		err_sys("writen error");
}

void Connect(int sockfd, struct sockaddr* address, socklen_t address_len) {
    if(connect(sockfd, address, address_len) < 0) {
        err_sys("connect error");
    }
}

void Inet_pton(int family, char* presentation, void* buf) {
    if(inet_pton(AF_INET, presentation, buf) <= -1) {
        err_sys("inet_pton errorr");
    }
}




char* Fgets(char *ptr, int n, FILE *stream)
{
	char* rptr;
	if ((rptr = fgets(ptr, n, stream)) == NULL && ferror(stream))
		err_sys("fgets error");
	return (rptr);
}


static int read_cnt; //default 0
static char* read_ptr;
static char read_buf[MAXLINE];

static ssize_t my_read(int fd, char* ptr) {
    if(read_cnt <= 0) {
        again:
            if((read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
                if(errno == EINTR)
                    goto again;
                return -1;
            } else if (read_cnt == 0) //EOF
                return 0;
            read_ptr = read_buf;
    }
    read_cnt--;
    *ptr = *read_ptr++; //每次读取一个字符
    return 1;
}
ssize_t readline(int fd, void *vptr, size_t maxlen) {
	ssize_t	n, rc;
	char	c, *ptr;
	ptr = vptr;
	for (n = 1; n < maxlen; n++) {
		if ( (rc = my_read(fd, &c)) == 1) {
			*ptr++ = c;
			if (c == '\n')
				break;	/* newline is stored, like fgets() */
		} else if (rc == 0) {
			*ptr = 0;
			return(n - 1);	/* EOF, n - 1 bytes were read */
		} else
			return(-1);		/* error, errno set by read() */
	}
	*ptr = 0;	/* null terminate like fgets() */
	return(n);
}
ssize_t Readline(int fd, void *ptr, size_t maxlen) {
	ssize_t n;
	if ( (n = readline(fd, ptr, maxlen)) < 0)
		err_sys("readline error");
	return(n);
}

void Fputs(const char *ptr, FILE *stream) {
	if (fputs(ptr, stream) == EOF)
		err_sys("fputs error");
}
