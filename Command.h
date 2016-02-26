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

struct Command* allocate();
struct Command* getCommand();

// Verify input/output redirection file permissions are correct.
int checkOutput(struct Command *cmd);
int checkInput(struct Command *cmd);

// Methods to create processes.
int createForeProcess(struct Command *cmd);
int createBackProcess(struct Command *cmd);
