#include "../lib/wrap.h"

#define uint_t unsigned int
int main(int argc, char** argv) {
    mqd_t mqd;
    void* ptr;
    size_t len;
    uint_t prio;
    if(argc != 4) {
        err_sys("usage: mqsend<name> <#bytes> <priority>");
    }
    len = atol(argv[2]);
    prio = atol(argv[2]);
    mqd = Mq_open(argv[1], O_WRONLY);
    ptr = Calloc(len ,sizeof(char));
    Mq_send(mqd, (const char*)ptr, len, prio);
    return 0;
}