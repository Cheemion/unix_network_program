#include "../lib/wrap.h"

mqd_t mqd;
void* buff;
struct mq_attr attr;
struct sigevent sigev;

static void sig_usr1(int);

int main(int argc, char** argv) {
    if(argc != 2)
        err_sys("usage: mqnotifysig<name>");
    mqd = Mq_open(argv[1], O_RDONLY);
    Mq_getattr(mqd, &attr);
    buff = Malloc(attr.mq_msgsize);
    Signal(SIGUSR1, sig_usr1);
    sigev.sigev_notify = SIGEV_SIGNAL;
    sigev.sigev_signo = SIGUSR1;
    Mq_notify(mqd, &sigev);
    for(;;) {
        pause();
    }
    exit(0);
}

static void sig_usr1(int signo) {
    SIGALRM
    ssize_t n;
    Mq_notify(mqd, &sigev);
    n = Mq_receive(mqd, (char*)buff, attr.mq_msgsize, NULL);
    printf("SIGUSER1 received, read %ld bytes \n", (long)n);
    return ;
}