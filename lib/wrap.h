#pragma once
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define IPV6
#define MAXFD 64
#define	UNIXSTR_PATH	"/tmp/unix.str"	/* Unix domain stream cli-serv */
int		daemon_proc;		/* set nonzero by daemon_init() */
/* define if struct msghdr contains the msg_control member */
#define HAVE_MSGHDR_MSG_CONTROL 1
#define	va_mode_t	mode_t
#include <mqueue.h>
#include <unistd.h>
#include <fcntl.h>
#include<stdarg.h>
#include<stdio.h>
#include <time.h>
#include <fcntl.h>
#include <syslog.h>
#include <features.h>
#include <netdb.h>
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
#include <poll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <pthread.h>
#define INFTIM          (-1)    /* infinite poll timeout */
#define	LISTENQ		1024	/* 2nd argument to listen() */
#define	SERV_PORT	9877			/* TCP and UDP */
#define	MAXLINE		4096	/* max text line length */
#define	SA	struct sockaddr

/* include sock_ntop */
char *
sock_ntop(const struct sockaddr *sa, socklen_t salen)
{
    char		portstr[8];
    static char str[128];		/* Unix domain is largest */

	switch (sa->sa_family) {
	case AF_INET: {
		struct sockaddr_in	*sin = (struct sockaddr_in *) sa;

		if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL)
			return(NULL);
		if (ntohs(sin->sin_port) != 0) {
			snprintf(portstr, sizeof(portstr), ":%d", ntohs(sin->sin_port));
			strcat(str, portstr);
		}
		return(str);
	}
/* end sock_ntop */

#ifdef	IPV6
	case AF_INET6: {
		struct sockaddr_in6	*sin6 = (struct sockaddr_in6 *) sa;

		str[0] = '[';
		if (inet_ntop(AF_INET6, &sin6->sin6_addr, str + 1, sizeof(str) - 1) == NULL)
			return(NULL);
		if (ntohs(sin6->sin6_port) != 0) {
			snprintf(portstr, sizeof(portstr), "]:%d", ntohs(sin6->sin6_port));
			strcat(str, portstr);
			return(str);
		}
		return (str + 1);
	}
#endif

#ifdef	AF_UNIX
	case AF_UNIX: {
		struct sockaddr_un	*unp = (struct sockaddr_un *) sa;

			/* OK to have no pathname bound to the socket: happens on
			   every connect() unless client calls bind() first. */
		if (unp->sun_path[0] == 0)
			strcpy(str, "(no pathname bound)");
		else
			snprintf(str, sizeof(str), "%s", unp->sun_path);
		return(str);
	}
#endif

#ifdef	HAVE_SOCKADDR_DL_STRUCT
	case AF_LINK: {
		struct sockaddr_dl	*sdl = (struct sockaddr_dl *) sa;

		if (sdl->sdl_nlen > 0)
			snprintf(str, sizeof(str), "%*s (index %d)",
					 sdl->sdl_nlen, &sdl->sdl_data[0], sdl->sdl_index);
		else
			snprintf(str, sizeof(str), "AF_LINK, index=%d", sdl->sdl_index);
		return(str);
	}
#endif
	default:
		snprintf(str, sizeof(str), "sock_ntop: unknown AF_xxx: %d, len %d",
				 sa->sa_family, salen);
		return(str);
	}
    return (NULL);
}

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
pid_t
Fork(void)
{
	pid_t	pid;
	if ( (pid = fork()) == -1)
		err_sys("fork error");
	return(pid);
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
void Inet_pton(int family, const char *strptr, void *addrptr) {
	int		n;
	if ( (n = inet_pton(family, strptr, addrptr)) < 0) {
		printf("inet_pton error for %s", strptr);	/* errno set */
		exit(1);
	} else if (n == 0)
		printf("inet_pton error for %s", strptr);	/* errno not set */
}

const char *
Inet_ntop(int family, const void *addrptr, char *strptr, size_t len)
{
	const char	*ptr;

	if (strptr == NULL)		/* check for old code */
		err_sys("NULL 3rd argument to inet_ntop");
	if ( (ptr = inet_ntop(family, addrptr, strptr, len)) == NULL)
		err_sys("inet_ntop error");		/* sets errno */
	return(ptr);
}

struct addrinfo* host_serv(const char* host, const char* serv, int family, int socktype) {
	int n;
	struct addrinfo hints, *res;
	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_flags = AI_CANONNAME;
	hints.ai_family = family;
	hints.ai_socktype = socktype;
	if( (n = getaddrinfo(host, serv, &hints, &res)) != 0)
		return NULL;
	return res;
}
 
 // client connect to server
int tcp_connect(const char* host, const char* serv) {
	int sockfd, n;
	struct addrinfo hints, *res, *ressave;
	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if((n = getaddrinfo(host, serv, &hints, &res)) != 0) {
		err_sys("tcp_connect error");
	}
	ressave = res;
	do {
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if(sockfd < 0) continue;
		if(connect(sockfd, res->ai_addr, res->ai_addrlen) == 0) //success;
			break;
		Close(sockfd);
	} while((res = res->ai_next) != NULL);
	if(res == NULL)
		err_sys("tcp_connect error");
	freeaddrinfo(ressave);
	return sockfd;
}
int TCP_connect(const char* host, const char* serv) {
	return tcp_connect(host, serv);
}

void Getpeername(int fd, struct sockaddr *sa, socklen_t *salenptr)
{
	if (getpeername(fd, sa, salenptr) < 0)
		err_sys("getpeername error");
}
char *
sock_ntop_host(const struct sockaddr *sa, socklen_t salen)
{
    static char str[128];		/* Unix domain is largest */

	switch (sa->sa_family) {
	case AF_INET: {
		struct sockaddr_in	*sin = (struct sockaddr_in *) sa;
		if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL)
			return(NULL);
		return(str);
	}

#ifdef	IPV6
	case AF_INET6: {
		struct sockaddr_in6	*sin6 = (struct sockaddr_in6 *) sa;

		if (inet_ntop(AF_INET6, &sin6->sin6_addr, str, sizeof(str)) == NULL)
			return(NULL);
		return(str);
	}
#endif

#ifdef	AF_UNIX
	case AF_UNIX: {
		struct sockaddr_un	*unp = (struct sockaddr_un *) sa;

			/* OK to have no pathname bound to the socket: happens on
			   every connect() unless client calls bind() first. */
		if (unp->sun_path[0] == 0)
			strcpy(str, "(no pathname bound)");
		else
			snprintf(str, sizeof(str), "%s", unp->sun_path);
		return(str);
	}
#endif

#ifdef	HAVE_SOCKADDR_DL_STRUCT
	case AF_LINK: {
		struct sockaddr_dl	*sdl = (struct sockaddr_dl *) sa;

		if (sdl->sdl_nlen > 0)
			snprintf(str, sizeof(str), "%*s",
					 sdl->sdl_nlen, &sdl->sdl_data[0]);
		else
			snprintf(str, sizeof(str), "AF_LINK, index=%d", sdl->sdl_index);
		return(str);
	}
#endif
	default:
		snprintf(str, sizeof(str), "sock_ntop_host: unknown AF_xxx: %d, len %d",
				 sa->sa_family, salen);
		return(str);
	}
    return (NULL);
}

