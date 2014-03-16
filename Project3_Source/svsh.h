#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/shm.h>
#include <sys/ipc.h>	
#include <sys/sem.h>	
#include <sys/msg.h>	
#include <sys/stat.h>
#include <sys/types.h>

#define CLOCK "/var/tmp/svsemc"
#define SLOCK "/var/tmp/svsems"
#define SHMPATH "/var/tmp/svshm"

#define MSG_R 0400 
#define SEM_R 0400 
#define SEM_A 0200
#define MSG_W 0200
#define	SVMSG_MODE	(MSG_R | MSG_W | MSG_R>>3 | MSG_R>>6)
#define	SVSEM_MODE	(SEM_R | SEM_A | SEM_R>>3 | SEM_R>>6)
#define	SVSHM_MODE	(SHM_R | SHM_W | SHM_R>>3 | SHM_R>>6)

union semun {
  int              val;
  struct semid_ds *buf;
  unsigned short  *array;
};

#define	MAXLINE		4096
#define	MAXMESGDATA	(PIPE_BUF - 2*sizeof(long))
#define	MESGHDRSIZE	(sizeof(struct mymesg) - MAXMESGDATA)

struct mymesg {
  long	mesg_len;	/* #bytes in mesg_data, can be 0 */
  long	mesg_type;	/* message type, must be > 0 */
  char	mesg_data[MAXMESGDATA];
};

