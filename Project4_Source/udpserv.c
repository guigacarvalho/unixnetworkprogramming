#include	"unp.h"

void server(int, SA *, socklen_t);

int main(int argc, char **argv)
{
	int					listenfd, connfd,sockfd;
	pid_t				childpid;
	socklen_t			clilen;
	struct sockaddr_in	cliaddr, servaddr;
	char * ipStr;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(0); //Getting unique port number from the kernel

	bind(sockfd, (SA *) &servaddr, sizeof(servaddr));

	//Getting port number
	int addrlen = sizeof(servaddr);
	getsockname(sockfd,(struct sockaddr*)&servaddr,&addrlen);  
	int port=ntohs(servaddr.sin_port);

	//Getting hostname
	char hostname[128];
	gethostname(hostname, sizeof hostname);
	
	//Getting IP Address
	struct hostent *h;
	struct in_addr **addr_list;
	h=gethostbyname(hostname);
	addr_list = (struct in_addr **)h->h_addr_list;
	//Displaying info to the user	
	printf("-server: %s running @ %s:%d\n", hostname, inet_ntoa(*addr_list[0]), port);

	//Writing network information into a file
	FILE*fp;
	fp = fopen(SOCKETSHAREDFILE, "w");
	fprintf(fp, "%s %d", inet_ntoa(*addr_list[0]), port);
	fclose(fp);

	server(sockfd, (SA *) &cliaddr, sizeof(cliaddr));

}

void server(int sockfd, SA *pcliaddr, socklen_t clilen)
{
	int		n;
	socklen_t	len;
	char		mesg[MAXLINE];
	char    file[100], cmd[100];
	char	*ptr;
	char	buff[MAXLINE];
	int fd;

	for ( ; ; ) {

			len = clilen;
			n = recvfrom(sockfd, mesg, MAXLINE, 0, pcliaddr, &len);

			//Parsing command and cleaning memorysockfd
			sscanf(mesg, "%s %s", cmd, file);

			//Executing command READ and sending response
			if(!strcmp(cmd, "READ")){
				if ( (fd = open(file, O_RDONLY)) < 0) {
					snprintf(buff, sizeof(buff), "Problems opening '%s': %s.\n", file, strerror(errno));
					n = strlen(buff);
					sendto(sockfd, buff, n, 0, pcliaddr, len);
				} else {
					while ( (n = read(fd, buff, MAXLINE)) > 0)
						sendto(sockfd, buff, n, 0, pcliaddr, len);
					close(fd);
			}

			//Executing command DELETE and sending response
		    } else if (!strcmp(cmd, "DELETE")){
			    	if (!remove(file))
						snprintf(buff, sizeof(buff), "Deleted successfully.\n");
					else 
						snprintf(buff, sizeof(buff), "Problems deleting '%s': %s.\n", file, strerror(errno));
						n = strlen(buff);
						sendto(sockfd, buff, n, 0, pcliaddr, len);

			//Executing command EXIT and server clean up
			} else if (!strcmp(cmd, "EXIT")) {
					remove(SOCKETSHAREDFILE);
				    	exit(0);
			}

			// Invalid Command
			else   {
			    	snprintf(buff, sizeof(buff),"Invalid command. Try READ, DELETE or EXIT.\n"); 
				n = strlen(buff);
				sendto(sockfd, buff, n, 0, pcliaddr, len);
			}
			memset(buff,0,MAXLINE);
	}
}


