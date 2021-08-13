#include <sys/socket.h>
#include <stdio.h>
#include <iostream>
static int i;
static int j;
int main() {
    std::cout << i << std::endl;
    printf("%d", i);
    printf("%d", j);
    return 0;
}