#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <mqueue.h>
#include <sys/stat.h>	
#include	<sys/mman.h>	/* Posix shared memory */
#include	<semaphore.h>	/* Posix semaphores */

#ifndef	SEM_FAILED
#define	SEM_FAILED	((sem_t *)(-1))
#endif

#ifndef max
	#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif

#ifndef	MAP_FAILED
#define	MAP_FAILED	((void *)(-1))
#endif


#ifdef	HAVE_SYS_MMAN_H
# include	<sys/mman.h>	/* Posix shared memory */
#endif

typedef	unsigned int	uint_t; 
#define	MAXLINE		4096
#define MSG_R 0400 
#define MSG_W 0200
#define	SVMSG_MODE	(MSG_R | MSG_W | MSG_R>>3 | MSG_R>>6)
#define	MAXMESGDATA	(PIPE_BUF - 2*sizeof(long))
#define	MESGHDRSIZE	(sizeof(struct mymesg) - MAXMESGDATA)
#define FILE_MODE       (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define CLOCK "posemc"
#define SLOCK "posems"
#define SHMPATH "/var/tmp/poshm"

struct mymesg {
  long	mesg_len;	/* #bytes in mesg_data, can be 0 */
  long	mesg_type;	/* message type, must be > 0 */
  char	mesg_data[MAXMESGDATA];
};

ssize_t	 mesg_send(int, struct mymesg *);
ssize_t	 mesg_recv();

struct mymq_attr {
  long	mq_flags;		/* message queue flag: O_NONBLOCK */
  long	mq_maxmsg;		/* max number of messages allowed on queue */
  long	mq_msgsize;		/* max size of a message (in bytes) */
  long	mq_curmsgs;		/* number of messages currently on queue */
};

char *
px_ipc_name(const char *name)
{
	char	*dir, *dst, *slash;

	if ( (dst = malloc(PATH_MAX)) == NULL)
		return(NULL);

		/* 4can override default directory with environment variable */
	if ( (dir = getenv("PX_IPC_NAME")) == NULL) {
#ifdef	POSIX_IPC_PREFIX
		dir = POSIX_IPC_PREFIX;		/* from "config.h" */
#else
		dir = "/tmp/";				/* default */
#endif
	}
		/* 4dir must end in a slash */
	slash = (dir[strlen(dir) - 1] == '/') ? "" : "/";
	snprintf(dst, PATH_MAX, "%s%s%s", dir, slash, name);

	return(dst);			/* caller can free() this pointer */
}
