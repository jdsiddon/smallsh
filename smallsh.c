#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_ARGS 512
#define MAX_CHARS 2048

struct Command {
  char cmd[MAX_ARGS];
  char args[MAX_ARGS][MAX_CHARS];               // Give me plenty of headroom for arguements.
  int argLen;
  char inFilename[MAX_ARGS];
  char outFilename[MAX_ARGS];
  int backgroundProcess;
};

/**************************************************
** Function: allocate
** Description: This function gets called to allocate enough space to hold the number rooms
**    that will be used to play.
** Parameters: none
** Returns: Pointer to a struct
**************************************************/
struct Command* allocate() {
  struct Command *command = (struct Command*) malloc (sizeof(struct Command));
  return command;
};


/**
: will be used to prompt for each command line (make sure you flush with fflush())
Syntax: command [arg1 arg2 ...] [< input_file] [> output_file] [&]
& denotes a process should be run in the background.

input/output redirection: > and < are used followed by a file name.
input redirection can appear before or after output redirection.

Supports maximum line of 2048 characters
Supports maximum of 512 arguements

fork, exec, waitpid will be used to exec commands.
- After fork, before exec you must do ay input/output redirection
- Input should only be opened for reading, if error opening to read, print error message and set exit to 1.
- Output should only be openend for writing, should be truncated if exists, created if doesn't
If can't open output should error message and set status to 1.

Shell will wait for completion of foreground commands (no &).

Shell will not wait for completion of background commands.
- Thier standard input will be redirected from /dev/null if user
didn't specify some other file.
- Starting a background process will print the pid.
- Completion of these processes will be checked before a the user
is prompted. If a process was complete it will will show the process id, and exit status
Use waitpid to check for completion of background processes.

It looks for PATH variables for commands.

Allows shell scripts to be executed.
- Failure of scripts prints and error message and sets exit status to 1.

CTRL-C will not terminate your shell, only foreground commands that it is running
- Use signal handlers to do this.
- background processes will not be terminated by this command either.

*/

/*
Supports 3 built in commands
- Doesn't need to support input/output redirection for built in commands.
- Do not need to set exit status.

exit - exits shell, takes no arguements.
- kills any other processes the shell started before terminating the shell


cd - changes directories
- With no arguement it changes directory to the dir specified in the HOME env variable
(not the location the shell was started)
- Can also take one arguement
- supports absolute and relative paths.


status - prints out the exit status or terminating signal of the last foreground process
*/

/**
FUNCTIONS



readCmd()
Breaks each command into a command struct. Command struct will be used through program to denote what the user wants to do.
struct command {
  array of arguements
  input filename
  output filename
  backgroundPID (yes/no)
}
specs
- command [arg1 arg2 ...] [< input_file] [> output_file] [&]
- square brackets are optional
- separated by spaces
- symbols <, >, and & are recognized, but they must be surrounded by spaces like other words
- to run process in background must end with &
- standard input or output is to be redirected, the > or < words followed by a filename word must appear after all the arguments.
- Input redirection can appear before or after output redirection.
- Your shell should support command lines with a maximum length of 2048 characters, and a maximum of 512 arguments. You do not need to do any error checking on the syntax of the command line.

Algorithm
- accepts cmdline arguements
- splits arguments on space characters
- places each word before a '<' or '>' into the array of arguements (same order there were entered)
- places word after '<' into input file variable
- places word after '>' into output file variable
- returns command struct





Your shell does not need to support any quoting; so arguments with spaces inside them are not possible.




*/


/**
MAIN PROGRAM Algorithm

start program
promptUser(); - Just outputs ":"
getCmd();
checkInput();
checkOutup();

if both successful continue
- continue
else
- reprompt

if backgroundPID
- createBackProcess();

else
- createForeProcess();
*/

/**
 * promptUser()
 * - print ":" to get command line arguments
 * - fflush() to clear output buffer
 */
 void promptUser(){
   printf(": ");
   fflush(stdout);
 };

 /****************************** ********************
 ** Function: getCommand
 ** Description: This function gets the users input, stores it into a command structure
 **  for use throughout the program.
 ** Parameters: none
 ** Returns: none
 **************************************************/
