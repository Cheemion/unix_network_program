#include "unp.h"
int Socket(int family, int type, int protocal) {
    int n;
    if((n = socket(family, type, protocal)) < 0) {
        err_sys("socket error");
    }
    return n;
}
