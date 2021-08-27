#define _GNU_SOURCE //not sure why this is needed for setenv()
//added for colours to some of the printing, values taken from stackoverflow
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#define MAX_INPUT 512
#define MAX_TOKENS 50
#define HISTORY_LENGTH 20
#define ALIASES_LENGTH 10
#define delims " ><&;\t"
#define GRN "\x1B[32m"
#define NORM "\x1B[0m"
#define BLU "\x1B[36m"
#define RED "\x1B[31m"

int runPipedExternal(char *args[], int pipeLength);

char *getHistoryCommand(int index);
int tokenize(char input[], char *tokens[]);
int runOwnCommands(char *args[]);
int runExternal(char *args[]);
void changeWorkingDirectory(char *args[]);
int getpath();
int setpath(char *arg[]);
void printHistory();
int loadHistory();
void saveHistory();
int findByCommandNumber(int number);
int isAliasToken(char *name);
int isAlias(char* name);
int addAlias(char *args[]);
int removeAlias(char *args[]);
void printAliases();
void loadAliases();
void saveAliases();

typedef struct {
	int commandNumber;
	char instruction[MAX_INPUT + 2];
} HistoryList;

typedef struct {
	char alias[MAX_INPUT + 2];
	char instruction[MAX_INPUT + 2];
} AliasList;
