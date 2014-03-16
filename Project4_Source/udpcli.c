#include	"unp.h"

void client (FILE *, int,const  SA* , socklen_t);

int main(int argc, char **argv)
{
	int					sockfd;
	struct sockaddr_in	servaddr;


	FILE *fp;
	fp = fopen(SOCKETSHAREDFILE, "r");
	char    ipaddr[100];char port[100];
	fscanf(fp, "%s %s", ipaddr, port);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(port));
	inet_pton(AF_INET, ipaddr, &servaddr.sin_addr);

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	client(stdin, sockfd, (SA*) &servaddr, sizeof(servaddr));		/* do it all */

	exit(0);
}

void client(FILE *fp, int sockfd, const SA *pservaddr, socklen_t servlen)
{
	int	n;
	char	sendline[MAXLINE], recvline[MAXLINE + 1];

	while (fgets(sendline, MAXLINE, fp) != NULL) {

		// Sending command to the server process
		sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen);

		//Exit command
		if(!strcmp(sendline, "EXIT\n"))
			exit(0);

		//Cleaning the Receive Buffer
		memset(recvline,0,MAXLINE);

		//Reading response from the server process
		n = recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);
		recvline[n] = 0;	/* null terminate */

		//Displaying response for the end user
		fputs(recvline, stdout);
	}
}
