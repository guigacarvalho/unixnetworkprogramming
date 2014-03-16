#include	"unp.h"

void server(int);

int main(int argc, char **argv)
{
	int					listenfd, connfd;
	pid_t				childpid;
	socklen_t			clilen;
	struct sockaddr_in	cliaddr, servaddr;
	char * ipStr;

	//Creates the listening socket
	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(0); //Getting unique port number from the kernel

	bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	//Getting hostname
	char hostname[128];
	gethostname(hostname, sizeof hostname);
	
	//Getting IP Address
	struct hostent *h;
	struct in_addr **addr_list;
	h=gethostbyname(hostname);
	addr_list = (struct in_addr **)h->h_addr_list;

	//Getting port number
	int addrlen = sizeof(servaddr);
	getsockname(listenfd,(struct sockaddr*)&servaddr,&addrlen);  
	int port=ntohs(servaddr.sin_port);

	//Displaying info to the user	
	printf("-server: %s running @ %s:%d\n", hostname, inet_ntoa(*addr_list[0]), port);

	//Writing network information into a file
	FILE*fp;
	fp = fopen(SOCKETSHAREDFILE, "w");
	fprintf(fp, "%s %d", inet_ntoa(*addr_list[0]), port);
	fclose(fp);

	listen(listenfd, LISTENQ);

	for ( ; ; ) {
		clilen = sizeof(cliaddr);
		connfd = accept(listenfd, (SA *) &cliaddr, &clilen);
		close(listenfd); /* close listening socket */
		server(connfd);	/* process the requests */
	}
}
void server(int sockfd)
{
	ssize_t		n;
	char		line[MAXLINE];
	char    file[100], cmd[100];
	char	*ptr;
	char	buff[MAXLINE];
	int fd;


	for ( ; ; ) {
		if ( (n = read(sockfd, line, MAXLINE)) == 0)
			return;		/* connection closed by other end */

			//Parsing command and cleaning memory
			sscanf(line, "%s %s", cmd, file);

			//Executing command READ and sending response
			if(!strcmp(cmd, "READ")){
				if ( (fd = open(file, O_RDONLY)) < 0) {
					snprintf(buff, sizeof(buff), "Problems opening '%s': %s.\n", file, strerror(errno));
					n = strlen(buff);
					write(sockfd, buff, n);
				} else {
					while ( (n = read(fd, buff, MAXLINE)) > 0)
						write(sockfd, buff, n);
					close(fd);
			}

			//Executing command DELETE and sending response
		    } else if (!strcmp(cmd, "DELETE")){
			    	if (!remove(file))
						snprintf(buff, sizeof(buff), "Deleted successfully.\n");
					else 
						snprintf(buff, sizeof(buff), "Problems deleting '%s': %s.\n", file, strerror(errno));
						n = strlen(buff);
						write(sockfd, buff, n);

			//Executing command EXIT and server clean up
			} else if (!strcmp(cmd, "EXIT")) {
					remove(SOCKETSHAREDFILE);
					close(sockfd);
					exit(0);
			}

			// Invalid Command
			else   {
	    			snprintf(buff, sizeof(buff),"Invalid command. Try READ, DELETE or EXIT.\n"); 
				n = strlen(buff);
				write(sockfd, buff, n);
			}

			memset(buff,0,MAXLINE);
	}
}
