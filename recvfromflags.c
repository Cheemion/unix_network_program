#include "lib/wrap.h"

/*
ssize_t recvfrom_flags(int fd, void* ptr, size_t nbytes, int* flagsp, SA* sa, socklen_t* salenptr, struct unp_in_pktinfo* pktp) {
    struct msghdr msg;
    struct iovec iov[1];
    ssize_t n;
    struct cmsghdr* cmptr;
    union {
        struct cmsghdr cm;
    }
};
*/
