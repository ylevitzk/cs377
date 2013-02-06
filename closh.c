// closh.c - CS 377, Spring 2013
// Gene Levitzky, Brian Dillmann

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

#include <pthread.h>

#define TRUE 1
#define FALSE 0

// tokenize the command string into arguments - do not modify
void readCmdTokens(char* cmd, char** cmdTokens) {
  cmd[strlen(cmd) - 1] = '\0'; // drop trailing newline
  int i = 0;
  cmdTokens[i] = strtok(cmd, " "); // tokenize on spaces
  while (cmdTokens[i++] && i < sizeof(cmdTokens)) {
    cmdTokens[i] = strtok(NULL, " ");
  }
}

// read one character of input, then discard up to the newline - do not modify
char readChar() {
  char c = getchar();
  while (getchar() != '\n');
  return c;
}

// ********* NEW ******* //
// Used only to pass arguments to threadFunction
struct thread_data {
	char** cmdTokens;
};

// ********* NEW ******* //
// What each thread will be doing
void * threadFunction (void * arg) {
	struct thread_data * data;	//	The struct where command Tokens reside
	data = (struct thread_data *) arg;	// Casting arg into thread_data struct
	
	//	This is the same as the body of the while loop
	int id = fork();
	if (id != 0) { //  This is the parent process
		waitpid(id, 0, 0); //  Wait for children to finish
	} else { //  Must be the child process
		execvp(data->cmdTokens[0], data->cmdTokens); // replaces the current process with the given program
		printf("Can't execute %s\n", data->cmdTokens[0]); // only reached if running the program failed
		exit(1);
	}	
	
	pthread_exit(NULL); //	Exit out of thread
}

// ********* NEW ******* //
//	Creates count number of threads, and sends them off to perform "loadFunction"
void executeInParallel (int count, char** cmdTokens) {
	struct thread_data data; //	For passing arguments to threadFunction
	data.cmdTokens = cmdTokens;	//	Set the data in the struct
	
	pthread_t threads[count]; //	Thread array

	for(int i = 0; i < count; i++) {		
		pthread_create(&threads[i], NULL, &threadFunction, &data); //	Create the threads
		pthread_join(threads[i], NULL);	//	Wait for them to finish before continuing. 
	}
}

// ********* NEW ******* //
//	This is exactly what we had inside the while loop before
void executeInSequence (int count, char** cmdTokens) {
	int id;	//  ID of the process
	while (count--) { //  Execute count times, 0 == false
		id = fork();
		if (id != 0) { //  This is the parent process
			waitpid(id, 0, 0); //  Wait for children to finish
		} else { //  Must be the child process
			execvp(cmdTokens[0], cmdTokens); // replaces the current process with the given program
			printf("Can't execute %s\n", cmdTokens[0]); // only reached if running the program failed
			exit(1);
		}
	}
}

// main method - program entry point
int main() {
  char cmd[81]; // array of chars (a string)
  char* cmdTokens[20]; // array of strings
  int count; // number of times to execute command
  char parallelInput; // input to parse and determine if parallel
  int parallel; // whether to run in parallel or sequentially
  int timeout; // max seconds to run set of commands (parallel) or each command (sequentially)

  while (TRUE) { // main shell input loop

    // begin parsing code - do not modify
    printf("closh> ");
    fgets(cmd, sizeof(cmd), stdin);
    if (cmd[0] == '\n') continue;
    readCmdTokens(cmd, cmdTokens);
    do {
      printf("  count> ");
      count = readChar() - '0';
    } while (count <= 0 || count > 9);
    do {
      printf("  [p]arallel or [s]equential> ");
      parallelInput = readChar();
    } while (parallelInput != 'p' && parallelInput != 's');
    parallel = (parallelInput == 'p') ? TRUE : FALSE;
    do {
      printf("  timeout> ");
      timeout = readChar() - '0';
    } while (timeout < 0 || timeout > 9);
    // end parsing code
    
	//	Separated the parallel and sequential executions into functions
	if (parallel) {
		executeInParallel(count, cmdTokens);
	} else {
		executeInSequence(count, cmdTokens);
	}
  }
}

