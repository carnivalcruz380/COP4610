#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

typedef struct {
    int size;
    char **items;
} tokenlist;

char *get_input(void);
tokenlist *get_tokens(char *input);
tokenlist *new_tokenlist(void);
void add_token(tokenlist *tokens, char *item);
void free_tokens(tokenlist *tokens);

void echo(char *input);
void tilde_expand(char *input);
char *path_search(tokenlist *input);
void exec_command(tokenlist *args, bool in_redirection, bool out_redirection, int inposition, int outposition);
void pipeline(int pipepos1, int pipepos2, bool check_pipe2, tokenlist *args);

int main()
{

    while (1) {
        printf("%s@%s:%s>", getenv("USER"), getenv("MACHINE"), getenv("PWD"));

        /* input contains the whole command
		 * tokens contains substrings from input split by spaces
		 */

        char *input = get_input();
        char *echo_resp;
		bool check_tilde = false;
		bool check_echo = false;
		bool check_inredirect = false;
		bool check_outredirect = false;
		bool check_pipe1 = false;
		bool check_pipe2 = false;
		int tildepos = -1;
		int echopos = -1;
		int pipepos1 = -1;
		int pipepos2 = -1;
		int inpos = -1;
		int outpos = -1;
	

        tokenlist *tokens = get_tokens(input);
        for (int i = 0; i < tokens->size; i++) {
			//printf("token %d: (%s)\n", i, tokens->items[i]);
        }
		
		// file redirection variables 
	
	
		for (int i = 0; i < tokens->size; i++){
			// block for echo calls
			/*
			if(strcmp(tokens->items[0],"echo") == 0){
				check_echo = true;
				echopos = i;
			}*/
			
			// block for tilde expansion
			if(strchr(tokens->items[i], '~') != NULL){
                check_tilde = true;
				tildepos = i;
            }
        
			// block for output redirection
			if (strcmp(tokens->items[i], ">") == 0){
				check_outredirect = true;
				outpos = i;
			} 
			// block for input redirection
			if (strcmp(tokens->items[i], "<") == 0){
				check_inredirect = true;
				inpos = i;
			}
			
			// block for piping 
			if (strcmp(tokens->items[i], "|") == 0){
				if(check_pipe1){
					check_pipe2 = true;
					pipepos2 = i;
				}
				else{
					check_pipe1 = true;
					pipepos1 = i;
				}
			}
		}
		
		// block for tilde expansion
		if (check_tilde){
			printf("going to tilde\n");
			tilde_expand(tokens->items[tildepos]);
		}
		else if (check_echo){
			printf("going to echo\n");
			echo(tokens->items[echopos + 1]);
		}
		else if (!check_pipe1)
        {
			printf("about to execute command\n");
            exec_command(tokens, check_inredirect, check_outredirect, inpos, outpos);
			printf("finishing executing\n");
        }
		else{
			//printf("goind to pipline\n");
			pipeline(pipepos1, pipepos2, check_pipe2, tokens);
		}



        free(input);
        free_tokens(tokens);
    }

    return 0;
}



tokenlist *new_tokenlist(void)
{
    tokenlist *tokens = (tokenlist *) malloc(sizeof(tokenlist));
    tokens->size = 0;
    tokens->items = (char **) malloc(sizeof(char *));
    tokens->items[0] = NULL; /* make NULL terminated */
    return tokens;
}

void add_token(tokenlist *tokens, char *item)
{
    int i = tokens->size;

    tokens->items = (char **) realloc(tokens->items, (i + 2) * sizeof(char *));
    tokens->items[i] = (char *) malloc(strlen(item) + 1);
    tokens->items[i + 1] = NULL;
    strcpy(tokens->items[i], item);

    tokens->size += 1;
}

char *get_input(void)
{
    char *buffer = NULL;
    int bufsize = 0;

    char line[5];
    while (fgets(line, 5, stdin) != NULL) {
        int addby = 0;
        char *newln = strchr(line, '\n');
        if (newln != NULL)
            addby = newln - line;
        else
            addby = 5 - 1;

        buffer = (char *) realloc(buffer, bufsize + addby);
        memcpy(&buffer[bufsize], line, addby);
        bufsize += addby;

        if (newln != NULL)
            break;
    }

    buffer = (char *) realloc(buffer, bufsize + 1);
    buffer[bufsize] = 0;

    return buffer;
}

tokenlist *get_tokens(char *input)
{
    char *buf = (char *) malloc(strlen(input) + 1);
    strcpy(buf, input);

    tokenlist *tokens = new_tokenlist();

    char *tok = strtok(buf, " ");
    while (tok != NULL) {
        add_token(tokens, tok);
        tok = strtok(NULL, " ");
    }

    free(buf);
    return tokens;
}

