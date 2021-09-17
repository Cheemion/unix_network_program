#include "./lib/wrap.h"
int main(int argc, char** argv) {
    char* ptr, ** pptr;
    char str[INET_ADDRSTRLEN];
    struct hostent* hptr;
    while(--argc > 0) {
        ptr = *++argv;
        if((hptr = gethostbyname(ptr)) == NULL) {
            printf("gethostbyname error for host: %s %s \n", ptr, hstrerror(h_errno));
            continue;
        }
        printf("official hostname: %s \n", hptr->h_name);
        switch (hptr->h_addrtype)
        {
        case AF_INET:
            pptr = hptr->h_addr_list;
            for( ; *pptr != NULL; pptr++) {
                printf("\t address : %s \n", Inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str)));
            }
            break;
        default:
            printf("unknown address type");
            break;
        }
    }
}