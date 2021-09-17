#include "../lib/wrap.h"
#define	BUFFSIZE	8192	/* buffer size for reads and writes */


ssize_t read_fd(int fd, void* ptr, size_t nbytes, int* recvfd) {
    struct msghdr msg;
    struct iovec iov[1];
    ssize_t n;
    union {
        struct cmsghdr cm;
        char control[CMSG_SPACE(sizeof(int))];
    } control_un;
    struct cmsghdr* cmptr;
    msg.msg_control = control_un.control;
    msg.msg_controllen = sizeof(control_un.control);
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    iov[0].iov_base = ptr;
    msg.msg_iovlen = 1;
    if((n = recvmsg(fd, &msg, 0)) <= 0)
        return n;
}

ssize_t
Read_fd(int fd, void *ptr, size_t nbytes, int *recvfd)
{
	ssize_t		n;

	if ( (n = read_fd(fd, ptr, nbytes, recvfd)) < 0)
		err_sys("read_fd error");

	return(n);
}



int	my_open(const char* pathname, int mode)  {
    int fd, sockfd[2], status;
    pid_t childpid;
    char c, argsockfd[10], argmode[10];
    Socketpair(AF_LOCAL, SOCK_STREAM, 0, sockfd);
    if((childpid == Fork()) == 0) { //child process
        Close(sockfd[0]);
        snprintf(argsockfd, sizeof(argsockfd), "%d", sockfd[1]);
        snprintf(argmode, sizeof(argmode), "%d", mode);
        execl("./openfile", "openfile", argsockfd, pathname, argmode, (char*) NULL);
        err_sys("execl error");
    }
    Close(sockfd[1]);
    Waitpid(childpid, &status, 0);
    if(WIFEXITED(status) == 0);
        err_sys("child did not terminate");
    if((status = WEXITSTATUS(status)) == 0)
        Read_fd(sockfd[0], &c, 1, &fd);
    else {
        errno = status;
        fd = -1;
    }
    Close(sockfd[0]);
    return fd;
}

int main(int argc, char **argv) {
	int		fd, n;
	char	buff[BUFFSIZE];
	if (argc != 2)
		err_sys("usage: mycat <pathname>");

	if ( (fd = my_open(argv[1], O_RDONLY)) < 0)
		err_sys("cannot open");
        
	while ( (n = Read(fd, buff, BUFFSIZE)) > 0)
		Write(STDOUT_FILENO, buff, n);

	exit(0);
}