struct Command* getCommand() {
  char buffer[MAX_CHARS];               // Buffer to hold user input.
  char *arg;
  int cmdSize;
  int ioAssigned;                       // Flag to determine if i/o args have been passed, no other (non-i/o) args except '&' can follow.
  int validInput = 1;
  struct Command *cmd = allocate();     // Create command structure, get memory.
  int i = 0;

  // Keep prompting user for input until they provide a valid command.
  do {
    promptUser();
    fgets(buffer, MAX_CHARS, stdin);                               // Read in command.

    buffer[strcspn(buffer, "\n")] = 0;                             // Pull out newline character from user entered string.
    cmdSize = (int)strlen(buffer);

    // printf("Command Size: %d", cmdSize);

    if(cmdSize > MAX_CHARS) {           // Check command length.
      validInput = 0;

    } else if(cmdSize == 0) {           // User pressed enter.
      validInput = 0;

    } else {
      arg = strtok(buffer, " ");        // Get command, (1st argument user enters.)
      strcpy(cmd->cmd, arg);
      ioAssigned = 0;                   // Initialize i/o to 0;

      arg = strtok(NULL, " ");          // Get first arguement.

      // Loop through each argument.
      while(arg != NULL) {
        // Check if arguements is actually input/output.
        if(strcmp(arg, ">") == 0) {             // Redirect OUTPUT
          arg = strtok(NULL, " ");              // Get following string.

          strcpy(cmd->outFilename, arg);
          ioAssigned = 1;                       // i/o assigned!
          continue;

        } else if(strcmp(arg, "<") == 0) {      // Redirect INPUT
          arg = strtok(NULL, " ");              // Get following string.

          strcpy(cmd->inFilename, arg);
          ioAssigned = 1;                       // i/o assigned!
          continue;

        } else if(strcmp(arg, "&") != 0) {      // Only get arguments if i/o isn't assigned, and != &.
          strcpy(cmd->args[cmd->argLen], arg);      // Put arguement string in the command structure.
          cmd->argLen = cmd->argLen + 1;            // Increment number of arguments.

          if(cmd->argLen >= MAX_ARGS) {             // Check and make sure user didn't pass to many arguements.
            validInput = 0;                         // Input isn't valid.
            memset(cmd->args, 0x00, MAX_ARGS);      // Reset array to 0.
          }

        } else {                            // At the end of the arg string.
          if(strcmp(arg, "&") == 0) {
            cmd->backgroundProcess = 1;
          } else {
            cmd->backgroundProcess = 0;
          }
        }
        arg = strtok(NULL, " ");

      }

      // Debuggin command struct and args.
      // printf("Command: %s\n", cmd->cmd);
      // fflush(stdout);
      // for(i = 0; i < cmd->argLen; i++) {
      //   printf("Arg %d: %s\n", i, cmd->args[i]);
      //   fflush(stdout);
      // }
      // printf("Input: %s\n", cmd->inFilename);
      // fflush(stdout);
      // printf("Output: %s\n", cmd->outFilename);
      // fflush(stdout);
      // printf("Bg: %d\n", cmd->backgroundProcess);
      // fflush(stdout);

      validInput = 1;
    }

  } while(validInput == 0);

  return cmd;
};




//
// checkInput()
// Checks that input file has 'read' only flag set.
// Algorithm
// - accepts command struct
// - opens input file
// - checks if read only flag is on file
// - if success, return 1
// - else, return error
//
int checkInput(struct Command *cmd) {
  int inputValid = 1;

  if((int)strlen(cmd->inFilename) > 0) {      // Only check validity of input if one was provided.
    FILE *input;
    input = fopen(cmd->inFilename, "r");      // Open input file for reading.
    if(input == NULL) {                       // Error opening file!
      switch(errno) {
        case 2:
          printf("%s: no such file or directory\n", cmd->inFilename);
          fflush(stdout);
          break;
        case 13:
          printf("%s: cannot open file for input\n", cmd->inFilename);
          fflush(stdout);
          break;
      }
      inputValid = 0;

    } else {                                  // File open was successful.
      fclose(input);                          // Close file.

    }
  }

  return inputValid;                          // Will return invalid if the file returned errors when trying to open file.
}




