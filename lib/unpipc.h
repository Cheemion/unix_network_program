#include <sys/stat.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>
#define MSG_R		0400		/* or S_IRUGO from <linux/stat.h> */
#define MSG_W		0200		/* or S_IWUGO from <linux/stat.h> */

#define SEM_R		0400		/* or S_IRUGO from <linux/stat.h> */
#define SEM_A		0200		/* or S_IWUGO from <linux/stat.h> */

#define SHM_R		0400		/* or S_IRUGO from <linux/stat.h> */
#define SHM_W		0200		/* or S_IWUGO from <linux/stat.h> */

#define	SVMSG_MODE	(MSG_R | MSG_W | MSG_R>>3 | MSG_R>>6)
					/* default permissions for new SV message queues */
/* $$.ix [SVMSG_MODE]~constant,~definition~of$$ */
#define	SVSEM_MODE	(SEM_R | SEM_A | SEM_R>>3 | SEM_R>>6)
					/* default permissions for new SV semaphores */
/* $$.ix [SVSEM_MODE]~constant,~definition~of$$ */
#define	SVSHM_MODE	(SHM_R | SHM_W | SHM_R>>3 | SHM_R>>6)
					/* default permissions for new SV shared memory */
/* $$.ix [SVSHM_MODE]~constant,~definition~of$$ */

void err_sys(char* chars) {
    printf("%s\n", chars);
    fflush(stdout);
    exit(1);
}
int Msgget(key_t key, int flag) {
	int		rc;
	if ( (rc = msgget(key, flag)) == -1)
		err_sys("msgget error");
	return(rc);
}

void Msgctl(int id, int cmd, struct msqid_ds *buf) {
	if (msgctl(id, cmd, buf) == -1)
		err_sys("msgctl error");
}

void Msgsnd(int id, const void *ptr, size_t len, int flag) {
	if (msgsnd(id, ptr, len, flag) == -1)
		err_sys("msgsnd error");
}

ssize_t Msgrcv(int id, void *ptr, size_t len, int type, int flag) {
	ssize_t	rc;

	if ( (rc = msgrcv(id, ptr, len, type, flag)) == -1)
		err_sys("msgrcv error");
	return(rc);
}