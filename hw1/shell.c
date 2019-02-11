#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include <fcntl.h>

#include "tokenizer.h"

/* Convenience macro to silence compiler warnings about unused function parameters. */
#define unused __attribute__((unused))

/* Whether the shell is connected to an actual terminal or not. */
bool shell_is_interactive;

/* File descriptor for the shell input */
int shell_terminal;

/* Terminal mode settings for the shell */
struct termios shell_tmodes;

/* Process group id for the shell */
pid_t shell_pgid;

int cmd_exit(struct tokens *tokens);
int cmd_help(struct tokens *tokens);
int cmd_pwd(struct tokens *tokens);
int cmd_cd(struct tokens *tokens);

/* Built-in command functions take token array (see parse.h) and return int */
typedef int cmd_fun_t(struct tokens *tokens);

/* Built-in command struct and lookup table */
typedef struct fun_desc {
  cmd_fun_t *fun;
  char *cmd;
  char *doc;
} fun_desc_t;

fun_desc_t cmd_table[] = {
  {cmd_help, "?", "show this help menu"},
  {cmd_exit, "exit", "exit the command shell"},
  {cmd_pwd, "pwd", "print out the current working directory to std out"},
  {cmd_cd, "cd", "change directory to according to stdin"}
};

/* Prints a helpful description for the given command */
int cmd_help(unused struct tokens *tokens) {
  for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
    printf("%s - %s\n", cmd_table[i].cmd, cmd_table[i].doc);
  return 1;
}

/* Exits this shell */
int cmd_exit(unused struct tokens *tokens) {
  exit(0);
}

/* I added this function, print the current working directory to standard output*/
int cmd_pwd(unused struct tokens *tokens){
   char cwd[4096];
   if (getcwd(cwd, sizeof(cwd))) {
       printf("%s\n", cwd);
   } else {
       perror("getcwd() error");
       return -1;
   }
   return 0;
}

/* I added this function, change the current directory according to stdinput*/
int cmd_cd(unused struct tokens *tokens){
  chdir(tokens_get_token(tokens, 1));
  return 1;


}



/* Looks up the built-in command, if it exists. */
int lookup(char cmd[]) {
  for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
    if (cmd && (strcmp(cmd_table[i].cmd, cmd) == 0))
      return i;
  return -1;
}

/* Intialization procedures for this shell */
void init_shell() {
  /* Our shell is connected to standard input. */
  shell_terminal = STDIN_FILENO;

  /* Check if we are running interactively */
  shell_is_interactive = isatty(shell_terminal);

  if (shell_is_interactive) {
    /* If the shell is not currently in the foreground, we must pause the shell until it becomes a
     * foreground process. We use SIGTTIN to pause the shell. When the shell gets moved to the
     * foreground, we'll receive a SIGCONT. */
    while (tcgetpgrp(shell_terminal) != (shell_pgid = getpgrp()))
      kill(-shell_pgid, SIGTTIN);

    /* Saves the shell's process id */
    shell_pgid = getpid();

    /* Take control of the terminal */
    tcsetpgrp(shell_terminal, shell_pgid);

    /* Save the current termios to a variable, so it can be restored later. */
    tcgetattr(shell_terminal, &shell_tmodes);
  }
}

