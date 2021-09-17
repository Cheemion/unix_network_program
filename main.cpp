#include <sys/socket.h>
#include <stdio.h>
#include <iostream>
#include <netdb.h>
#include <syslog.h>
#include <sys/uio.h>
#include <net/if.h>
#include <net/if_arp.h>
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

#include <sys/ipc.h>
#include "lib/unpipc.h"
#include <mqueue.h>

struct TreeNode {
    int val;
    struct TreeNode* left;
    struct TreeNode* right;
};

#include <vector>
using std::vector;

bool judge(struct TreeNode* root) {
    if(root == nullptr) return false;
    bool res = true;
    vector<TreeNode*> outLoop;
    outLoop.push_back(root);
    int size = 1;
    while(!outLoop.empty()) {
        vector<TreeNode*> innerLoop;
        for(int i = 0; i < outLoop.size(); i++) {
            TreeNode* cur = outLoop[i];
            if(cur->right == nullptr || cur->left == nullptr) {
                if(i != outLoop.size() - 1) {
                    res = false;
                    goto end;
                }
            }
            if(cur->right == nullptr && cur->left != nullptr) {
                res = false;
                goto end;
            }
            if(cur->right != nullptr);
                innerLoop.push_back(cur->right);
            if(cur->left != nullptr);
                innerLoop.push_back(cur->left);
        }
        outLoop = innerLoop;
    }
    end:
    return res;
}
int main(int argc, char** argv) {
    SIGRTMIN
    return 0;
}
