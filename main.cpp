#include <sys/socket.h>
#include <stdio.h>
#include <iostream>
static int i;
static int j;

struct B {
    int k = 1;
};

struct A {
    struct B b;
#define k b.k
};

/* Type of a signal handler.  */
typedef void (*test) (int);
#define	SIG_ERR ((test) 2)	/* Error return.  */

int main() {
    A a;
    std::cout << a.k << std::endl;
    std::cout << 2 << std::endl;
    auto x = SIG_ERR;
    //printf("%d\n", x);
    std::cout << x << std::endl;
    return 0;
}