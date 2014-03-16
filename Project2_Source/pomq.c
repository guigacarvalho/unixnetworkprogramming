#include	"pomq.h"

void	client(int, int), server(int, int);
struct mq_attr	attr;	

int
main(int argc, char **argv)
{
	//Variables and Message Queues initialization
	int	c, flags;
	mqd_t	readmqd, writemqd;
	int	childpid;
	flags = O_RDWR | O_CREAT;
	readmqd = mq_open("/mq2", flags, FILE_MODE,
				  (attr.mq_maxmsg != 0) ? &attr : NULL);
	writemqd = mq_open("/mq1", flags, FILE_MODE,
				  (attr.mq_maxmsg != 0) ? &attr : NULL);

	//Initiating server process
	if ( (childpid = fork()) == 0) {
		server(readmqd, writemqd);
	}

	//Initiating client process
	client(writemqd, readmqd);

	exit(0);
}

//Receiving message function
ssize_t
mesg_recv(mqd_t mqd, struct mymesg *mptr)
{

	int	 i;
	ssize_t	n;
	uint_t	prio=1;
	struct mq_attr	attr;

	mq_getattr(mqd, &attr);

	i = mq_receive(mqd, mptr->mesg_data, attr.mq_msgsize, &prio);
	return(i);
}

//Sending message function
ssize_t
mesg_send(mqd_t mqd, struct mymesg *mptr)
{
	return(mq_send(mqd, mptr->mesg_data, mptr->mesg_len, 1));

}

void
client(mqd_t readfd, mqd_t writefd)
{
	size_t	len;
	ssize_t	n;
	struct mymesg	mesg;

	// Reading user input
	while (fgets(mesg.mesg_data, MAXMESGDATA, stdin) != NULL) {
		len = strlen(mesg.mesg_data);
		if (mesg.mesg_data[len-1] == '\n')
			len--;

		// Sending command to the server process
		mesg.mesg_len = len;
		mesg.mesg_type = 1;
		mesg_send(writefd, &mesg);

		//Exit command
		if(!strcmp(mesg.mesg_data, "EXIT\n"))
			exit(0);
	
		// Reading response from the server process	
		while ( (n = mesg_recv(readfd, &mesg)) > 0)
			write(STDOUT_FILENO, mesg.mesg_data, n);

	}
}


void
server(mqd_t readfd, mqd_t writefd)
{
	FILE	*fp;
	ssize_t	n;
	struct mymesg	mesg;
	char    file[100], cmd[100];


	for ( ; ; ) {
		//Reading command from server
		mesg.mesg_type = 1;
		if ( (n = mesg_recv(readfd, &mesg)) == 0) 
			printf("pathname missing");
		mesg.mesg_data[n]='\0'; //Null terminate command

		//Parsing command and filename
		sscanf(mesg.mesg_data, "%s %s", cmd, file);

		//Executing command READ and sending response
		if(!strcmp(cmd, "READ")){
			if ( (fp = fopen(file, "r")) == NULL) {
				snprintf(mesg.mesg_data + n, sizeof(mesg.mesg_data) - n,
						 ": can't open, %s\n", strerror(errno));
				mesg.mesg_len = strlen(mesg.mesg_data);
				mesg_send(writefd, &mesg);
		
			} else {
				while (fgets(mesg.mesg_data, MAXMESGDATA, fp) != NULL) {
					mesg.mesg_len = strlen(mesg.mesg_data);
					mesg_send(writefd, &mesg);
				}
				fclose(fp);
			}

		//Executing command DELETE and sending response
		} else if (!strcmp(cmd, "DELETE")){
			if (!remove(file))
				snprintf(mesg.mesg_data, sizeof(mesg.mesg_data), "Deleted successfully.\n");
			else 
				snprintf(mesg.mesg_data, sizeof(mesg.mesg_data), "Problems deleting '%s': %s.\n", file, strerror(errno));
				mesg.mesg_len = strlen(mesg.mesg_data);				
				mesg_send(writefd, &mesg);

		//Executing command EXIT and cleaning up
	    	} else if (!strcmp(cmd, "EXIT")) {
			mq_close(readfd);
			mq_close(writefd);
			mq_unlink("/mq1");
			mq_unlink("/mq2");
			exit(0);

   		}	
		
		// Invalid Command
		else   {
	    		snprintf(mesg.mesg_data, sizeof(mesg.mesg_data),"Invalid command. Try READ, DELETE or EXIT.\n"); 
			mesg.mesg_len = strlen(mesg.mesg_data);
 			mesg_send(writefd, &mesg);
		}

		mesg.mesg_len = 0;
		mesg_send(writefd, &mesg);
	}
}