int main(unused int argc, unused char *argv[]) {
  init_shell();

  static char line[4096];
  int line_num = 0;

  /* Please only print shell prompts when standard input is not a tty */
  if (shell_is_interactive)
    fprintf(stdout, "%d: ", line_num);

  while (fgets(line, 4096, stdin)) {
    /* Split our line into words. */
    struct tokens *tokens = tokenize(line);

    /* Find which built-in function to run. */
    int fundex = lookup(tokens_get_token(tokens, 0));

    if (fundex >= 0) {
      cmd_table[fundex].fun(tokens);
    } else {

      int length = tokens_get_length(tokens);
      char *target[length+1];
      char *filename;
      char *carrot = NULL;
      
      int is_background = 0;
      
      
      for(int i=0; i<length;i++ ){
      target[i] = tokens_get_token(tokens, i);
        if (strcmp(target[i], ">") == 0){
          carrot = ">";
        }
        if (strcmp(target[i], "<") == 0){
          carrot = "<";
        }
      }
      
      target[length] = NULL;

      if(carrot != NULL){
        filename = target[length-1];
        target[length-1] = NULL;
        target[length-2] = NULL;
      }

      
      if (target[length-1] != NULL){
        if(strcmp(target[length-1], "&") == 0){
          is_background = 1;
          target[length-1] = NULL;
        }
      }
      //_________Debugging Use__________
      /*
      for (int i = 0; i< length+1; i++){
        printf("token %d, %s\n", i, target[i]);
      }
      */

      //__________Debugging Use___________

      // check the first token if / exists there.
      // If / is there, the first argument is a path
      if (strchr(tokens_get_token(tokens, 0), '/')){
        
        //fprintf(stderr, "Got argument as a path\n");
        

        if (carrot != NULL){
          if( strcmp(carrot, ">") == 0){
            int fd = open(filename, O_CREAT | O_TRUNC| O_WRONLY, 0666);
            dup2(fd, 1);
            close(fd);
            }


          if( strcmp(carrot, "<") == 0){
            target[length-2] = filename;
            }  
        }


        pid_t pid = fork();
        if (pid == 0){

          pid_t pgrp = getpgrp();
          if(is_background == 0){
            tcsetpgrp(0, pgrp);
          }



          if (execv(target[0], target) < 0) { 
            fprintf(stderr, "Command doesn't exist"); 
          }
        }else if (pid >0){


          signal(SIGINT,SIG_IGN);

          wait(NULL);
        }
      }else{ // this means the first token is not a path, so loof for it in PATH

        if (carrot != NULL){
          if( strcmp(carrot, ">") == 0){
            int fd = open(filename, O_CREAT | O_TRUNC| O_WRONLY, 0666);
            dup2(fd, 1);
            close(fd);
            } 


          if( strcmp(carrot, "<") == 0){
            target[length-2] = filename;
            }  

        }

        char *path = getenv("PATH");
        //printf("the PATH ENV is : \n ");
        //printf("%s \n", path);
        //printf("**********Here break point 1******\n");
        char *token;
        token = strtok(path, ":");
        //printf("**********Here break point 2******\n");
        //printf("%s", token);
        //printf("the get first token is: \n");
        //printf("%s \n", tokens_get_token(tokens, 0));
        while (token != NULL){
          //printf("token currently is ");
          //printf("%s \n", token);
          //char *pa = strcat(token, "/");
          //pa = strcat(token, tokens_get_token(tokens, 0));
          //printf("pa current is: ");
          //printf("%s \n", pa);
          //if ( access(path, F_OK) == 0){ 
          // concactenate token(which is the path) and the first token, which is the executable.
          char *fin_path = (char *) malloc(strlen(token)+ 2 + strlen(tokens_get_token(tokens, 0)));
          strcpy(fin_path, token);
          strcat(fin_path, "/");
          strcat(fin_path, tokens_get_token(tokens, 0));
          if (access(fin_path, F_OK) == 0){ 
            //printf("the final path finally is: %s", fin_path);
            pid_t pid = fork();
            if (pid == 0){

              pid_t pgrp = getpgrp();
              if(is_background == 0){
                tcsetpgrp(0, pgrp);
              }




              if (execv(fin_path, target) < 0) {
                //fprintf(stderr,"Got argument as an executable. execv failed");
              }

            }else if (pid >0){

              signal(SIGINT,SIG_IGN);


              wait(NULL);
            }
          }
          token = strtok(NULL, ":");        
        }
      //fprintf(stderr, "executable not found ");
      
      } 
     }
      

      /* REPLACE this to run commands as programs. */



    if (shell_is_interactive)
      /* Please only print shell prompts when standard input is not a tty */
      fprintf(stdout, "%d: ", ++line_num);

    /* Clean up memory */
    tokens_destroy(tokens);
  }

  return 0;

}