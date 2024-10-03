#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <pwd.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/stat.h>

//+
// File:	shell.c
//
// Pupose:	This program implements a simple shell program. It does not start
//		processes at this point in time. However, it will change directory
//		and list the contents of the current directory.
//
//		The commands are:
//		   cd name -> change to directory name, print an error if the directory doesn't exist.
//		              If there is no parameter, then change to the home directory.
//		   ls -> list the entries in the current directory.
//			      If no arguments, then ignores entries starting with .
//			      If -a then all entries
//		   pwd -> print the current directory.
//		   exit -> exit the shell (default exit value 0)
//				any argument must be numeric and is the exit value
//
//		if the command is not recognized an error is printed.
//-

#define CMD_BUFFSIZE 1024
#define MAXARGS 10

int splitCommandLine(char * commandBuffer, char* args[], int maxargs);
int doInternalCommand(char * args[], int nargs);
int doExternalCommand(char * args[], int nargs);

//+
// Function:	main
//
// Purpose:	The main function. Contains the read
//		eval print loop for the shell.
//
// Parameters:	(none)
//
// Returns:	integer (exit status of shell)
//-

int main() {

    char commandBuffer[CMD_BUFFSIZE];
    // note the plus one, allows for an extra null
    char *args[MAXARGS+1];

    // print prompt.. fflush is needed because
    // stdout is line buffered, and won't
    // write to terminal until newline
    printf("%%> ");
    fflush(stdout);

    while(fgets(commandBuffer,CMD_BUFFSIZE,stdin) != NULL){
        //printf("%s",commandBuffer);

	// remove newline at end of buffer
	int cmdLen = strlen(commandBuffer);
	if (commandBuffer[cmdLen-1] == '\n'){
	    commandBuffer[cmdLen-1] = '\0';
	    cmdLen--;
            //printf("<%s>\n",commandBuffer);
	}

	// split command line into words.(Step 2)
    
	int nargs = splitCommandLine(commandBuffer, args, MAXARGS);

    // add a null to end of array (Step 2)
    args[nargs] = NULL;

	// debugging
	// printf("%d\n", nargs);
	// int i;
	// for (i = 0; i < nargs; i++){
	//   printf("%d: %s\n",i,args[i]);
	// }

	// element just past nargs
	// printf("%d: %p\n",i, args[i]);

    if (nargs != 0) {
    if (doInternalCommand(args, nargs) == 0) {
        if (doExternalCommand(args, nargs) == 0) {
            fprintf(stderr, "%s: command not found\n", args[0]);
        }
    }
}
    
	// print prompt
	printf("%%> ");
	fflush(stdout);
    }
    return 0;
}

////////////////////////////// String Handling (Step 1) ///////////////////////////////////

//+
// Function:	skipChar
//
// Purpose:	This function skips over a given char in a string
//		For security, will not skip null chars.
//
// Parameters:
//    charPt Pointer to string
//    skip character to skip
//
// Returns:	Pointer to first character after skipped chars
//		ID function if the string doesn't start with skip,
//		or skip is the null character
//-

char * skipChar(char * charPtr, char skip){
    if (charPtr == NULL || skip == '\0') { // Checks if delimiter is null or if current character is null
        return charPtr;
    } else {
        while(*charPtr == skip) { // If current char is to be skipped
            charPtr++; // Increment the pointer address
        } 
        return charPtr; // Return new address
    }
}

//+
// Funtion:	splitCommandLine
//
// Purpose:	Splits the input command line string into individual arguments.
//
// Parameters:
//	commandBuffer The input command line string to be split into arguments
//	args[]  An array of character pointers where the split arguments will be stored
//	maxargs  The maximum number of arguments that can be stored in args[]
//
// Returns:	Number of arguments (< maxargs).
//
//-

