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
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

#define NAME_MAX 100


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
pid_t* pid;
char** commandArray;
int z;

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

   int pipeFlag = 0;
   int bgFlag = 0;
   int inFlag = 0;
   int outFlag = 0;

   char* token = strtok(input, " ");
   char** arguments = (char**)malloc(32*sizeof(char*));
   char** arguments2 = (char**)malloc(32*sizeof(char*));

   int x = 0;
   while(token != NULL){

     arguments[x] = (char*)malloc(50*sizeof(char));
     arguments[x] = token;

     if(strcmp(arguments[x], "&") == 0){
       bgFlag = 1;
       arguments[x] = NULL;
       break;
       x--;
     }else if(strcmp(arguments[x], "|") == 0){
       pipeFlag = 1;

       arguments[x] = NULL;
       token = strtok(NULL, " ");

       int y = 0;

       while(token != NULL){
         arguments2[y] = (char*)malloc(50*sizeof(char));
         arguments2[y] = token;
         token = strtok(NULL, " ");
         y++;
       }
       x--;
       break;
     }else if(strcmp(arguments[x], ">") == 0){
       inFlag = 1;
       x--;
     }else if(strcmp(arguments[x], "<") == 0){
       outFlag = 1;
       x--;
     }

     token = strtok(NULL, " ");
     x++;
   }

   if(bgFlag == 1){

     pid_t quashProcess;
     int status;

     quashProcess = fork();



     if(quashProcess > 0){

       commandArray[z] = (char*)malloc(50*sizeof(char));
       strcpy(commandArray[z], arguments[0]);
       printf("Running %s in background.\n", commandArray[z]);
       pid[z] = quashProcess;
       z++;

      bgFlag = 0;


    }else if(quashProcess < 0){

       printf("Error forking child.\n");
       return EXIT_FAILURE;


     }else{

       pid_t pid2;

       pid2 = fork();

       if(pid2 == 0){
         if(execvpe(arguments[0], arguments, env) < 0){
           printf("Error opening program.\n");
         }
       }else{
         waitpid( pid2, &status, 0);
       }


       bgFlag = 0;

       printf("[%d] has ended\n", getpid());
       kill(getpid(), -1);
       exit(0);

       return EXIT_SUCCESS;

     }



   }else if(pipeFlag == 1){


     pid_t quashPipe1;
     pid_t quashPipe2;
     int pipeEnds1[2];
     int pipeEnds2[2];
     int status;

     pipe(pipeEnds1);
     pipe(pipeEnds2);

     quashPipe1 = fork();

     if(quashPipe1 < 0){
       printf("Error forking first child.\n");
       return EXIT_FAILURE;

     }else if(quashPipe1 == 0){

       dup2(pipeEnds1[1], STDOUT_FILENO);

       close(pipeEnds1[0]);
       close(pipeEnds1[1]);
       close(pipeEnds2[0]);
       close(pipeEnds2[1]);

       if(execvpe(arguments[0], arguments, env) < 0){
         printf("Error opening program.\n");
       }

       exit(0);

     }


     quashPipe2 = fork();


     if(quashPipe2 < 0){
       printf("Error forking second child.\n");
       return EXIT_FAILURE;

     }else if(quashPipe2 == 0){

       dup2(pipeEnds2[0], STDOUT_FILENO);

       close(pipeEnds1[0]);
       close(pipeEnds1[1]);
       close(pipeEnds2[0]);
       close(pipeEnds2[1]);



       if(execvpe(arguments2[0], arguments2, env) < 0){
         printf("Error opening program.\n");
       }

       exit(0);

     }

     close(pipeEnds1[0]);
     close(pipeEnds1[1]);
     close(pipeEnds2[0]);
     close(pipeEnds2[1]);

     if ((waitpid(quashPipe1, &status, 0)) == -1) {
       fprintf(stderr, "Process 1 encountered an error. ERROR%d", errno);
       return EXIT_FAILURE;
     }

     if ((waitpid(quashPipe2, &status, 0)) == -1) {
       fprintf(stderr, "Process 2 encountered an error. ERROR%d", errno);
       return EXIT_FAILURE;
     }



   }else if(inFlag == 1){

   }else if(outFlag == 1){

   }else{


     pid_t quashPipe;
     int pipeEnds[2];
     int status;

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


   }


   return EXIT_SUCCESS;


 }

 static void call_chdir(char* dir)
{
  char buf[NAME_MAX];

  if(dir == getenv("HOME")){
    strcpy(buf, dir);
  }else{
    strcpy(buf, "./");
    strcat(buf, dir);
  }
  if(chdir(buf) == -1){
    fprintf(stderr, "error: could not change to dir %s\n", buf);
    return;
  }
  printf("-->change the current working directory to %s\n", buf);
}


static void call_getcwd()
{
  char* cwd;
  cwd = getcwd(0, 0);
  if(!cwd){
    fprintf(stderr, "getcwd failed: %s\n", strerror(errno));
  }else{
    printf("%s\n", cwd);
    free(cwd);
  }
}

void printJobs(){

  printf("Jobs: \n");

  int i;
  for(i = 0; i < z; i++){
    if(kill(pid[i], 0) == 0){
      printf("[%d] - %d - %s\n", i, pid[i], commandArray[i]);
    }
  }

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

  pid = (int *)malloc(30 * sizeof(int));
  commandArray = (char**)malloc(32*sizeof(char*));
  z = 0;



  char* inputString;

  while(1){

    printf("$ ");

    get_command(&cmd, stdin);

    inputString = cmd.cmdstr;


    if(strcmp(inputString, "exit") == 0 || strcmp(inputString, "quit") == 0){
      printf("Goodbye.\n");
      return EXIT_SUCCESS;
    }else if(strncmp(inputString, "set", 3) == 0){

      char setter[1024];
      strcpy(setter, inputString);

      setPathOrHome(inputString);

    }else if(strncmp(inputString, "echo", 4) == 0){
      printEcho(inputString);
    }else if(strcmp(inputString, "pwd") == 0){
      call_getcwd();
    }else if(strncmp(inputString, "cd", 2) == 0){

      char* token = strtok(inputString, " ");
      token = strtok(NULL, " ");


      if(token == NULL){
        call_chdir(getenv("HOME"));
      }else{
        call_chdir(token);
      }

    }else if(strcmp(inputString, "jobs") == 0){
      printJobs();
    }else{
      execute(inputString, envp);
    }

    //free(inputString);

  }

  return EXIT_SUCCESS;
}
