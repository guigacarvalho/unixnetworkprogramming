#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>


#define	MAXLINE		4096

void	client(int, int), server(int, int);

int main(int argc, char **argv) {

	//Initialazing variables
	int		pipe1[2], pipe2[2];
	pid_t	childpid;

	// Creating pipes
	pipe(pipe1);
	pipe(pipe2);

	//Initiating server process
	if ( (childpid = fork()) == 0) {		
		close(pipe1[1]);
		close(pipe2[0]);
		server(pipe1[0], pipe2[1]);
	}

	//Initiating client process
	close(pipe1[0]);
	close(pipe2[1]);
	client(pipe2[0], pipe1[1]);

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
	    } else if (!strcmp(cmd, "EXIT")) {
		    	close(readfd);
		    	close(writefd);
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