int splitCommandLine(char *commandBuffer, char *args[], int maxargs) {
    int nargs = 0;  // Number of arguments found
    char *currentPos = commandBuffer;

    // Loop through the command buffer to find arguments
    while (*currentPos != '\0') {
        // Skip over any leading spaces
        currentPos = skipChar(currentPos, ' ');

        // If end of string is reached, break the loop
        if (*currentPos == '\0') {
            break;
        }

        // If number of arguments exceeds maxargs, print an error
        if (nargs >= maxargs) {
            fprintf(stderr, "Error: too many arguments (max %d).\n", maxargs);
            return 0;
        }

        // Store the pointer to the current argument
        args[nargs++] = currentPos;

        // Find the next space or end of the string
        char *nextSpace = strchr(currentPos, ' ');

        // If no space was found, we are at the last argument
        if (nextSpace == NULL) {
            break;
        }

        // Null-terminate the current argument
        *nextSpace = '\0';

        // Move to the next character after the space
        currentPos = nextSpace + 1;
    }
    return nargs;
}

////////////////////////////// External Program  (Note this is step 4, complete doeInternalCommand first!!) ///////////////////////////////////

// list of directorys to check for command.
// terminated by null value
char * path[] = {
    ".",
    "/bin",
    "/usr/bin",
    NULL
};

//+
// Function:	doExternalCommand
//
// Purpose:	Search for and execute external commands.
//
// Parameters:
//	args[]	- Array of arguments where args[0] is the command to be executed.
//	nargs	- Number of arguments in the args array.
//
// Returns	int
//		1 = Found and executed the file.
//		0 = Could not find or execute the file.
//-

int doExternalCommand(char *args[], int nargs) {
    struct stat statbuf;
    char cmd_path[CMD_BUFFSIZE];
    int i = 0;

    // Iterate over all directories in the path array
    while (path[i] != NULL) {
        // Construct the full path to the command
        snprintf(cmd_path, sizeof(cmd_path), "%s/%s", path[i], args[0]);

        // Check if the file exists and is executable
        if (stat(cmd_path, &statbuf) == 0 && S_ISREG(statbuf.st_mode) && (statbuf.st_mode & S_IXUSR)) {
            // If the file is found and executable fork a child process to execute it
            pid_t pid = fork();

            if (pid == 0) {
                // This is the child process execute the command
                execv(cmd_path, args);
                // If execv returns, there was an error
                perror("execv");
                exit(EXIT_FAILURE);
            } else if (pid > 0) {
                // Parent process: wait for the child to complete
                int status;
                wait(&status);
                return 1;  // Command executed successfully
            } else {
                // Fork failed
                perror("fork");
                return 0;
            }
        }

        // Move to the next directory in the path
        i++;
    }

    // If no executable file was found, return 0
    return 0;
}

////////////////////////////// Internal Command Handling (Step 3) ///////////////////////////////////

// define command handling function pointer type
typedef void(*commandFunction)(char * args[], int nargs);

// associate a command name with a command handling function
struct cmdData{
   char 	* cmdName;
   commandFunction 	cmdFunc;
};

// prototypes for command handling functions
void exitFunc();
void pwdFunc(char *args[], int nargs);
void cdFunc(char *args[], int nargs);
void lsFunc(char *args[], int nargs);

// list commands and functions
// must be terminated by {NULL, NULL} 
// in a real shell, this would be a hashtable.

struct cmdData commands[] = {
   {"exit", exitFunc},
   {"pwd", pwdFunc},
   {"cd", cdFunc},
   {"ls", lsFunc},
   { NULL, NULL}		// terminator
};

//+
// Function:	doInternalCommand
//
// Purpose:	Checks if the first argument is an internal command and executes it if found.
//
// Parameters:
//	args[]	- Array of arguments where args[0] is the command to check.
//	nargs	- Number of arguments in the args array.
//
// Returns	int
//		1 = args[0] is an internal command
//		0 = args[0] is not an internal command
//-

