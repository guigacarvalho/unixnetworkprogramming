#include "posh.h"

void	client(sem_t *, sem_t *), server(sem_t *, sem_t *);
int	fd, val;
char	*ptr;

int main(int argc, char **argv) {

	//Initialazing variables
	pid_t	childpid;
	int oflag;
	sem_t * semids, * semidc;
	oflag = O_RDWR | O_CREAT;

	//Making sure that the pathnames chosen exist
	FILE * fp = fopen(SHMPATH, "ab+");fclose(fp);

	// Creating and initializing semaphores
	semidc = sem_open(CLOCK, oflag, FILE_MODE, 1);
	semids = sem_open(SLOCK, oflag, FILE_MODE, 0);

	//Creating Shared Memory Segment
	fd = open(SHMPATH, O_RDWR | O_CREAT | O_TRUNC, FILE_MODE);
	ptr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	ftruncate(fd, 4096);

	//Initiating server process
	if ( (childpid = fork()) == 0) {
		server(semids, semidc);
	}

	//Initiating client process
	client(semids,semidc);
}

void client(sem_t * semids, sem_t * semidc)
{
	struct stat stat;
	char	buff[MAXMESGDATA];

	for (;;) {
		//Waits for the semaphore
		sem_wait(semidc);

		//Reading user input
		if(fgets(buff, 4096, stdin) != NULL)

		//Sending command to server
		snprintf(ptr, MAXMESGDATA, buff);

		//Releasing the server semaphore
		sem_post(semids);

		//Exit command
		if(!strcmp(buff, "EXIT\n")) {
			snprintf(ptr, MAXMESGDATA, buff);
			exit(0);
		}

		//Wait for the server response
		sem_wait(semidc);

		//Reading response from the server process
		fstat(fd, &stat);
		write(STDOUT_FILENO, ptr, stat.st_size);
		printf("\n");

		//Releasing the semaphore to restart the loop
		sem_post(semidc);
		
	}
}


void server(sem_t * semids, sem_t * semidc)
{
	ssize_t	n;
	char	buff[MAXMESGDATA];
	char    file[100], cmd[100];

	for (;;) {
		//Waits for the semaphore
		sem_wait(semids);

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
				sem_unlink(SLOCK);
				sem_unlink(CLOCK);
				shm_unlink(SHMPATH);
			    	exit(0);
		}

		// Invalid Command
		else   {
		    	snprintf(ptr, MAXMESGDATA,"Invalid command. Try READ, DELETE or EXIT.\n"); 
		}

		//Releasing the client semaphore
		sem_post(semidc);
	}
}
