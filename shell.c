#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/types.h>

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
void exec_command(char *input, tokenlist *args);

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
        else{
            if (path_search(tokens) == true){
				printf("It was found\n");
            }
            else{
				printf("It was not found\n");
            }

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

void exec_command(char *input, tokenlist *args){
    pid_t pid, wait;	// pid variables 
    int status;
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

bool path_search(tokenlist *input)
{
	// static variable initialization
    char slash[2] = "/";	// slash for command path variable 
	const char delim[2] = ":";	// delimeter for path string variable 
    const char *path = getenv("PATH");	// enviornmental path variable 
	int check = 1;		// error checking variable that indicates whether the file was found or not 
	char copy[strlen(path)];	// local path variable
    strcpy(copy, path);		
	char *temp = strtok(copy, delim);	// temp variable to store first directory path
	printf("the temp variable is: %s\n", temp);
	
	// dynamic memory allocation
    char *command = (char *) calloc(strlen(slash) + strlen(input->items[0]), sizeof(char));	// full command arguement variable
	if(command == NULL){
		return false;
	}
	else{
		printf("Got the command\n");
	}
    strcpy(command,slash);
    strcat(command,input->items[0]);

    char *token = (char *) calloc(strlen(temp), sizeof(char));	// token variable to store each command file path
	strcpy(token, temp);
	if (token == NULL){
		return false;
	}
	else{
		printf("Got the token\n");
	}
    
    char *filepath = (char *) calloc(strlen(command) + strlen(token), sizeof(char));	// full command file path variable
	if (filepath == NULL){
		return false;
	}
	else{
		printf("Got the filepath\n");
	}

	// directory search loop
    while(token != NULL && check != 0)
    {
		// creating full command file path variable for each directory
        strcpy(filepath, token);
        strcat(filepath, command);
		
		// test statements 
        printf("here's the new x: %s\n", filepath);
        printf("here's the new token: %s\n", token);
        printf("here's command: %s\n", command);
		
		// checking if file exists in the directory
        check = access(filepath,F_OK);		
		
		// if file exists, execute it
        if(check == 0){
            exec_command(filepath, input);	// pass the file and args to be executed 
            printf("found the token\n");
        }
        
		char *temp2 = strtok(NULL, delim);	// temporary variable to store the next directory to be searched
        token = (char *) realloc(token , sizeof(char) * strlen(temp2));	// reallocating space for the new directory string 
		strcpy(token, temp2);
    }
	
	// memory deallocation
    free(command);
    printf("command was freed\n");
    free(token);
	printf("token was freed\n");
	free(filepath);
	printf("filepath was freed\n");
    if (check == 0)
        return true;
    else
        return false;
}

void tilde_expand(char *input){
    char *home = getenv("HOME");	// enviornmental home variable 
    char *position;	// position of tilde variable
    char answer[strlen(home) + strlen(input)];// local home variable 	
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
    echo(answer);
}

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