char * Sock_ntop_host(const struct sockaddr *sa, socklen_t salen)
{
	char	*ptr;
	if ( (ptr = sock_ntop_host(sa, salen)) == NULL)
		err_sys("sock_ntop_host error");	/* inet_ntop() sets errno */
	return(ptr);
}

int tcp_listen(const char* host, const char* serv, socklen_t* addrlenp) {
	int listenfd, n;
	const int on = 1;
	struct addrinfo hints, *res, *ressave;
	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if((n = getaddrinfo(host, serv, &hints, &res)) != 0)
		err_sys("tcp_listen error");
	ressave = res;
	do {
		listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if(listenfd < 0) continue;

		Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
		if(bind(listenfd, res->ai_addr, res->ai_addrlen) == 0) break;
		Close(listenfd);
	} while( (res = res->ai_next) != NULL);
	if(res == NULL)
		err_sys("tcp_listen error");
	Listen(listenfd, LISTENQ);
	if(addrlenp)
		*addrlenp = res->ai_addrlen;
	freeaddrinfo(ressave);
	return listenfd;
}

int daemon_init(const char* pname, int facility) {
	int i;
	pid_t pid;
	if((pid = Fork()) < 0)
		return -1;
	else if (pid)
		_exit(0); //parent terminates
	// child 1 continues
	if(setsid() <0) // become session leader
	Signal(SIGHUP, SIG_IGN);
	if((pid = Fork()) <0)
		return -1;
	else if (pid)
		_exit(0); // child 1 terminates
	
	daemon_proc = 1;
	chdir("/");
	for(i = 0; i < MAXFD; i++) {
		close(i);
	}

	/* redirect stdin, stdout, and stderr to /dev/null */
	open("/dev/null", O_RDONLY);
	open("/dev/null", O_RDWR);
	open("/dev/null", O_RDWR);

	openlog(pname, LOG_PID, facility);
	return 0;
}

