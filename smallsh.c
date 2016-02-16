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

promptUser()
- print ":" to get command line arguments
- fflush() to clear output buffer

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

checkInput()
Checks that input file has 'read' only flag set.
Algorithm
- accepts command struct
- opens input file
- checks if read only flag is on file
- if success, return 1
- else, return error

checkOutput()
Checks that output file has 'write' only flag set.
Algorithm
- accepts command struct
- opens input file
- checks if write only flag is on file
- if success, return 1
- else, return error

commandBuiltIn()
Checks to see if the passed command is a built in command or not (exit, cd, or status)
if it isn't it returns 0, if it is it returns 1.
Algorithm
- Get command from command struct
- check command
- if command exit, return 1
- if command cd, return 1
- if command status, return 1
- else return 0




Your shell does not need to support any quoting; so arguments with spaces inside them are not possible.


createForeProcess()
Creates a foreground process and executes it.
Algorithm
- Fork process
- Call waitpid on the child process, requiring the parent to wait until child returns.
- commandBuiltIn();
- Execute process
- Process completes return to get command line statement

createBackProcess()
Creates a background process and executes it.
Algorithm
- Fork Process
- setup input redirection, use dup2
- setup output redirection, use dup2
- commandBuiltIn();
- execute process
- Print background process pid "background pid is XXXX"
- Return command line control immediately


*/


/**
MAIN PROGRAM Algorithm

start program
promptUser(); - Just outputs ":"
readCmd();
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

int main() {
  
}
