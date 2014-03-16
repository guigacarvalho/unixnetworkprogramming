#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>


#define	MAXLINE		4096
#define FIFO1 "/var/tmp/fifo.1"
#define FIFO2 "/var/tmp/fifo.2"
#define	FILE_MODE	(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)


void	client(int, int), server(int, int);

int main(int argc, char **argv) {

	//Variables
	int readfd, writefd;
	pid_t	childpid;

	// Creating fifos
	if ((mkfifo(FIFO1, FILE_MODE) < 0) && (errno != EEXIST))
		printf("can't create fifo.1");
	if ((mkfifo(FIFO2, FILE_MODE) < 0) && (errno != EEXIST)) {
		unlink(FIFO1);
		printf("can't create fifo.2");
	}


	//Initiating server process
	if ( (childpid = fork()) == 0) {		
		readfd = open(FIFO1, O_RDONLY, 0);
		writefd = open(FIFO2, O_WRONLY, 0);
		server(readfd, writefd);
	}

	//Initiating client process
	writefd = open(FIFO1, O_WRONLY, 0);
	readfd = open(FIFO2, O_RDONLY, 0);
	client(readfd, writefd);
}


void
client(int readfd, int writefd)
{
	size_t	len;
	ssize_t	n;
	char	buff[MAXLINE];

	// Reading user input
  	while (fgets(buff, 4096, stdin) != NULL) {
		len = strlen(buff);
		if (buff[len - 1] == '\n')
			len--;	

		// Sending command to the server process
		write(writefd, buff, len);

		//Exit command
		if(!strcmp(buff, "EXIT\n"))
			exit(0);

		// Reading response from the server process
		if ( (n = read(readfd, buff, MAXLINE)) > 0) { 
			write(STDOUT_FILENO, buff, n);
		}

	}
}


void
server(int readfd, int writefd)
{
	int len;
	int		fd;
	ssize_t	n;
	char	buff[MAXLINE+1];
	char    file[100], cmd[100];

	//Reading command from server
	while((n = read(readfd, buff, MAXLINE))!=0) {
		buff[n] = '\0'; //Null terminate command

		//Parsing command and filename
		sscanf(buff, "%s %s", cmd, file);

		//Executing command READ and sending response
	    if(!strcmp(cmd, "READ")){
			if ( (fd = open(file, O_RDONLY)) < 0) {
				snprintf(buff, sizeof(buff), "Problems opening '%s': %s.\n", file, strerror(errno));
				n = strlen(buff);
				write(writefd, buff, n);
			} else {
				while ( (n = read(fd, buff, MAXLINE)) > 0)
					write(writefd, buff, n);
				close(fd);
			}

		//Executing command DELETE and sending response
	    } else if (!strcmp(cmd, "DELETE")){
			    	if (!remove(file))
						snprintf(buff, sizeof(buff), "Deleted successfully.\n");
					else 
						snprintf(buff, sizeof(buff), "Problems deleting '%s': %s.\n", file, strerror(errno));
						n = strlen(buff);
						write(writefd, buff, n);

		//Executing command EXIT and cleaning up
	    } else if (!strcmp(cmd, "EXIT\n")) {
			close(readfd);
			close(writefd);
		    	unlink("/var/tmp/fifo.1");
		    	unlink("/var/tmp/fifo.2");
		    	exit(0);
		}

	    // Invalid Command
	    else   {
	    	snprintf(buff, sizeof(buff),"Invalid command. Try READ, DELETE or EXIT.\n"); 
				n = strlen(buff);
				write(writefd, buff, n);
		}
	}
}