/*
static void connect_alarm(int);

int connect_timeo(int sockfd, const SA* saptr, socklen_t salen, int nsec) {
	Sigfunc* Sigfunc;
	int n;
	Sigfunc = Signal(SIGALRM, connect_alarm);
	if(alarm(nsec) != 0) 
		err_sys("connect_timeo: alarm was already set");
	if((n = connect(sockfd, saptr, salen)) < 0) {
		close(sockfd);
		if(errno == EINTR)
			errno == ETIMEDOUT;
		alarm(0);
		Signal(SIGALRM, Sigfunc);
	}
	return n;
}
*/

void
Socketpair(int family, int type, int protocol, int *fd)
{
	int		n;
	if ( (n = socketpair(family, type, protocol, fd)) < 0)
		err_sys("socketpair error");
}

pid_t
Waitpid(pid_t pid, int *iptr, int options)
{
	pid_t	retpid;

	if ( (retpid = waitpid(pid, iptr, options)) == -1)
		err_sys("waitpid error");
	return(retpid);
}

int Fcntl(int fd, int cmd, int arg) {
	int	n;

	if ( (n = fcntl(fd, cmd, arg)) == -1)
		err_sys("fcntl error");
	return(n);
}

int connect_nonb(int sockfd, const SA* saptr, socklen_t salen, int nsec) {
	int flags, n, error;
	socklen_t len;
	fd_set rset, wset;
	struct timeval tval;
	flags = Fcntl(sockfd, F_GETFL, 0);
	Fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
	error = 0;
	if( (n = connect(sockfd, saptr, salen)) < 0)
		if(errno != EINPROGRESS)
			return -1;
	if (n == 0)
		goto done;

	FD_ZERO(&rset);
	FD_SET(sockfd, &rset);
	wset = rset;
	tval.tv_sec = nsec;
	tval.tv_usec = 0;

	if((n = Select(sockfd + 1, &rset, &wset, NULL, nsec ? &tval : NULL)) == 0){
		close(sockfd);
		errno = ETIMEDOUT;
		return -1;
	}

	if(FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset)) {
		len = sizeof(error);
		if(getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) return -1;
		else err_sys("select error: sockfd not set");
	}

	done:
		Fcntl(sockfd, F_SETFL, flags);
		if(error) {
			close(sockfd);
			errno =  error;
			return -1;
		}
		
	return 0;
}

struct unp_in_pktinfo {
	struct in_addr ipi_addr;
	int ipi_ifindex;
};


#include <sys/stat.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>

#define	FILE_MODE	(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

#define MSG_R		0400		/* or S_IRUGO from <linux/stat.h> */
#define MSG_W		0200		/* or S_IWUGO from <linux/stat.h> */

#define SEM_R		0400		/* or S_IRUGO from <linux/stat.h> */
#define SEM_A		0200		/* or S_IWUGO from <linux/stat.h> */

#define SHM_R		0400		/* or S_IRUGO from <linux/stat.h> */
#define SHM_W		0200		/* or S_IWUGO from <linux/stat.h> */

#define	SVMSG_MODE	(MSG_R | MSG_W | MSG_R>>3 | MSG_R>>6)
					/* default permissions for new SV message queues */
/* $$.ix [SVMSG_MODE]~constant,~definition~of$$ */
#define	SVSEM_MODE	(SEM_R | SEM_A | SEM_R>>3 | SEM_R>>6)
					/* default permissions for new SV semaphores */
/* $$.ix [SVSEM_MODE]~constant,~definition~of$$ */
#define	SVSHM_MODE	(SHM_R | SHM_W | SHM_R>>3 | SHM_R>>6)
					/* default permissions for new SV shared memory */
/* $$.ix [SVSHM_MODE]~constant,~definition~of$$ */


