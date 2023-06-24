#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


#define BYTESHELL_BUFFER_SIZE 64
#define BYTESHELL_LINE_SIZE 1024
#define BYTESHELL_DELIMIT " \t\r\n\a"


int BYTESHELL_exit(char **input_args);
int BYTESHELL_help(char **input_args);
int BYTESHELL_cd(char **input_args);
int BYTESHELL_history(char **input_args);
int BYTESHELL_bg(char **input_args);





char *builtins[] = { "exit", "help", "cd", "history", "bg" };





int (*builtin_functions[])(char **) = { &BYTESHELL_exit, &BYTESHELL_help, &BYTESHELL_cd, &BYTESHELL_history, &BYTESHELL_bg };


int BYTESHELL_builtins_num()
{
  return sizeof(builtins)/sizeof(char *);
}


// exit builtin
int BYTESHELL_exit(char **input_args)
{
  return 0;
}


// help builtin

int BYTESHELL_help(char **input_args)
{
  printf("BYTESHELL\n");
  printf("These shell commands are defined internally:\n");

  for (int i = 0; i < BYTESHELL_builtins_num(); i++)
  {
    printf("  %s\n", builtins[i]);
  }

  return 1;
}


//cd builtin

int BYTESHELL_cd(char **input_args)
{
  if (input_args[1] == NULL)
  {
    fprintf(stderr, "BYTESHELL: expected argument to \"cd\"\n");
  }
  else
  {
    if (chdir(input_args[1]) != 0)
    {
      perror("BYTESHELL");
    }
  }
  return 1;
}



//Linked list for history builtin

struct Node {
    char* str;
    struct Node* next;
};
struct Node* head = NULL;
struct Node* curr_ind = NULL;


//Utility functions for history builtin:

char* concatenate(char* str1, char* str2)
{
  int sz1 = strlen(str1);
  int sz2 = strlen(str2);
	char* temp_str = (char*)malloc(sizeof(char*)*(sz1+sz2));
  strcpy(temp_str, str1);
  strcat(temp_str, str2);
	return temp_str;
}

void append(char **input_args){
  if(head==NULL){
    head = (struct Node*)malloc(sizeof(struct Node));
    head->str = (char *)malloc(0x1000);
    char *str1 = " ";
    if (input_args[1] == NULL) 
      strcpy(head->str, concatenate(input_args[0], str1));
    else{  
      strcpy(head->str, concatenate(input_args[0], str1));
      strcpy(head->str, concatenate(head->str, input_args[1]));
    }
    head->next = NULL;
    curr_ind = head;
  }
  else{
    struct Node *temp_node = (struct Node *)malloc(sizeof(struct Node));
    curr_ind->next = temp_node;
    temp_node->str = (char *)malloc(0x1000);
    char *str1 = " ";
    if (input_args[1] == NULL) 
      strcpy(temp_node->str, concatenate(input_args[0],str1));
    else{  
      strcpy(temp_node->str, concatenate(input_args[0],str1));
      strcpy(temp_node->str, concatenate(temp_node->str, input_args[1]));
    }
    temp_node->next = NULL;
    curr_ind = temp_node;
  }
}


// history builtin

int BYTESHELL_history(char **args){  
   struct Node* temp_node = head;
    int i = 1;
    while (temp_node != NULL)
    {
      printf(" %d %s\n",i++,temp_node->str);
      temp_node = temp_node->next;
    }
  return 1; 
  }



// bg builtin


int BYTESHELL_bg(char **args)
{
  ++args;
  char *start = args[0];
  int childpid = fork();
  if (childpid >= 0)
  {
    if (childpid == 0)
    {
      if (execvp(start, args) < 0)
      {
        perror("Error on execvp\n");
        exit(0);
      }
    }
  }
  else
  {
    perror("Error while doing fork()");
  }
  return 1;
}


// BYTESHELL LAUNCHER


int BYTESHELL_launch(char **args)
{
  pid_t pid;
  int status;

  pid = fork();
  if (pid == 0)
  {
    // Child process
    if (execvp(args[0], args) == -1)
    {
      perror("ACMShell");
    }
    exit(EXIT_FAILURE);
  }
  else if (pid < 0)
  {
    // Error forking
    perror("ACMShell");
  }
  else
  {
    // Parent process
    do
    {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}


// EXECUTION

int BYTESHELL_execute(char **args)
{



  if (args[0] == NULL)
  {
    return 1;
  }

  // search for builtins
  for (int i = 0; i < BYTESHELL_builtins_num(); i++)
  {
    if (strcmp(args[0], builtins[i]) == 0)
    {
      return (*builtin_functions[i])(args);
    }
  }

  return BYTESHELL_launch(args);
}



// Reading INPUT


char *BYTESHELL_read_line()
{
  char *input = NULL;
  ssize_t buffsize = 0;

  if (getline(&input, &buffsize, stdin) == -1){
    if (feof(stdin)) {
      exit(EXIT_SUCCESS);  // We recieved an EOF
    } else  {
      perror("readline");
      exit(EXIT_FAILURE);
    }
  }

  return input;
}




// Parsing


char **BYTESHELL_split_line(char *input)
{
  int buffsize = BYTESHELL_BUFFER_SIZE;
  char **tokens_arr = malloc(buffsize * sizeof(char *));
  char *token, **tokens_backup;

  if (!tokens_arr)
  {
    fprintf(stderr, "BYTESHELL: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(input, BYTESHELL_DELIMIT);
  int ind=0;
  while (token != NULL)
  {
    tokens_arr[ind] = token;
    ind++;

    if (ind >= buffsize)
    {
      buffsize += BYTESHELL_BUFFER_SIZE;
      tokens_backup = tokens_arr;
      tokens_arr = realloc(tokens_arr, buffsize * sizeof(char *));
      if (!tokens_arr)
      {
        free(tokens_backup);
        fprintf(stderr, "BYTESHELL: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, BYTESHELL_DELIMIT);
  }
  tokens_arr[ind] = NULL;
  return tokens_arr;
}




void BYTESHELL_main()
{
  char *input;
  char **input_args;
  int status;

  do
  {
    printf(">> ");
    input = BYTESHELL_read_line();
    input_args = BYTESHELL_split_line(input);
    append(input_args);
    status = BYTESHELL_execute(input_args);



    free(input);
    free(input_args);

  } while(status);


}

int main()
{
  BYTESHELL_main();
  return EXIT_SUCCESS;
}
