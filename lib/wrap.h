#pragma once
#define _DEFAULT_SOURCE
#include <features.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h> 
#include <netinet/in.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/time.h>
#define INFTIM          (-1)    /* infinite poll timeout */
#define	LISTENQ		1024	/* 2nd argument to listen() */
#define	SERV_PORT	9877			/* TCP and UDP */
#define	MAXLINE		4096	/* max text line length */
#define	SA	struct sockaddr
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
void
Inet_pton(int family, const char *strptr, void *addrptr)
{
	int		n;

	if ( (n = inet_pton(family, strptr, addrptr)) < 0)
		err_sys("inet_pton error for");	/* errno set */
	else if (n == 0)
		err_quit("inet_pton error for");	/* errno not set */

	/* nothing to return */
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
	ptr = (char*)vptr;
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


int Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout) {
	int	n;
	if ( (n = select(nfds, readfds, writefds, exceptfds, timeout)) < 0)
		err_sys("select error");
	return(n);		/* can return 0 on timeout */
}



typedef	void	Sigfunc(int);	/* for signal handlers */

Sigfunc * signal(int signo, Sigfunc *func) {
	struct sigaction	act, oact;

	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if (signo == SIGALRM) {
#ifdef	SA_INTERRUPT
		act.sa_flags |= SA_INTERRUPT;	/* SunOS 4.x */ // 
#endif
	} else {
#ifdef	SA_RESTART
		act.sa_flags |= SA_RESTART;		/* SVR4, 44BSD */
#endif
	}
	if (sigaction(signo, &act, &oact) < 0)
		return(SIG_ERR);
	return(oact.sa_handler);
}
/* end signal */

Sigfunc* Signal(int signo, Sigfunc *func)	/* for our signal() function */ {
	Sigfunc	*sigfunc;

	if ( (sigfunc = signal(signo, func)) == SIG_ERR)
		err_sys("signal error");
	return(sigfunc);
}


ssize_t Read(int fd, void *ptr, size_t nbytes) {
	ssize_t		n;
	if ( (n = read(fd, ptr, nbytes)) == -1)
		err_sys("read error");
	return(n);
}

void Write(int fd, void *ptr, size_t nbytes)
{
	if (write(fd, ptr, nbytes) != nbytes)
		err_sys("write error");
}
void Shutdown(int fd, int how)
{
	if (shutdown(fd, how) < 0)
		err_sys("shutdown error");
}

int Poll(struct pollfd *fdarray, unsigned long nfds, int timeout) {
	int		n;

	if ( (n = poll(fdarray, nfds, timeout)) < 0)
		err_sys("poll error");

	return(n);
}



#define	min(a,b)	((a) < (b) ? (a) : (b))
#define	max(a,b)	((a) > (b) ? (a) : (b))

ssize_t
Recvfrom(int fd, void *ptr, size_t nbytes, int flags,
		 struct sockaddr *sa, socklen_t *salenptr)
{
	ssize_t		n;

	if ( (n = recvfrom(fd, ptr, nbytes, flags, sa, salenptr)) < 0)
		err_sys("recvfrom error");
	return(n);
}

void
Sendto(int fd, const void *ptr, size_t nbytes, int flags,
	   const struct sockaddr *sa, socklen_t salen)
{
	if (sendto(fd, ptr, nbytes, flags, sa, salen) != (ssize_t)nbytes)
		err_sys("sendto error");
}
void *
Malloc(size_t size)
{
	void	*ptr;

	if ( (ptr = malloc(size)) == NULL)
		err_sys("malloc error");
	return(ptr);
}

char *
Sock_ntop(const struct sockaddr *sa, socklen_t salen)
{
	char	*ptr;

	if ( (ptr = sock_ntop(sa, salen)) == NULL)
		err_sys("sock_ntop error");	/* inet_ntop() sets errno */
	return(ptr);
}
void
Setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen)
{
	if (setsockopt(fd, level, optname, optval, optlen) < 0)
		err_sys("setsockopt error");
}

void
sig_chld(int signo)
{
	pid_t	pid;
	int		stat;

	while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0)
		printf("child %d terminated\n", pid);
	return;
}
