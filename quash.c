/**
 * @file quash.c
 *
 * Quash's main file
 */

/**************************************************************************
 * Included Files
 **************************************************************************/
#include "quash.h" // Putting this above the other includes allows us to ensure
                   // this file's headder's #include statements are self
                   // contained.

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>


/**************************************************************************
 * Private Variables
 **************************************************************************/
/**
 * Keep track of whether Quash should request another command or not.
 */
// NOTE: "static" causes the "running" variable to only be declared in this
// compilation unit (this file and all files that include it). This is similar
// to private in other languages.
static bool running;


/**************************************************************************
 * Private Functions
 **************************************************************************/
/**
 * Start the main loop by setting the running flag to true
 */
static void start() {
  running = true;
}

/**************************************************************************
 * Public Functions
 **************************************************************************/
bool is_running() {
  return running;
}

void terminate() {
  running = false;
}

bool get_command(command_t* cmd, FILE* in) {
  if (fgets(cmd->cmdstr, MAX_COMMAND_LENGTH, in) != NULL) {
    size_t len = strlen(cmd->cmdstr);
    char last_char = cmd->cmdstr[len - 1];

    if (last_char == '\n' || last_char == '\r') {
      // Remove trailing new line character.
      cmd->cmdstr[len - 1] = '\0';
      cmd->cmdlen = len - 1;
    }
    else
      cmd->cmdlen = len;

    return true;
  }
  else
    return false;
}

/**
 * Quash entry point
 *
 * @param argc argument count from the command line
 * @param argv argument vector from the command line
 * @return program exit status
 */

 void setPathOrHome(char* input){

   char* token1 = strtok(input, " ");
   token1 = strtok(NULL, " ");

   char* token2 = strtok(token1, "=");

   if(strcmp(token2, "PATH") == 0){
     token2 = strtok(NULL, " ");

     setenv("PATH", token2, 1);

     char* testPath = getenv("PATH");

     printf(testPath);
     printf("\n");
   }else if(strcmp(token2, "HOME") == 0){
     token2 = strtok(NULL, " ");

     setenv("HOME", token2, 1);

     char* testHome = getenv("HOME");

     printf(testHome);
     printf("\n");
   }else{
     printf("Error parsing set function. Please use proper format.");
   }




 }

 void printEcho(char* input){

   char* token = strtok(input, " ");
   token = strtok(NULL, " ");

   if(strcmp(token, "$PATH") == 0){
     printf(getenv("PATH"));
     printf("\n");
   }else if(strcmp(token, "$HOME") == 0){
     printf(getenv("HOME"));
     printf("\n");
   }else{
     printf(token);
     printf("\n");
   }

 }

 void execute(char* input, char** env){

   pid_t quashPipe;
   int pipeEnds[2];
   int status;

   char* token = strtok(input, " ");
   char** arguments = (char**)malloc(32*sizeof(char*));
   int x = 0;

   while(token != NULL){
     arguments[x] = (char*)malloc(50*sizeof(char));
     arguments[x] = token;
     token = strtok(NULL, " ");
     x++;
   }

   pipe(pipeEnds);

   quashPipe = fork();

   if(quashPipe < 0){
     printf("Error forking child.\n");
     return EXIT_FAILURE;

   }else if(quashPipe == 0){

     dup2(pipeEnds[1], STDOUT_FILENO);

     close(pipeEnds[0]);
     close(pipeEnds[1]);

     if(execvpe(arguments[0], arguments, env) < 0){
       printf("Error opening program.\n");
     }

   }

   close(pipeEnds[0]);
   close(pipeEnds[1]);

   if ((waitpid(quashPipe, &status, 0)) == -1) {
     fprintf(stderr, "Process encountered an error. ERROR%d", errno);
     return EXIT_FAILURE;
   }

   return EXIT_SUCCESS;


 }




int main(int argc, char *argv[], char *envp[]) {
  command_t cmd; //< Command holder argument

  start();

  puts("Welcome to Quash!");
  puts("Type \"exit\" or \"quit\" to quit");

  // Main execution loop
  /*while (is_running() && get_command(&cmd, stdin)) {
    // NOTE: I would not recommend keeping anything inside the body of
    // this while loop. It is just an example.

    // The commands should be parsed, then executed.
    if (!strcmp(cmd.cmdstr, "exit"))
      terminate(); // Exit Quash
    else
      puts(cmd.cmdstr); // Echo the input string
  }*/

  char inputString[1024];

  while(1){

    printf("$ ");
    gets(inputString);

    if(strcmp(inputString, "exit") == 0 || strcmp(inputString, "quit") == 0){
      printf("Goodbye.\n");
      return EXIT_SUCCESS;
    }else if(strncmp(inputString, "set", 3) == 0){

      char setter[1024];
      strcpy(setter, inputString);

      setPathOrHome(setter);

    }else if(strncmp(inputString, "echo", 4) == 0){
      printEcho(inputString);
    }else{
      execute(inputString, envp);
    }

  }

  return EXIT_SUCCESS;
}
