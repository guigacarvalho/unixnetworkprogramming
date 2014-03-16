#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


volatile sig_atomic_t myFlag = 0; 
struct sigaction newAction, oldAction; 
sigset_t zeromask;

static void myHandler(int); // Signal Handler
void printOutput (FILE * ); 
void sendCommand (int );
void respondCommand();

int main (int argc, char *argv[]) {

	// File handling variables
	int ch; // Variable used in File Reading
	FILE *fp1, *fp2; // File Pointer
	char    buf[4096]; // Variable used to read command line
	char    file[100], cmd[100]; // Variables for the filename and command

	// Process Handling variables
	int childpid, parentpid;

	// Setting up signals
	sigemptyset(&newAction.sa_mask); 
	newAction.sa_flags = 0;
	newAction.sa_handler = myHandler;

	sigemptyset (&oldAction.sa_mask); 
	sigemptyset(&zeromask);
	sigaddset (&newAction.sa_mask, SIGUSR1);

	sigaction(SIGUSR1, &newAction, &oldAction);

	// Creating Shared operations file
	fp1 = fopen("/var/tmp/exchange.txt", "w");
	  if (fp1 == NULL) {
	    printf("Sorry can't open /var/tmp/exchange.txt\n");
	    return 1;
	  }
	fclose(fp1);

	// Creating Server Process
	if ((childpid = fork()) == -1) {
		perror("Can't fork");
		exit(1);
	} else if (childpid != 0) {
		// Client execution code 

		// Reading command line
		printf("%% ");  // Printing prompt
	  	while (fgets(buf, 4096, stdin) != NULL) {

			// Writing Commands to file
			fp1 = fopen("/var/tmp/exchange.txt", "w");
			  if (fp1 == NULL) {
			    printf("Sorry can't open /var/tmp/exchange.txt\n");
			  } else {
			  	fprintf(fp1, "%s", buf);
			  }
			fclose(fp1);

	  		// Parsing the Commands
	  		sscanf(buf, "%s %s", cmd, file);

		    // Commands READ and DELETE
		    if(!strcmp(cmd, "READ") || !strcmp(cmd, "DELETE") || !strcmp(cmd, "EXIT")) {
			    sendCommand (childpid);
				printOutput(fp1);
				if(!strcmp(cmd, "EXIT")) {
					remove("/var/tmp/exchange.txt");
			    	exit(0);
				}
			}

		    // Invalid Command
		    else   printf("Invalid command. Try READ, DELETE or EXIT.\n"); 

			// Printing new prompt
			printf("\n%% ");
		}
		exit(0);
	} else {
		// Server execution code
		while(1) {
			// Wait for signal
			while(myFlag == 0)
				sigsuspend (&zeromask);
			myFlag=0;

			// Handle the signal: Read and execute the command from the exchange file
		  	fp1 = fopen("/var/tmp/exchange.txt", "r");
			if (fp1 == NULL) {
				//Treating problems with the exchange file opening
				printf("Sorry can't open /var/tmp/exchange.txt\n");
				exit(1);
			}
			else {
		  		// Parsing the Commands
		  		fscanf(fp1, "%s %s", cmd, file);
				fclose(fp1); 
		  		fp1 = fopen("/var/tmp/exchange.txt", "w");
			
			    // Command READ
			    if(!strcmp(cmd, "READ")) {
			    	fp2 = fopen(file, "r");
					if (fp2 == NULL) {
						//Treating problems with the exchange file opening
						fprintf(fp1, "Problems reading '%s'\n", file);
					}
					else {
						//Echoing the file read
						while( ( ch = fgetc(fp2) ) != EOF )
					      		fprintf(fp1, "%c",ch);
					}
					fclose(fp1); 
					if (fp2 != NULL) fclose(fp2); 
					respondCommand();
				}
			    // Command DELETE
			    else if(!strcmp(cmd, "DELETE")) {
			    	if (!remove(file))
						fprintf(fp1, "File '%s' deleted\n", file);
					else 
						fprintf(fp1, "Problems deleting '%s'\n", file);
					fclose(fp1); 
					respondCommand();		    	
			    }

			    // Command EXIT
			    else if(!strcmp(cmd, "EXIT")) {
					fprintf(fp1, "Server Terminated\n");
					fclose(fp1); 
					respondCommand();
				   	exit(0);
			    }
			}

	   }
	}
}

static void myHandler(int sigNo) { 
	sigprocmask (SIG_BLOCK, &newAction.sa_mask, &oldAction.sa_mask);
	myFlag = 1;
	return;
}


void printOutput (FILE * fp1){
    // Read exchange file and print it on the standard IO
	int ch;
  	FILE* fp2 = fopen("/var/tmp/exchange.txt", "r");
	if (fp2 == NULL) {
		printf("Sorry can't open /var/tmp/exchange.txt\n");
	}
	else {
		//Echoing the file read
		while( ( ch = fgetc(fp1) ) != EOF )
     		printf("%c",ch);
	}
	fclose(fp2); 
}

void sendCommand (int childpid) {
	// Send signal to server for reading the command
	kill(childpid, SIGUSR1);

	// Receive signal back from server
	while(myFlag == 0)
		sigsuspend (&zeromask);
	myFlag=0;
}

void respondCommand() {
	//Unblock signals
	sigprocmask (SIG_SETMASK, &oldAction.sa_mask, NULL);
	
	// Signal back to parent and Wait for signal again
   	kill(getppid(), SIGUSR1);
}
