#include <stdio.h>
#include "Command.h"





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
}



/****************************** ********************
** Function: getCommand
** Description: This function gets the users input, stores it into a command structure
**  for use throughout the program.
** Parameters: none
** Returns: a command struct for reference throughout.
**************************************************/
struct Command* getCommand(int *backStatus) {
 char buffer[MAX_CHARS];               // Buffer to hold user input.
 char *arg;
 int cmdSize;
 int ioAssigned;                       // Flag to determine if i/o args have been passed, no other (non-i/o) args except '&' can follow.
 int validInput = 1;
 struct Command *cmd = allocate();     // Create command structure, get memory.
 int i = 0;

 // Keep prompting user for input until they provide a valid command.
 do {
   memset(buffer, 0, MAX_CHARS * sizeof(char));                   // Make sure input buffer is clean.

   // Check on background processes.
   pid_t childpid = waitpid(-1, backStatus, WNOHANG);
   if(childpid > 0) {                     // Let use know background pid completed.
     //printf("background pid %d is done: %s %d\n", childpid, backStatus == 0 ?"exit value" : "terminated by signal", backStatus);
     printf("background pid %d is done: ", childpid);
     fflush(stdout);
     printf(*backStatus == 0 ? "exit value %d\n" : "terminated by signal %d\n", *backStatus);
     fflush(stdout);

     break;
   }

   promptUser();
   fgets(buffer, MAX_CHARS, stdin);                               // Read in command.



   buffer[strcspn(buffer, "\n")] = 0;                             // Pull out newline character from user entered string.
   cmdSize = (int)strlen(buffer);

   // printf("Command Size: %d", cmdSize);

   if(cmdSize > MAX_CHARS) {           // Check command length.
     validInput = 0;

   } else if(cmdSize == 0) {           // User pressed enter.
     validInput = 0;

   } else if(buffer[0] == 35) {       // User entered a comment line.
     validInput = 0;                   // Comments are valid input, but they aren't a valid command so set flag to false.

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

     validInput = 1;
   }

 } while(validInput == 0);

 return cmd;
}


/**************************************************
** Function: checkInput
** Description: This function checks that the input file supplied has the
**   'read' only flag set.
** Parameters: command struct
** Returns: int, 1 if file has read flag set only, 0 if not.
**************************************************/
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


/**************************************************
** Function: checkOutput
** Description: This function checks that the output file supplied has the
**   'write' only flag set.
** Parameters: command struct
** Returns: int, 1 if file has wrtie flag set only, 0 if not.
**************************************************/
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




/**************************************************
** Function: createForeProcess
** Description: Creates a command string from the command structure
**  then executes it as a foreground process.
** Parameters: command struct
** Returns: int, how the process exited
**************************************************/
int createForeProcess(struct Command *cmd) {
  pid_t parent = getpid();          // Get parent pid.
  int i;
  pid_t pid = fork();               // Fork process

  if(pid == -1) {
    exit(1);                        // error

  } else if(pid > 0) {              // PARENT PROCESS
    int status;
    waitpid(pid, &status, 0);       // Waits for child process.

    if( WIFEXITED(status) == 1) {
      int val = WEXITSTATUS(status);
      if(val > 0) {
        return 1;
      }
      return 0;
    }
    return status;

  } else {                          // CHILD PROCESS
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

    if(strlen(cmd->outFilename) > 0) {               // Redirect output.
      int output;

      output = open(cmd->outFilename, O_WRONLY | O_APPEND);      // Open input file for reading.
      dup2(output, 1);                           // Redirect stdin to the 'input' file.

    }

    int execReturn = execvp(argv[0], argv);                  // Execute the passed command.

    if(execReturn == -1) {

      perror(argv[0]);
      _exit(-1);
    }
    _exit(0);



  }
}



/**************************************************
** Function: createBackProcess
** Description: Creates a command string from the command structure
**  then executes it as a background process.
** Parameters: command struct
** Returns: int, how the process exited
**************************************************/
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
    argv[newArgLen] = NULL;                   // Set last element to NULL as required by exec function family.

    int output;                               // Where the processes output will go.

    if(strlen(cmd->outFilename) > 0) {                           // Redirect output to the specified file, if one was provided.
      output = open(cmd->outFilename, O_WRONLY | O_APPEND);      // Open input file for reading.
    } else {                                                     // No file was provided so redirect to dev/null
      output = open("/dev/null", O_WRONLY);
    }
    dup2(output, 1);                                             // Redirect stdout to the 'output' or dev/null.


    int input;
    input = open("/dev/null", O_RDONLY);                         // If stdin not defined set to /dev/null
    dup2(input, 0);

    int execReturn = execvp(argv[0], argv);                  // Execute the passed command.

    if(execReturn == -1) {
      _exit(-1);
    }
    _exit(0);

  }
}