// checkOutput()
// Checks that output file has 'write' only flag set.
// Algorithm
// - accepts command struct
// - opens input file
// - checks if write only flag is on file
// - if success, return 1
// - else, return error
int checkOutput(struct Command *cmd) {
  int outputValid = 1;

  if((int)strlen(cmd->outFilename) > 0) {      // Only check validity of output if one was provided.
    FILE *output;
    output = fopen(cmd->outFilename, "w");      // Open output file for reading.
    if(output == NULL) {                       // Error opening file!
      switch(errno) {
        case 2:
          printf("%s: no such file or directory", cmd->outFilename);
          fflush(stdout);
          break;
        case 13:
          printf("%s: cannot open file for output", cmd->outFilename);
          fflush(stdout);
          break;
      }
      outputValid = 0;

    } else {                                    // File open was successful.
      fclose(output);                           // Close file.

    }
  }

  return outputValid;                          // Will return invalid if the file returned errors when trying to open file.
}







// createForeProcess()
// Creates a foreground process and executes it.
// Algorithm
// - Fork process
// - Call waitpid on the child process, requiring the parent to wait until child returns.
// - commandBuiltIn();
// - Execute process
// - Process completes return to get command line statement
int createForeProcess(struct Command *cmd) {
  pid_t parent = getpid();          // Get parent pid.
  int i;
  pid_t pid = fork();               // Fork process

  if(pid == -1) {
    exit(1);                        // error

  } else if(pid > 0) {              // PARENT PROCESS
    int status;
    waitpid(pid, &status, 0);       // Waits for child process.
    return status;

  } else {                          // CHILD PROCESS
  //  printf("Front");
  //  fflush(stdout);
    int newArgLen = cmd->argLen+1;              // Add two to arg array, 1 to hold command, 1 to hold 'NULL'.
    char* argv[newArgLen];                      // Adding 1 so the arg array can have the command as the first element.

    // Create an array to pass to the exec command.
    argv[0] = cmd->cmd;                         // Set first array element to command line command.
    int j = 1;                                  // Set place of first command line arguement.

    // Put each command in the command line arguement array.
    for(i = 0; i < cmd->argLen; i++) {
      argv[j] = cmd->args[i];
      j++;
    }
    argv[newArgLen] = NULL;                     // Set last element to NULL as required by exec function family.

    if((strlen(cmd->inFilename) > 0) || (strlen(cmd->outFilename) > 0)) {       // User wants to redirect input/output.
      if(strlen(cmd->outFilename) > 0) {               // Redirect output.
        int output;

        output = open(cmd->outFilename, O_WRONLY | O_APPEND);      // Open input file for reading.
        dup2(output, 1);                           // Redirect stdin to the 'input' file.

      }
    }

    execvp(argv[0], argv);                  // Execute the passed command.
    _exit(0);

  }
}