int Msgget(key_t key, int flag) {
	int		rc;
	if ( (rc = msgget(key, flag)) == -1)
		err_sys("msgget error");
	return(rc);
}

void Msgctl(int id, int cmd, struct msqid_ds *buf) {
	if (msgctl(id, cmd, buf) == -1)
		err_sys("msgctl error");
}

void Msgsnd(int id, const void *ptr, size_t len, int flag) {
	if (msgsnd(id, ptr, len, flag) == -1)
		err_sys("msgsnd error");
}

ssize_t Msgrcv(int id, void *ptr, size_t len, int type, int flag) {
	ssize_t	rc;

	if ( (rc = msgrcv(id, ptr, len, type, flag)) == -1)
		err_sys("msgrcv error");
	return(rc);
}

void Pipe(int *fds) {
	if (pipe(fds) < 0)
		err_sys("pipe error");
}
int
Open(const char *pathname, int oflag, ...)
{
	int		fd;
	va_list	ap;
	mode_t	mode;
	if (oflag & O_CREAT) {
		va_start(ap, oflag);		/* init ap to final named argument */
		mode = va_arg(ap, va_mode_t);
		if ( (fd = open(pathname, oflag, mode)) == -1)
			err_sys("open error for");
		va_end(ap);
	} else {
		if ( (fd = open(pathname, oflag)) == -1)
			err_sys("open error for");
	}
	return(fd);
}
int Getopt(int argc, char *const *argv, const char *str) {
	int		opt;

	if ( ( opt = getopt(argc, argv, str)) == '?')
		exit(1);		/* getopt() has already written to stderr */
	return(opt);
}

mqd_t Mq_open(const char *pathname, int oflag, ...) {
	mqd_t	mqd;
	va_list	ap;
	mode_t	mode;
	struct mq_attr	*attr;

	if (oflag & O_CREAT) {
		va_start(ap, oflag);		/* init ap to final named argument */
		mode = va_arg(ap, va_mode_t);
		attr = va_arg(ap, struct mq_attr *);
		if ( (mqd = mq_open(pathname, oflag, mode, attr)) == (mqd_t) -1)
			err_sys("mq_open error for ");
		va_end(ap);
	} else {
		if ( (mqd = mq_open(pathname, oflag)) == (mqd_t) -1)
			err_sys("mq_open error for");
	}
	return(mqd);
}
void Mq_close(mqd_t mqd) {
	if (mq_close(mqd) == -1)
		err_sys("mq_close error");
}
void
Mq_unlink(const char *pathname)
{
	if (mq_unlink(pathname) == -1)
		err_sys("mq_unlink error");
}

void *
Calloc(size_t n, size_t size)
{
	void	*ptr;

	if ( (ptr = calloc(n, size)) == NULL)
		err_sys("calloc error");
	return(ptr);
}
void
Mq_send(mqd_t mqd, const char *ptr, size_t len, unsigned int prio)
{
	if (mq_send(mqd, ptr, len, prio) == -1)
		err_sys("mq_send error");
}
void
Mq_getattr(mqd_t mqd, struct mq_attr *mqstat)
{
	if (mq_getattr(mqd, mqstat) == -1)
		err_sys("mq_getattr error");
}

void
Mq_setattr(mqd_t mqd, const struct mq_attr *mqstat, struct mq_attr *omqstat)
{
	if (mq_setattr(mqd, mqstat, omqstat) == -1)
		err_sys("mq_setattr error");
}

void
Mq_notify(mqd_t mqd, const struct sigevent *notification)
{
	if (mq_notify(mqd, notification) == -1)
		err_sys("mq_notify error");
}


ssize_t
Mq_receive(mqd_t mqd, char *ptr, size_t len, unsigned int *prio)
{
	ssize_t	n;

	if ( (n = mq_receive(mqd, ptr, len, prio)) == -1)
		err_sys("mq_receive error");
	return(n);
}


void
Sigemptyset(sigset_t *set)
{
	if (sigemptyset(set) == -1)
		err_sys("sigemptyset error");
}
void
Sigaddset(sigset_t *set, int signo)
{
	if (sigaddset(set, signo) == -1)
		err_sys("sigaddset error");
}

void
Sigprocmask(int how, const sigset_t *set, sigset_t *oset)
{
	if (sigprocmask(how, set, oset) == -1)
		err_sys("sigprocmask error");
}



SI_ASYNCIO