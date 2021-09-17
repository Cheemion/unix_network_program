#include "../lib/wrap.h"

int main(int argc, char** argv) {
    int sockfd;
    struct sockaddr_un servaddr;
    sockfd = Socket(AF_LOCAL, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sun_family = AF_LOCAL;
    strcpy(servaddr.sun_path, UNIXSTR_PATH);
    Connect(sockfd, (SA *) &servaddr, sizeof(servaddr));
    exit(0) ;
}