// createBackProcess()
// Creates a background process and executes it.
// Algorithm
// - Fork Process
// - setup input redirection, use dup2
// - setup output redirection, use dup2
// - commandBuiltIn();
// - execute process
// - Print background process pid "background pid is XXXX"
// - Return command line control immediately
int createBackProcess(struct Command *cmd) {
  pid_t parent = getpid();          // Get parent pid.
  int i;
  pid_t pid = fork();               // Fork process

  if(pid == -1) {
    exit(1);                        // error

  } else if(pid > 0) {              // PARENT PROCESS
    int status;
    // Print out background PID info.
    printf("background pid is %d\n", pid);      // Since we are in the parent process, 'pid' is assigned the value of the childs pid.
    fflush(stdout);
    waitpid(pid, &status, WNOHANG);       // Waits for child process.
    return status;
    //return parent;

  } else {                          // CHILD PROCESS, pid = 0.
    int newArgLen = cmd->argLen+1;              // Add two to arg array, 1 to hold command, 1 to hold 'NULL'.
    char* argv[newArgLen];                      // Adding 1 so the arg array can have the command as the first element.

    // Create an array to pass to the exec command.
    argv[0] = cmd->cmd;                         // Set first array element to command line command.
    int j = 1;                                  // Set place of first command line arguement.

    // Put each command in the command line arguement array.
    for(i = 0; i < cmd->argLen; i++) {
      argv[j] = cmd->args[i];
      j++;
    }
    argv[newArgLen] = NULL;                     // Set last element to NULL as required by exec function family.

    int output;                               // Where the processes output will go.

    if(strlen(cmd->outFilename) > 0) {                           // Redirect output to the specified file, if one was provided.
      output = open(cmd->outFilename, O_WRONLY | O_APPEND);      // Open input file for reading.
    } else {                                                     // No file was provided so redirect to dev/null
      output = open("/dev/null", O_WRONLY);
    }
    dup2(output, 1);                                           // Redirect stdout to the 'output' or dev/null.


    int input;
    // If stdin not defined set to /dev/null
    input = open("/dev/null", O_RDONLY);
    dup2(input, 0);

    sleep(5);
    execvp(argv[0], argv);                  // Execute the passed command.
    _exit(0);

  }
}

// Built in command, stops all running processes with the same group id as calling process.
void exitCommand() {
  kill(0, SIGKILL);
}


// Built in command, prints the exit status or terminating signal of the last foreground process.
void statusCommand(int status) {
  printf("exit value %d\n", status);                      // Print out the previous commands status value.
  fflush(stdout);
}

// The cd command changes directories.  By itself, it changes to the directory specified
// in the HOME environment variable (not to the location where smallsh was executed
// from, unless your shell is located in the HOME directory).  It can also take one
// argument, the path of the directory to change to. Note that this is a working
// directory: when smallsh exits, the pwd will be the original pwd when smallsh was
// launched. Your cd command should support both absolute and relative paths.
void cdCommand(char *path) {
  if((int)strlen(path) > 0) {       // If user specified a path.
    chdir(path);
  } else {
    chdir(getenv("HOME"));          // User didn't specify path so go to where the home env variable points.
  }
}


int executeCommand(struct Command *cmd, int prevCmdStatus) {
  int foreStatus;
  int backStatus;


  pid_t childpid = waitpid(-1, &backStatus, WNOHANG);
  if(childpid > 0) {                                 // Let use know background pid completed.
    printf("background pid %d is done: exit value %d\n", childpid, backStatus);
    fflush(stdout);
  }

  if(strcmp("exit", cmd->cmd) == 0) {             // Built in command exit.
    exitCommand();

  } else if(strcmp("cd", cmd->cmd) == 0) {            // Built in command cd.
    cdCommand(cmd->args[0]);                          // First arguement will be path.

  } else if(strcmp("status", cmd->cmd) == 0) {        // Built in command status.
    statusCommand(prevCmdStatus);                     // Pass status pointer to status command method.

  } else if(cmd->backgroundProcess == 1) {            // Background process.
    backStatus = createBackProcess(cmd);

  } else {                                            // Foreground process.
    foreStatus = createForeProcess(cmd);              // -1 means error

  }
  return 1;

}


void sighandler(int signum) {
  printf("Caught err: %d\n", signum);
  fflush(stdout);

}

int main() {
  int commandStatus = 0;
  struct Command *userCmd;
  int i = 0;


  do {
    userCmd = getCommand();             // Get the user's entered command.

    if(checkInput(userCmd) == 0) {
      continue;                         // Input file was bad, reprompt.
    }

    if(checkOutput(userCmd) == 0) {
      continue;                         // Output file was bad, reprompt.
    }

    commandStatus = executeCommand(userCmd, commandStatus); // Returns 1 if users entered 'exit', 0 otherwise.

  } while(strcmp("exit", userCmd->cmd) != 0);

};
