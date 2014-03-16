#include	"unp.h"

void client(FILE *, int);

int main(int argc, char **argv)
{
	int	sockfd;
	struct sockaddr_in	servaddr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	FILE *fp;
	fp = fopen(SOCKETSHAREDFILE, "r");
	char    ipaddr[100];char port[100];
	fscanf(fp, "%s %s", ipaddr, port);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(port));
	inet_pton(AF_INET, ipaddr, &servaddr.sin_addr);

	connect(sockfd, (SA *) &servaddr, sizeof(servaddr));

	client(stdin, sockfd);		/* do it all */

	exit(0);
}

void client(FILE *fp, int sockfd)
{
	char	sendline[MAXLINE], recvline[MAXLINE];

	while (fgets(sendline, MAXLINE, fp) != NULL) {
		
		// Sending command to the server process
		write(sockfd, sendline, strlen(sendline));

		//Exit command
		if(!strcmp(sendline, "EXIT\n"))
			exit(0);

		//Cleaning the Receive Buffer
		memset(recvline,0,MAXLINE);

		//Reading response from the server process
		if (read(sockfd, recvline, MAXLINE) == 0)
			printf("str_cli: server terminated prematurely");

		//Displaying response for the end user
		fputs(recvline, stdout);
	}
}
