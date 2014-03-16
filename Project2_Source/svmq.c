#include	"svmq.h"

void	client(int, int), server(int, int);

int
main(int argc, char **argv)
{
	//Variables and Message Queues initialization
	int	readid,writeid, childpid;
	readid = msgget(MQ_KEY1, SVMSG_MODE | IPC_CREAT);
	writeid = msgget(MQ_KEY2, SVMSG_MODE | IPC_CREAT);

	//Initiating server process
	if ( (childpid = fork()) == 0) {
		server(readid, writeid);
	}

	//Initiating client process
	client(writeid, readid);

	exit(0);
}

//Receiving message function
ssize_t
mesg_recv(int id, struct mymesg *mptr)
{
	ssize_t	n;

	n = msgrcv(id, &(mptr->mesg_type), MAXMESGDATA, mptr->mesg_type, 0);
	mptr->mesg_len = n;

	return(n);
}

//Sending message function
ssize_t
mesg_send(int id, struct mymesg *mptr)
{
	return(msgsnd(id, &(mptr->mesg_type), mptr->mesg_len, 0));
}

void
client(int readfd, int writefd)
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
server(int readfd, int writefd)
{
	FILE	*fp;
	ssize_t	n;
	struct mymesg	mesg;
	char    file[100], cmd[100], msg[100];;

	for ( ; ; ) {
		//Reading command from server
		mesg.mesg_type = 1;
		if ( (n = mesg_recv(readfd, &mesg)) == 0) 
			printf("pathname missing");

		mesg.mesg_data[n]='\0';//Null terminate command

		//Parsing command and filename
		sscanf(mesg.mesg_data, "%s %s", cmd, file);

		//Executing command READ and sending response
		if(!strcmp(cmd, "READ")){
			if ( (fp = fopen(file, "r")) == NULL) {
				/* 4error: must tell client */
				snprintf(mesg.mesg_data + n, sizeof(mesg.mesg_data) - n,
						 ": can't open, %s\n", strerror(errno));
				mesg.mesg_len = strlen(mesg.mesg_data);
				mesg_send(writefd, &mesg);
		
			} else {
				/* 4fopen succeeded: copy file to IPC channel */
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
			msgctl(readfd, IPC_RMID, NULL);
			msgctl(writefd, IPC_RMID, NULL);
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
