#include "../lib/wrap.h"

int main(int argc, char** argv) {
    if(argc != 2)
        err_sys("usage: mqunlink<name>");
    mq_unlink(argv[1]);
    return 0;
}