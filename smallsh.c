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

// Custom libraries
#include "Command.h"

// Global var to check if kill signal delivered.
int static killDelivered = 0;




/**************************************************
** Function: kill_handler
** Description: This function prints out the signal that terminated the process.
** Parameters: int signum, the signal number emitted.
** Returns: Nothing
**************************************************/
void kill_handler(int signum) {
  killDelivered = signum;
  return;
}


/**************************************************
** Function: promptUser
** Description: This function prints out ":" to prompt users to enter input.
** Parameters: None
** Returns: None
**************************************************/
void promptUser(){
  signal(SIGKILL, kill_handler);
  // signal(SIGILL, kill_handler);
  printf(": ");
  fflush(stdout);
}


/**************************************************
** Function: exitCommand
** Description: This function quits the current terminal session.
** Parameters: None
** Returns: None
**************************************************/
void exitCommand() {
  kill(0, SIGKILL);
}


/**************************************************
** Function: statusCommand
** Description: This is a built in shell function to print out the exit value
**  of the last foreground process.
** Parameters: int status, status of the last command.
** Returns: None
**************************************************/
void statusCommand(int status) {
  printf("exit value %d\n", status);                      // Print out the previous commands status value.
  fflush(stdout);
}


/**************************************************
** Function: cdCommand
** Description: This is a built in shell function to change the current working directory,
**  to that specified by path. If path is blank, changes directory to that specified by
**  by the HOME path variable in the users environment.
** Parameters: char *path, path the user would like to change directory to.
** Returns: None
**************************************************/
void cdCommand(char *path) {
  if((int)strlen(path) > 0) {       // If user specified a path.
    chdir(path);
  } else {
    chdir(getenv("HOME"));          // User didn't specify path so go to where the home env variable points.
  }
}


/**************************************************
** Function: executeCommand
** Description: This method checks to see what command the user entered and runs it.
**  Based on the foreground/background process specified it manages the process.
** Parameters: Command *cmd, commandt the user wants to execute.
**  int prevCmdStatus, status returned by the previous command
**  int *backStatus, pointer to an integer to store the background process return value.
**  int *foreStatus, pointer to an integer to store the foreground process return value.
** Returns: None
**************************************************/
int executeCommand(struct Command *cmd, int prevCmdStatus, int* backStatus, int* foreStatus) {

  if(strcmp("exit", cmd->cmd) == 0) {                 // Built in command exit.
    exitCommand();

  } else if(strcmp("cd", cmd->cmd) == 0) {            // Built in command cd.
    cdCommand(cmd->args[0]);                          // First arguement will be path.

  } else if(strcmp("status", cmd->cmd) == 0) {        // Built in command status.
    statusCommand(prevCmdStatus);                     // Pass status pointer to status command method.

  } else if(cmd->backgroundProcess == 1) {            // Background process.
    *backStatus = createBackProcess(cmd);
    return *backStatus;                               // Return how the process executed.

  } else if(cmd->backgroundProcess == 0){                                            // Foreground process.
    *foreStatus = createForeProcess(cmd);             // -1 means error
    // printf("for: %i", *foreStatus);
    // fflush(stdout);
    return *foreStatus;                               // Return how the process executed.

  }
  return 1;
}



// Main.
int main() {
  char buffer[MAX_CHARS];               // Buffer to hold user input.
  int commandStatus = 0;
  struct Command *userCmd;
  int i = 0;
  int foreStatus;
  int backStatus;

  signal(SIGINT, kill_handler);

  do {
    if(killDelivered > 0) {
      printf("terminated by signal %d\n", killDelivered);
      killDelivered = 0;
    }

    userCmd = getCommand(&backStatus);               // Get the user's entered command.

    if(checkInput(userCmd) == 0) {
      continue;                           // Input file was bad, reprompt.
    }

    if(checkOutput(userCmd) == 0) {
      continue;                           // Output file was bad, reprompt.
    }

    commandStatus = executeCommand(userCmd, commandStatus, &backStatus, &foreStatus); // Returns 1 if users entered 'exit', 0 otherwise.



  } while(strcmp("exit", userCmd->cmd) != 0);

};
