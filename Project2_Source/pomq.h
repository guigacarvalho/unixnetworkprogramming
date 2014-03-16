#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <mqueue.h>

typedef	unsigned int	uint_t; 

#define MSG_R 0400 
#define MSG_W 0200
#define	SVMSG_MODE	(MSG_R | MSG_W | MSG_R>>3 | MSG_R>>6)
#define	MAXMESGDATA	(PIPE_BUF - 2*sizeof(long))
#define	MESGHDRSIZE	(sizeof(struct mymesg) - MAXMESGDATA)
#define FILE_MODE       (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

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
