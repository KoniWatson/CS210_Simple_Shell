//GROUP 27
#include "ace4.h"

/*
Wee little global for original path, for setpath
*/
const char* originalPath;

// History
HistoryList history[HISTORY_LENGTH]; //defines array of structs
int commandIndex = 0;

// Aliases
AliasList aliases[ALIASES_LENGTH];

FILE *fp;
//const char* filename = ".hist_list";

int main(){
	/* SETUP CODE */

	const char *exitString = "exit";
	originalPath = getenv("PATH"); //saving original path on start

	char strIn[MAX_INPUT + 2], currentWorkingDir[MAX_INPUT + 2]; //MADE cwd maxlength same as input
	char* inputPointer; //tests for ctrl-d
	char* home = getenv("HOME");//Getting home directory

	chdir(getenv("HOME")); //changing to home directory

	printf("HOME directory: %s\n", home); //testing the print for the home directory (stage 3 testing)

	int commandCount = loadHistory();
	loadAliases();

	/* END OF SETUP CODE */

	/*	ACTUAL LOOP OF THE PROGRAM	*/
	while(1){
		if (aliases[0].alias[5] == 127 && !strcmp(aliases[0].instruction, "exit")) {	//not sure what is causing the error that this fixes 
			printf("overflow happened so fixing\n");
			strcpy(aliases[0].alias, "");
			strcpy(aliases[0].instruction, "");
		}
		
		getcwd(currentWorkingDir, sizeof(currentWorkingDir)); //gets current directory
		printf("%s%s%s > ", GRN, currentWorkingDir, NORM); //adds current directory to prompt
		inputPointer = fgets(strIn, MAX_INPUT + 2, stdin);

		if (!inputPointer) {
			strcpy(strIn, "exit");
		}

		if ((strlen(strIn) == MAX_INPUT + 1) && (strIn[strlen(strIn) -1] != '\n')){
			printf("%sToo many characters.%s\n", RED, NORM);
			while((getchar()) != '\n');
			continue;
		}

		if (strIn[strlen(strIn)-1] == '\n') {
			strIn[strlen(strIn)-1] = '\0'; //removes the newline character from input string
		}
		
		if (strIn[0] == '\0') {
			continue;
		};

		char *args[MAX_TOKENS + 1];
		char tempIn[MAX_INPUT + 2];
		strcpy(tempIn, strIn);
		
		if (!tokenize(tempIn, args)) {
			continue;
		};
		
		
		int index = 0;
		int substitutions = 0;
		if (strcmp(args[0], "unalias") && strcmp(args[0], "alias")) {
			while(args[index] != NULL){

				int check;
				for (int i = 0; i < 3; i++) {							//only making up to 3 substitutions
					check = isAlias(args[index]);
					if (check >= 0) {
							strcpy(args[index], aliases[check].instruction);
							substitutions++;
					}
				}
				index++;
			}
		}
		if(substitutions == 3){
			printf("%sSubstitution limit reached.%s\n", RED, NORM);
			//continue; //if we end up needing to change it to an error which does not execute
		}

		//check if it is histroy command invocation
		if(strcspn(strIn, "!") == 0){

			if(strIn[1] == '!'){
				
				strcpy(strIn, getHistoryCommand(commandCount - 1));
			}else if(strIn[1] == '-'){

				memset(strIn, '0', 2);
				strcpy(strIn, getHistoryCommand(commandCount - atoi(strIn)));
				
			}else{

				strIn[0] = '0';		
				strcpy(strIn, getHistoryCommand(atoi(strIn)));
			}
			//if yes then execute command from history
		} else {

			strcpy(history[commandIndex].instruction, strIn);
			history[commandIndex].commandNumber = commandCount;
			printf("Command #%d added at position %d\n", commandCount, commandIndex);

			commandCount++;
			commandIndex = (commandIndex + 1) % HISTORY_LENGTH;
		}


		if (!strcmp(strIn, exitString) || !inputPointer){			
			printf("Now quitting...\n");

			chdir(getenv("HOME"));
			saveAliases();
			saveHistory();

			setenv("PATH", originalPath, 1); //making sure to restore original PATH
			break;
		}


		if (!runOwnCommands(args)) { //check and do if command recognised as one of ours
			runExternal(args); //run external command
		}
	}
	return 0;
}

char *getHistoryCommand(int index){
	int commandCount = history[(commandIndex + 19) % 20].commandNumber;
	if(index <= 0){

		printf("ERROR HISTORY - NOT NUMBER or TOO SMALL\n");

	}else if (index >= commandCount){

		printf("ERROR HISTORY - NUMBER TOO LARGE\n");

	}else if(index < (commandCount - HISTORY_LENGTH)){

		printf("ERROR HISTORY - COMMAND NO LONGER SAVED\n");

	}else{

		return history[findByCommandNumber(index)].instruction;
	}
	return "";
}

