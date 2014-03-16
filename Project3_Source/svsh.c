#include "svsh.h"

void	client(int, int), server(int, int), slock(int), srelease();

int shmid; //Shared memory identifier

int main(int argc, char **argv) {

	//Initialazing variables
	pid_t	childpid;
	int semids, semidc, oflag;
	char * prt;
	oflag = SVSEM_MODE | IPC_CREAT;
	size_t	n=MAXMESGDATA;
	union semun arg;

	//Making sure that the pathnames chosen exist
	FILE *fp = fopen(CLOCK, "ab+");fclose(fp);
	fp = fopen(SLOCK, "ab+");fclose(fp);
	fp = fopen(SHMPATH, "ab+");fclose(fp);

	// Creating semaphores
	semidc = semget(ftok(CLOCK, 0), 1, oflag);
	semids = semget(ftok(SLOCK, 0), 1, oflag);

	//Initializing semaphores (0 for the client and 1 for the server)
	arg.val = 1; semctl(semids, 0, SETVAL, arg);
	arg.val = 0; semctl(semidc, 0, SETVAL, arg);

	//Creating Shared Memory Segment
	oflag = SVSHM_MODE | IPC_CREAT;
	shmid = shmget(ftok(SHMPATH, 0), n, oflag);

	//Initiating server process
	if ( (childpid = fork()) == 0) {		
		server(semids, semidc);
	}

	//Initiating client process
	client(semids,semidc);
}

void
client(int semids, int semidc)
{
	int i=0;
	char	buff[MAXMESGDATA];
	union semun arg; arg.val=0;
	struct shmid_ds	shbuff;
	char	*ptr;

	//Attaching the shared memory to the client process
	ptr = shmat(shmid, NULL, 0);

	for (;;) {
		//Testing the semaphore
		if (semctl(semidc, 0, GETVAL, arg)==0) {

			//Reading user input
			if(fgets(buff, 4096, stdin) != NULL)

			//Cleaning memory and sending command to server
			snprintf(ptr, MAXMESGDATA, buff);

			//Locking the client semaphore and releasing the server semaphore
			arg.val=1; semctl(semidc, 0, SETVAL, arg);
			arg.val=0; semctl(semids, 0, SETVAL, arg);

			//Exit command
			if(!strcmp(buff, "EXIT\n")) {
				snprintf(ptr, MAXMESGDATA, buff);
				exit(0);
			}

			//Waiting response from the server process
			while (semctl(semids, 0, GETVAL, arg)==0);

			//Reading response from the server process
			shmctl(shmid, IPC_STAT, &shbuff);
			write(STDOUT_FILENO, ptr, shbuff.shm_segsz);
			printf("\n");
		}
	}
}


void server(int semids, int semidc)
{
	int len;
	int	fd;
	ssize_t	n;
	char	buff[MAXMESGDATA];
	char    file[100], cmd[100];
	union semun arg; arg.val=0;
	struct shmid_ds	shbuff;
	char	*ptr;

	//Attaching the shared memory to the server process
	ptr = shmat(shmid, NULL, 0);

	for (;;) {
		//Testing the semaphore
		if (semctl(semids, 0, GETVAL, arg)==0) {

			//Reading shared memory
			shmctl(shmid, IPC_STAT, &shbuff);

			//Parsing command and cleaning memory
			sscanf(ptr, "%s %s", cmd, file);
			memset(ptr, 0, MAXMESGDATA);

			//Executing command READ and sending response
			if(!strcmp(cmd, "READ")){
					if ( (fd = open(file, O_RDONLY)) < 0) {
						snprintf(ptr, MAXMESGDATA, "Problems opening '%s': %s.\n", file, strerror(errno));
					} else {
						while ( (n = read(fd, buff, MAXLINE)) > 0)
							snprintf(ptr, n, buff);
						close(fd);
			}

			//Executing command DELETE and sending response
			} else if (!strcmp(cmd, "DELETE")){
				if (!remove(file))
						snprintf(ptr, MAXMESGDATA, "Deleted successfully.\n");
					else 
						snprintf(ptr, MAXMESGDATA, "Problems deleting '%s': %s.\n", file, strerror(errno));

			//Executing command EXIT and server clean up (remove semaphores and shared memory)
			} else if (!strcmp(cmd, "EXIT")) {
					semctl(semids, 0, IPC_RMID);
			                semctl(semidc, 0, IPC_RMID);
					shmctl(shmid, IPC_RMID, NULL);
				    	exit(0);
			}

			// Invalid Command
			else   {
			    	snprintf(ptr, MAXMESGDATA,"Invalid command. Try READ, DELETE or EXIT.\n"); 
			}

			//Locking the server semaphore and releasing the client semaphore
			arg.val=1; semctl(semids, 0, SETVAL, arg);
			arg.val=0; semctl(semidc, 0, SETVAL, arg);
		}
	}
}
