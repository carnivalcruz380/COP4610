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
bool path_search(tokenlist *input);
void exec_command(char *input, tokenlist *args, bool in_redirection, bool out_redirection, int inposition, int outposition);
bool in_redirect(const char *file, char *input, tokenlist *args);
bool out_redirect(const char *file, char *input, tokenlist *args);

int main()
{

    while (1) {
        printf("%s@%s:%s>", getenv("USER"), getenv("MACHINE"), getenv("PWD"));

        /* input contains the whole command
		 * tokens contains substrings from input split by spaces
		 */

        char *input = get_input();
        char *echo_resp;

        tokenlist *tokens = get_tokens(input);
        for (int i = 0; i < tokens->size; i++) {

        }
		// block for echo calls
    if (strcmp(tokens->items[0],"echo") == 0){
			// block for tilde expansion
            if(strchr(tokens->items[1], '~') != NULL){
                tilde_expand(tokens->items[1]);
            }
            else
                echo(tokens->items[1]);
    }
		// block for path_search
        //else{
    if (path_search(tokens) == true){
          if(strchr(tokens->items[0], '~') != NULL)
                tilde_expand(tokens->items[0]);
          }
          else{
				           printf("It was not found\n");
          }

        //}


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
    char *answer;//[strlen(home) + strlen(input)];// local home variable
    int index;
    int x = strlen(home);

    if(strchr(input,'~') != NULL){
		// retreiving position of tilde
        position  = strchr(input,'~');
        index = (int)(position - input);
		// copying enviornmental into local home variable
        for( int i = 0; i < x; i++){
            answer[i] = home[i];
        }
		// removing tilde
        for( int i = index + 1; i < strlen(input); i++)
        {
            //answer[variable] = input[i]
            answer[x] = input[i];
            x += 1;
        }
    }
    printf(answer);
}


bool path_search(tokenlist *input)
{
	// file redirection variables
	bool check_inredirect = false;
	bool check_outredirect = false;
	int inpos = -1;
	int outpos = -1;
	for (int i = 0; i < input->size; i++){
		if (strcmp(input->items[i], ">") == 0){
			check_outredirect = true;
			outpos = i;
		}
		if (strcmp(input->items[i], "<") == 0){
			check_inredirect = true;
			inpos = i;
		}

	}

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

	// directory search loop
    while(token != NULL && check != 0)
    {
		// creating full command file path variable for each directory
        strcpy(filepath, token);
        strcat(filepath, command);

		// test statements

		// checking if file exists in the directory
        check = access(filepath,F_OK);

		// if file exists, execute it
        if(check == 0){
			exec_command(filepath, input, check_inredirect, check_outredirect, inpos, outpos);	// pass the file and args to be executed
        }

		char *temp2 = strtok(NULL, delim);	// temporary variable to store the next directory to be searched
        token = (char *) realloc(token , sizeof(char) * strlen(temp2));	// reallocating space for the new directory string
		strcpy(token, temp2);
    }

	// memory deallocation
    free(command);
    free(token);
	free(filepath);
    if (check == 0)
        return true;
    else
        return false;
}


void exec_command(char *input, tokenlist *args, bool in_redirection, bool out_redirection, int inposition, int outposition){
    pid_t pid, wait;	// pid variables
    int status;
	char *infile;	// input redirection file variable
	char *outfile;	// output redirection file variable
	int index;	// position of the redirection variable
	tokenlist *arglist = new_tokenlist();	// new token list to parse out redirection

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
  				close(0);
	                      	dup(in);
                        	close(in);

                        	execv(input, arglist->items);
                	}
                	else
                        	wait = waitpid(pid, &status, 0);

			//start of output redirect
			index = outposition;
			arglist = new_tokenlist();
			for (int i = 0; i < index; i++){
                                add_token(arglist, args->items[i]);
                        }
			pid = fork();
                	if(pid == 0){
                        	int out = open(outfile, O_CREAT | O_TRUNC | O_RDWR);
				close(1);
                        	dup(out);
                        	close(out);

                        	execv(input,arglist->items);
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

			execv(input, arglist->items);
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
			int out = open(outfile, O_RDWR | O_CREAT | O_TRUNC | O_RDWR);
			close(1);
			dup(out);
			close(out);

			execv(input,arglist->items);
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
			execv(input, args->items);
		}
		// parent process
		else {
			// waiting for command execution to return
			wait = waitpid(pid, &status, 0);
		}
	}

	free_tokens(arglist);
}


bool in_redirect(const char *file, char *input, tokenlist *args){
	printf("Got to input redirection\n");
	int in = open(file, O_RDONLY);
	if (in == -1){
		return false;
	}
	else {
		close(0);
		dup(in);

		return true;
	}

}

bool out_redirect(const char *file, char *input, tokenlist *args){
	char *command[2];	// command variable
	command[0] = args->items[0];
	command[1] = NULL;

	int out = open(file, O_RDWR | O_CREAT | O_TRUNC);	// output file to replace stdout
	close(1);
	dup(out);
	close(out);

	int status;	// variable to hold the status of the process
	pid_t pid, wait;	// pid variables
	/*pid = fork();
	if (pid == 0){
		execv()
	}
	else {
		wait = waitpid(pid, &status, 0);
	}*/
	printf("Got to output redirection\n");
	return true;
}