void free_tokens(tokenlist *tokens)
{
    for (int i = 0; i < tokens->size; i++)
        free(tokens->items[i]);
    free(tokens->items);
    free(tokens);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void echo(char* input){
    char* answer = input;
    char* home;
    int index;
    int counter = 0;
    const char ch = '$';

    if(strchr(input,ch) == NULL){
        printf("%s \n", input);
    }
    else{
        while(counter < strlen(answer))
        {
            answer[counter] = answer[counter+1];
            counter += 1;
        }
        answer = getenv(input);
        if(answer == NULL)
            printf("Error \n");
        else
            printf("%s \n", answer);
    }
}

void tilde_expand(char *input){
   char *home = getenv("HOME");	// enviornmental home variable 
    char *position;	// position of tilde variable
    char answer[strlen(home) + strlen(input)];// local home variable 	
    int index;
    int x = strlen(home);

    if(strchr(input,'~') != NULL)
		echo(home);
}


char* path_search(tokenlist *input)
{
	// static variable initialization
    char slash[2] = "/";	// slash for command path variable 
	const char delim[2] = ":";	// delimeter for path string variable 
    const char *path = getenv("PATH");	// enviornmental path variable 
	int check = 1;		// error checking variable that indicates whether the file was found or not 
	char copy[strlen(path)];	// local path variable
    strcpy(copy, path);		
	char *temp = strtok(copy, delim);	// temp variable to store first directory path

	// dynamic memory allocation
    char *command = (char *) calloc(strlen(slash) + strlen(input->items[0]), sizeof(char));	// full command arguement variable
	if(command == NULL){
		return false;
	}
	else{
	}
    strcpy(command,slash);
    strcat(command,input->items[0]);

    char *token = (char *) calloc(strlen(temp), sizeof(char));	// token variable to store each command file path
	strcpy(token, temp);
	if (token == NULL){
		return false;
	}
	else{
	}
    
    char *filepath = (char *) calloc(strlen(command) + strlen(token), sizeof(char));	// full command file path variable
	if (filepath == NULL){
		return false;
	}
	else{
	}

    char *file = (char*) calloc(strlen(command) + strlen(token), sizeof(char));

	// directory search loop
    while(token != NULL && check != 0)
    {
		// creating full command file path variable for each directory
        strcpy(filepath, token);
        strcat(filepath, command);

		// checking if file exists in the directory
        check = access(filepath,F_OK);
		if (check == 0){
			strcpy(file, filepath);
			//printf("the file is: %s\n", file);
			free(command);
			free(token);
			free(filepath);
			return file;
		}
		char *temp2 = strtok(NULL, delim);	// temporary variable to store the next directory to be searched
        token = (char *) realloc(token , sizeof(char) * strlen(temp2));	// reallocating space for the new directory string 
		strcpy(token, temp2);
    }
	
	// memory deallocation
	free(command);
	free(token);
	free(filepath);
	return NULL;
}


void exec_command(tokenlist *args, bool in_redirection, bool out_redirection, int inposition, int outposition){
	printf("Made it to exec_command\n");
    pid_t pid, wait;	// pid variables 
    int status;
	char *infile;	// input redirection file variable 
	char *outfile;	// output redirection file variable 
	int index;	// position of the redirection variable 
	tokenlist *arglist = new_tokenlist();	// new token list to parse out redirection

    //printf(path_search(args));
    printf("before filepath\n");
	//char filepath[strlen(path_search(args))];
    printf("calling path search\n");
	//strcpy(filepath, path_search(args));
	printf("the filepath is: %s\n", path_search(args));
	// if blocks to parse the input and remove the redirection statements 
	if (in_redirection && out_redirection){
		// retrieving the input and output files 
		infile = args->items[inposition + 1];
		outfile = args->items[outposition +1];
		printf("both output & input: infile - %s: outfile %s \n",infile, outfile);
		// if block for if the input redirection comes first
		if (inposition < outposition){
			index = inposition;
			// loop to add tokens before redirection
			//start of input redirect
			for (int i = 0; i < index; i++){
				add_token(arglist, args->items[i]);
			}
			pid = fork();
               		if(pid == 0){
                        	int in = open(infile, O_RDONLY);
                            int out = open(outfile, O_RDWR | O_CREAT | O_TRUNC, 0666);
  				            close(0);
	                      	dup(in);
                        	close(in);

                            close(1);
                            dup(out);
                            close(out);

                        	execv(path_search(args), arglist->items);
                	}
                	else
                        	wait = waitpid(pid, &status, 0);

		}
		// block for if the output redirection comes first 
		else {
			index = outposition;
			// loop to add tokens before redirection
			for (int i = 0; i < index; i++){
				add_token(arglist, args->items[i]);
			}
		}
	}
	// block for if there is only input redirection
	else if (in_redirection == true){
		index = inposition;
		infile = args->items[inposition + 1];
		for (int i = 0; i < index; i++)
			add_token(arglist, args->items[i]);	
		printf("only input, %s \n",infile);

		pid = fork();
		if(pid == 0){
			int in = open(infile, O_RDONLY);
			close(0);
			dup(in);
			close(in);

			execv(path_search(args), arglist->items);
		}
		else
			wait = waitpid(pid, &status, 0);

	}
	// block for if there is only output redirection 
	else if (out_redirection == true){
		index = outposition;
		outfile = args->items[outposition+1];
		printf("only output, %s \n", outfile);
		for (int i = 0; i < index; i++)
			add_token(arglist, args->items[i]);
		pid = fork();
		if(pid == 0){
			int out = open(outfile, O_RDWR | O_CREAT | O_TRUNC, 0666);
			close(1);
			dup(out);
			close(out);

			execv(path_search(args), arglist->items);
		}
		else
			wait = waitpid(pid, &status, 0);

	}
	// block if there is no redirection statements 
	else {
		infile = NULL;
		outfile = NULL;
	}
	
	// block for executing commands that have redirection
	if (arglist->items[0] != NULL){
		
	}
	// block for executing commands with no redirection
	else {
		// creating new process
		pid = fork();
		// child process
		if (pid == 0){
			// executing command
			execv(path_search(args), args->items);
		}
		// parent process
		else {
			// waiting for command execution to return 
			wait = waitpid(pid, &status, 0);
		}
	}
	
	free_tokens(arglist);
}

void pipeline(int pipepos1, int pipepos2, bool check_pipe2, tokenlist *args){
	//printf("Made it to pipline\n");
    //int fd[2];
	int fd[4];
    pid_t pid1;	
	pid_t pid2;
    pid_t pid3;
	pid_t wait1, wait2, wait3;
	int status1, status2, status3;
	tokenlist *arglist1 = new_tokenlist();		// parsed arguement list variable
	tokenlist *arglist2 = new_tokenlist();		// parsed arguement list variable
	tokenlist *arglist3 = new_tokenlist();		// parsed arguement list variable
	char *path1, *path2, *path3;
	if (!check_pipe2){
		path3 = NULL;
	}
	
	
	// loops for parsing arguement list from input 
	for (int i = 0; i < pipepos1; i++){
		add_token(arglist1, args->items[i]);
	}
	//printf("arg 1 of arglist1: %s\n", arglist1->items[0]);
	//printf("params of arlist1: %s\n", arglist1->items[1]);
	
	// block for two pipes 
	if(check_pipe2){
		for (int i = pipepos1 + 1; i < pipepos2; i++ ){
			add_token(arglist2, args->items[i]);
		}
		for (int i = pipepos2 + 1; i < args->size; i++){
			add_token(arglist3, args->items[i]);
		}
		//printf("got second token\n");
		pipe(fd);
		pipe(fd + 2);
	}
	// block for a single pipe
	else{
		for (int i = pipepos1 + 1; i < args->size; i++){
			add_token(arglist2, args->items[i]);
		}
		pipe(fd);
	}
	
	//two pipes
	if (check_pipe2){
		path1 = path_search(arglist1);
		//printf("path1: %s\n", path1);
		pid1 = fork();	// stout redirection 
		// block for stdout redirection
		if(pid1 == 0){
			//printf("in the first child\n");
			//dup2(fd[1], 1);
			//close(fd[0]);		// close file not using first 
			//close(fd[1]);
			dup2(fd[1], 1);
			close(fd[0]);
			close(fd[1]);
			close(fd[2]);
			close(fd[3]);
			execv(path1, arglist1->items);
		}
		else {
			path2 = path_search(arglist2);
			pid2 = fork();
			if (pid2 == 0){
				//printf("In the second child\n");
				//dup2(fd[0], 0);
				//dup2(fd[1], 1);
				//close(fd[1]);
				//close(fd[0]);
				dup2(fd[0], 0);
				dup2(fd[3], 1);
				close(fd[0]);
				close(fd[1]);
				close(fd[2]);
				close(fd[3]);
				execv(path2, arglist2->items);
			}
			else{
				path3 = path_search(arglist3);
				//printf("path3: %s\n", path3);
				pid3 = fork();
				if (pid3 == 0){
					//printf("In the third child\n");
					//dup2(fd[0], 0);
					//close(fd[1]);
					//close(fd[0]);
					dup2(fd[2], 0);
					close(fd[0]);
					close(fd[1]);
					close(fd[2]);
					close(fd[3]);
					execv(path3, arglist3->items);
				}
			}
		}
		
		close(fd[0]);
		close(fd[1]);
		close(fd[2]);
		close(fd[3]);
		free_tokens(arglist1);
		free_tokens(arglist2);
		free_tokens(arglist3);
		
		wait1 = waitpid(pid1, &status1, 0);
		wait2 = waitpid(pid2, &status2, 0);
		wait3 = waitpid(pid3, &status3, 0);
	}
	//one pipe
	else{
		path1 = path_search(arglist1);
		//printf("path1: %s\n", path1);
		pid1 = fork();	// stout redirection 
		// block for stdout redirection
		if(pid1 == 0){
			dup2(fd[1], 1);
			close(fd[0]);
			close(fd[1]);
			execv(path1, arglist1->items);
		}
		path2 = path_search(arglist2);
		pid2 = fork();
		if (pid2 == 0){
			//printf("In the second child\n");
			dup2(fd[0], 0);
			close(fd[0]);
			close(fd[1]);
			execv(path2, arglist2->items);
		}
		
		close(fd[0]);
		close(fd[1]);
		free_tokens(arglist1);
		free_tokens(arglist2);
		wait1 = waitpid(pid1, &status1, 0);
		wait2 = waitpid(pid2, &status2, 0);
	}

}