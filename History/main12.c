//GROUP 27

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define MAX_INPUT 514
#define delims " |><&;\t"

int tokenize(char input[], char *tokens[]);
int runExternal(char *args[]);

int main(){
	const char *exitString = "exit";
	char strIn[MAX_INPUT];
	char* inputPointer; //tests for ctrl-d

	/*	ACTUAL LOOP OF THE PROGRAM	*/
	while(1){
		printf("> ");
		inputPointer = fgets(strIn, MAX_INPUT, stdin);

		if ((strlen(strIn) == 513) && (strIn[strlen(strIn) -1] != '\n')){
			
			while((getchar()) != '\n');
		}

		strIn[strlen(strIn)-1] = '\0'; //removes the newline character from input string

		if (!strcmp(strIn, exitString) || !inputPointer){
			
			printf("Now quitting...\n");
			break;
		}

		char *args[51];
		
		if (tokenize(strIn, args)){
			
			runExternal(args); //run external command
		}
	}
	return 0;
}

/*
Takes in the input array and a pointer to the output array. The input array is then split into tokens,
each of which is stored as a seperate array where the output pointer points to. The function returns 1
if the tonization was succesful. If too many tokens were entered, the function returns 0 and clears
the pointer to the output array.
*/ 
int tokenize(char *input, char **tokens){
	int index = 0;
  	char *token;

  	token = strtok(input, delims); //gets the first token of the input
  	
  	while (token != NULL) {
  		
  		tokens[index] = token;
  		index++;

    		token = strtok(NULL, delims); //gets further tokens of the input
  	}
  	
  	tokens[index] = NULL; //NULL pointer required for execvp()

	if (index >= 51) {	//checking number of tokens
		
		printf("Too many arguments, can only take a maximum of 50\n");
		tokens = NULL;
		return 0;
	
	} else { //printing the tokens
		
		printf("\nTokens:\n");
		
		for (int i = 0; i < index; i++){
			printf("\"%s\"\n", tokens[i]);
		}
		
		printf("\n");
	}
	return 1;
}

/*
Takes an array of arguments used to run an external command in a child process. If a command is not run due
to the first argument not being a recognised external command then the user is notified and returned
to the main program
*/ 
int runExternal(char *args[]) {
	pid_t pid;
	pid = fork(); //setting the process id's (child gets 0, parent gets >0, <0 is failure)

	if (pid < 0) { //in case fork fails
	
		fprintf(stderr, "Fork Failed");
		exit(-1);
	
	} else if (pid == 0) { //child process
	
		if (execvp(args[0], args) == -1) { //attempt to run external command, returns -1 if not run
			printf("Invalid Command\n");
			exit(0);
		}
	
	} else { //return to parent
	
		wait(NULL);
	}
	
	return 0;
}