int isAlias(char* name){

  char temp[MAX_INPUT];
	for (int i = 0; i < ALIASES_LENGTH; i++){

    strcpy(temp, aliases[i].alias);

		if (!strcmp(name, temp)){

			return i;
		}
	}

	return -1;
}


int findByCommandNumber(int number){
	for (int i = 0; i < HISTORY_LENGTH; i++){
		if(number == history[i].commandNumber) return i;
	}

	return -1;
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

	if (index > MAX_TOKENS) {	//checking number of tokens

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


int runPipedExternal(char *args[], int pipeLength){
	int des_p[2];
	if(pipe(des_p) == -1) {
		perror("Pipe failed");
		exit(1);
	}

	int pipeCheck = 0;
	int hitNull = 0;
	for (int i = 0; i < MAX_TOKENS + 1; i++)
	{
		if(args[i] == NULL){
			char **p = &(args[hitNull]);
			hitNull = i + 1;

			if(fork() == 0)
			{
				if(pipeCheck != 0){

					close(STDIN_FILENO);
					dup(des_p[0]);
					close(des_p[1]); 
					close(des_p[0]);
				} 

				if(pipeCheck != pipeLength - 1){

					close(STDOUT_FILENO);
					dup(des_p[1]);
					close(des_p[0]); 
					close(des_p[1]);
				}

				if (execvp(p[0], p) == -1) { //attempt to run external command, returns -1 if not run
					printf("%s%s: command not found\n", RED, args[0]);
					exit(0);
				}
			}	
			pipeCheck++;
		}		

		if(pipeCheck == pipeLength){
			break;
		}
	}

	close(des_p[0]);
	close(des_p[1]);

	for (int i = 0; i < pipeLength; i++)
	{
		wait(0);
	}

	return 0;
}

/*
Takes an array of arguments used to run an external command in a child process. If a command is not run due
to the first argument not being a recognised external command then the user is notified and returned
to the main program
*/
int runExternal(char *args[]) {

	int pipeLength = 0;
	for (int i = 0; i < MAX_TOKENS + 1; i++)
	{
		if(args[i] != NULL){
			if(!strcmp(args[i], "|")){
				if(args[i + 1] != NULL){
					pipeLength++;
				}
				args[i] = NULL;
			}
		} else {
			break;
		}
	}
	if(pipeLength) {
		return runPipedExternal(args, pipeLength + 1);
	}

	pid_t pid;
	pid = fork(); //setting the process id's (child gets 0, parent gets >0, <0 is failure)

	if (pid < 0) { //in case fork fails

		fprintf(stderr, "Fork Failed");
		exit(-1);

	} else if (pid == 0) { //child process

		if (execvp(args[0], args) == -1) { //attempt to run external command, returns -1 if not run
			printf("%s%s: command not found\n", RED, args[0]);
			exit(0);
		}

	} else { //return to parent

		wait(NULL);
	}

	return 0;
}

/*
Run our commands
*/
int runOwnCommands(char *args[]) {				//should change to switch statement if we make these into single characters later

	if (!strcmp(args[0] , "getpath")){
			getpath();
			return 1;
	}

	else if (!strcmp(args[0], "setpath")){
			setpath(args);
			return 1;
	}
	
	else if (!strcmp(args[0], "cd")) {
		changeWorkingDirectory(args);
		return 1;
	}

	else if (!strcmp(args[0], "history")){
			printHistory();
			return 1;
	}

    else if (!strcmp(args[0], "alias")){
            addAlias(args);
            return 1;
    }

    else if (!strcmp(args[0], "unalias")) {
            removeAlias(args);
            return 1;
    }

	return 0;
}

/*
Part 3 code commands getpath - prints system path, and setpath - takes in a path and makes it the new value of path
*/

int getpath() {
	printf("%s%s\n",BLU, getenv("PATH"));

	return 0;
}

int setpath(char *args[]){
	if (args[1] == NULL){ //checking for to few arguments
		printf("%sToo few arguments\n", RED);
	}

	else if (args[1] != NULL && args[2] !=  NULL){ //Checking for to many values
		printf("%sToo many arguments\n", RED);
	}

	else if (!strcmp(args[1], "origin")) { //resetting path to original path name
		setenv("PATH", originalPath, 1);
	}

	else {
		char* pathName = args[1];
		setenv("PATH", pathName, 1);
	}

	return 0;
}

/*
Change working directory
*/
void changeWorkingDirectory(char *args[]) {
	if (args[1] != NULL && args[2] != NULL) { //check no of arguments
		printf("%sToo many arguments\n", RED);
	}
	else if (args[1] == NULL) {
		chdir(getenv("HOME")); //go to home directory if no further argument
	}
	else {
		if (chdir(args[1]) == -1) {
			switch(errno) {
				case EACCES :
					printf("%sPermission denied\n", RED);
					break;
				case ELOOP :
					printf("%sToo many levels of symbolic links\n", RED);
					break;
				case ENAMETOOLONG :
					printf("%sFile name too long\n", RED);
					break;
				case ENOENT :
					printf("%sNo such file or directory\n", RED);
					break;
				case ENOTDIR :
					printf("%sNot a directory\n", RED);
					break;
			}
		}
	}
}

void printHistory(){
	for (int i = 0; i < HISTORY_LENGTH; i++){
		int count = history[(commandIndex + i) % HISTORY_LENGTH].commandNumber;
		if(!count) continue;
		printf("%2d %s\n", count, history[(commandIndex + i) % HISTORY_LENGTH].instruction);
	}
}

int loadHistory(){
	fp = fopen(".hist_list", "r");

	if(fp == NULL){
		printf("No histroy list was found, no histroy was loaded.\n");
		return 1;
	}

	int commandCount = 1;
	for (int i = 0; i < HISTORY_LENGTH; i++){

		int num;
		char instruction[MAX_INPUT + 2];

		fscanf(fp, "%d %[^\n] ", &num, instruction);
		history[i].commandNumber = num;
		strcpy(history[i].instruction, instruction);
		commandCount = num + 1;
		commandIndex = (commandIndex + 1) % HISTORY_LENGTH;

		if(feof(fp)) break;
	}

	fclose(fp);
	return commandCount;
}

void saveHistory(){
	fp = fopen(".hist_list", "w");

	if(fp == NULL){
		printf("ERROR creating file! [.hist_list]\n");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < HISTORY_LENGTH; i++){
		int count = history[(commandIndex + i) % HISTORY_LENGTH].commandNumber;
		if(!count) continue;
		fprintf(fp, "%2d %s\n", count, history[(commandIndex + i) % HISTORY_LENGTH].instruction);
	}

	fclose(fp);
	return;
}

int addAlias(char *args[]) {
    int state = 0; //when state = 1 then successfully added alias, when state = -1 overwritten alias, when state = 0 then error occured.

	if (args[1] == NULL) {
	    printAliases();
	} else if ((args[2] == NULL)){ //checking for to few arguments
      
        printf("%sToo few arguments\n", RED);
    
    } else {

        char *name = args[1];
        char command[MAX_INPUT];
        strcpy(command, args[2]);

        int index = 3;
        while (args[index] != NULL) {
            strcat(command, " ");
            strcat(command, args[index]);
            index++;
        }

        index = isAlias(name);

        if (index == -1) {
            for (int i = 0; i < ALIASES_LENGTH; i++){

                if (!strcmp(aliases[i].alias, "")){
                    printf("Alias %s was added. \n", name);
                    strcpy(aliases[i].alias, name);
                    strcpy(aliases[i].instruction, command);
                    state = 1;
                    break;
                }else if(i == ALIASES_LENGTH-1){
                    printf("%sMaximum number of alias reached, alias was not added.\n%s", RED, NORM);
                }
            }
        }else {
            strcpy(aliases[index].instruction, command);
            printf("Alias %s was overwritten. \n", aliases[index].alias);
            state = -1;
        }
    }
    return state;
}

int removeAlias(char *args[]) {
    int state = 0; // when state = 1 then successfully removed alias, when state = -1 then alias does not exit, when state = 0 then error occured.
    int index = 0;

    if (args[1] == NULL){ //checking for to few arguments
        printf("%sToo few arguments\n", RED);
    }

    else if ((args[1] != NULL) && (args[2] !=  NULL)){ //Checking for to many values
        printf("%sToo many arguments\n", RED);
    }

    else {
        index = isAlias(args[1]);

        if (index != -1) {
            strcpy(aliases[index].alias, "");
            strcpy(aliases[index].instruction, "");
            state = 1;
        }else {
            printf("Alias %s does not exist so cannot be removed. \n", args[1]);
            state = -1;
        }
    }
    return state;
}

void printAliases(){
	for (int i = 0; i < ALIASES_LENGTH; i++) {
       	if (strcmp(aliases[i].alias, "")){
	        printf("%s\t%s\n", aliases[i].alias, aliases[i].instruction);
      	}
	}
}

void loadAliases(){
	fp = fopen(".aliases", "r");

	if(fp == NULL){
		printf("No alias list was found, no aliases were loaded.\n");
		return;
	}

	for (int i = 0; i < ALIASES_LENGTH; i++){

		char alias[MAX_INPUT + 1];
		char instruction[MAX_INPUT + 1];
		fscanf(fp, "%s %[^\n] ", alias, instruction);

        strcpy(aliases[i].alias, alias);
        strcpy(aliases[i].instruction, instruction);

		if(feof(fp)) break;
	}

	fclose(fp);
	return;
}

void saveAliases(){
	fp = fopen(".aliases", "w");

	if(fp == NULL){
		printf("ERROR creating file! [.aliases]\n");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < ALIASES_LENGTH; i++) {
       	if (strcmp(aliases[i].alias, "")){
	        fprintf(fp, "%s\t%s\n", aliases[i].alias, aliases[i].instruction);
			printf("added #%s#%s#\n", aliases[i].alias, aliases[i].instruction);
      	}
	}

	fclose(fp);
	return;
}