int doInternalCommand(char * args[], int nargs) {
    // Loop through the list of internal commands
    for (int i = 0; commands[i].cmdName != NULL; i++) {
        // Compare the input command (args[0]) with each internal command
        if (strcmp(args[0], commands[i].cmdName) == 0) {
            // Call the associated command function
            commands[i].cmdFunc(args, nargs);
            return 1;  // Return 1 to indicate that the command was found and executed
        }
    }

    // If no matching internal command was found, return 0
    return 0;
}
///////////////////////////////
// comand Handling Functions //
///////////////////////////////

//+
// Function:	exitFunc
//
// Purpose:	To exit with given status
//
// Parameters:
//	no parameters
//
// Returns	void
//		no return
//-

void exitFunc() {
    exit(0); 
}

//+
// Function:	pwdFunc
//
// Purpose:	To print the current working directory.
//
// Parameters:
//	args[]	- Array of arguments (not used here).
//	nargs	- Number of arguments (not used here).
//
// Returns	void
//-

void pwdFunc(char *args[], int nargs) {
    char *cwd = getcwd(NULL, 0);  // Get the current working directory
    if (cwd != NULL) {
        printf("%s\n", cwd);  // Print the working directory
        free(cwd);  // Free the memory allocated by getcwd
    } else {
        perror("pwd");  // Print error if getcwd fails
    }
}

//+
// Function:	cdFunc
//
// Purpose:	To change the current working directory.
//
// Parameters:
//	args[]	- Array of arguments where args[1] is the directory to change to.
//	nargs	- Number of arguments (1 or 2).
//
// Returns	void
//-

void cdFunc(char *args[], int nargs) {
    const char *targetDir;
    
    // If no argument, change to the home directory
    if (nargs == 1) {
        struct passwd *pw = getpwuid(getuid());  // Get the home directory of the current user
        if (pw != NULL) {
            targetDir = pw->pw_dir;  // Set the target directory to the user's home directory
        } else {
            fprintf(stderr, "cd: could not find home directory\n");
            return;
        }
    } else {
        targetDir = args[1];  // Set the target directory to the argument
    }

    // Try to change to the target directory
    if (chdir(targetDir) != 0) {
        perror("cd");  // Print error if chdir fails
    }
}

//+
// Function:	filter
//
// Purpose:	To filter out hidden files (those starting with a dot).
//
// Parameters:
//	d	- A pointer to a struct dirent.
//
// Returns	int
//		1 = if the file should be displayed (not hidden).
//		0 = if the file should not be displayed (hidden).
//-

int filter(const struct dirent *d) {
    return d->d_name[0] != '.';  // Return 0 (false) if the name starts with a dot, 1 (true) otherwise
}

//+
// Function:	lsFunc
//
// Purpose:	List the contents of the current directory.
//
// Parameters:
//	args[]	- Array of arguments where args[1] could be "-a" to show all files.
//	nargs	- Number of arguments (1 or 2).
//
// Returns	void
//-

void lsFunc(char *args[], int nargs) {
    struct dirent **namelist;
    int numEnts;

    if (nargs == 1) {
        // No arguments: List files, excluding hidden ones
        numEnts = scandir(".", &namelist, filter, NULL);  // No sorting
    } else if (strcmp(args[1], "-a") == 0) {
        // "-a" argument: List all files, including hidden ones
        numEnts = scandir(".", &namelist, NULL, NULL);  // No sorting, no filter
    } else {
        // Invalid argument
        fprintf(stderr, "ls: invalid option '%s'\n", args[1]);
        return;
    }

    if (numEnts < 0) {
        perror("ls");  // Print error if scandir fails
    } else {
        for (int i = 0; i < numEnts; i++) {
            printf("%s\n", namelist[i]->d_name);  // Print each file/directory name
            free(namelist[i]);  // Free memory for each dirent
        }
        free(namelist);  // Free the namelist array
    }
}