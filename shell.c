#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>

typedef struct {
    int size;
    char **items;
} tokenlist;

// struct to create a job
typedef struct {
    pid_t pid;
    pid_t waitpid;
    int num;
    tokenlist *command;
} job;

// struct to hold all active background jobs
typedef struct {
    int size;
    job **item;
} joblist;

char *get_input(void);
tokenlist *get_tokens(char *input);
tokenlist *new_tokenlist(void);
void add_token(tokenlist *tokens, char *item);
void free_tokens(tokenlist *tokens);

job *new_job(void);
void free_job(job *item);
joblist *new_joblist(void);
void add_job(joblist *list, pid_t p, pid_t w, int n, tokenlist *c);
void remove_job(joblist *list, int i);
void free_jobslist(joblist *list);

bool echo(tokenlist *input, bool check_out, bool check_in, int outpos, int inpos);
int change_direct(tokenlist *input);
tokenlist *tilde_expand(tokenlist *input, int pos);
char *path_search(tokenlist *input);
bool exec_command(tokenlist *args, bool in_redirection, bool out_redirection, bool background, int inposition, int outposition, int backpos);
bool pipeline(int pipepos1, int pipepos2, int backpos, bool check_pipe2, bool background, tokenlist *args);
pid_t back_exec(pid_t pid, tokenlist* c);
void exit_call(time_t start, int comm_count);
bool print_jobs(void);

joblist *BACKGROUNDLIST;    // list of active background processes
int jobnum; // counts the number of jobs to set job number in struct
int main()
{
    int commandcount = 0;   // counts the number of commands
    time_t start_time;
    start_time = time(NULL);
    BACKGROUNDLIST = new_joblist();

    while (1) {
        char *cwd = getcwd(NULL, 0);
        setenv("PWD", cwd, 1);
        free(cwd);

        // block to check if any background processes have finished
        if (BACKGROUNDLIST->size > 0){
            for (int i = 0; i < BACKGROUNDLIST->size; i++){
                if (waitpid(BACKGROUNDLIST->item[i]->pid, NULL, WNOHANG) != 0){
                    printf("[%d] Done [%s]\n", BACKGROUNDLIST->item[i]->num, BACKGROUNDLIST->item[i]->command->items[0]);
                    remove_job(BACKGROUNDLIST, i);
                }
            }
        }

        if(BACKGROUNDLIST->size == 0)
            jobnum = 0;



        const char *user = getenv("USER");
        const char *machine = getenv("MACHINE");
        const char *pwd = getenv("PWD");

        printf("%s@%s:%s>", user, machine, pwd);


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
        bool check_background = false;
        bool check_jobprint = false;
        bool check_cd = false;
        bool check_exit = false;

        int tildepos = -1;
        int echopos = -1;
        int pipepos1 = -1;
        int pipepos2 = -1;
        int inpos = -1;
        int outpos = -1;
        int backpos = -1;

        tokenlist *tokens = get_tokens(input);

        // block to check for empty command line
        if (tokens->items[0] == NULL){
            continue;
        }

        // block for cd command
        if(strcmp(tokens->items[0],"cd") == 0){
            if(change_direct(tokens) == 0){
                check_cd = true;
                commandcount += 1;
            }
			else
				check_cd = true;
			}

        // block to check if command was jobs
        if(strcmp(tokens->items[0], "jobs") == 0){
            check_jobprint = true;
            if(print_jobs()){
                commandcount += 1;
            }
            else{
                printf("No active background processes\n");
            }
        }

        // block for exit
        if( (strcmp(tokens->items[0],"exit") == 0) && tokens->size == 1){
            check_exit = true;
            exit_call(start_time, commandcount + 1);
            free(input);
            free_tokens(tokens);
            free(BACKGROUNDLIST);
            return 0;
        }
        else if((strcmp(tokens->items[0],"exit") == 0) && tokens->size != 1){
            printf("Error: 'exit' should not have arguments\n");
			continue;
        }

        for (int i = 0; i < tokens->size; i++){

            // block for echo call
            if(strcmp(tokens->items[0],"echo") == 0){
                check_echo = true;
                echopos = 0;
            }

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

            // block for background processes
            if(strcmp(tokens->items[i], "&") == 0){
                check_background = true;
                backpos = i;
            }

        }

        // block to check if arguments contain tildes
        if (check_tilde){
            char *e;
            int index;
            e = strchr(tokens->items[tildepos],'~');
            index =(int)(e-tokens->items[tildepos]);
            if(index > 0){

            }
            if(index == 0){
                tokens = tilde_expand(tokens,tildepos);
            }
        }

        // block to check if echo should be executed
        if (check_echo && !check_pipe1){

            if (echo(tokens, check_outredirect, check_inredirect, outpos, inpos)){
                commandcount += 1;
            }
            else{
                printf("Error: executing echo.\n");
            }
        }

        // block to check if a command should be executed
        if (!check_pipe1 && !check_echo)
        {
			if(check_cd){
				continue;
			}
			else if(check_tilde){
				continue;
			}
            else if (exec_command(tokens, check_inredirect, check_outredirect, check_background, inpos, outpos, backpos)){
                commandcount += 1;
            }
			
            else{
                printf("Error: executing %s\n", tokens->items[0]);
            }

        }
            // block to check if command contains piping
        else if (!check_echo){
            if (pipeline(pipepos1, pipepos2, backpos, check_pipe2, check_background, tokens)){
                if (check_pipe2){
                    commandcount += 3;
                }
                else{
                    commandcount += 2;
                }
            }
            else{
                printf("Error: in pipeline\n");
            }
        }

        free(input);
        free_tokens(tokens);

    }

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

// creates new job
job *new_job(void){
    job *item = (job *) malloc(sizeof(job));
    item->pid = 0;  // pid of child
    item->waitpid = 0;
    item->num = 0;
    item->command = new_tokenlist();
    return item;
}

// deallocates a single job
void free_job(job *item){
    free(item);
}

// creates a new joblist
joblist *new_joblist(){
    joblist *list = (joblist *) malloc(sizeof(joblist));
    list->size = 0;

    list->item = (job **) malloc(sizeof(job *));
    list->item[0] = NULL; /* make NULL terminated */
    return list;
}


// adds a job to the current joblist
void add_job(joblist *list, pid_t p, pid_t w, int n, tokenlist *c){

    int i = list->size;
    list->item = (job **) realloc(list->item, (i + 2) * sizeof(job *));
    list->item[i] = new_job();
    list->item[i + 1] = NULL;
    list->item[i]->pid = p;
    list->item[i]->waitpid = w;
    list->item[i]->num = n;
    for(int x = 0; x < c->size; x++){
        add_token(list->item[i]->command, c->items[x]);
    }


    list->size += 1;

}

void remove_job(joblist *list, int i){
    for(i; i < list->size; i++){
        list->item[i] = list->item[i+1];
    }
    list->size -= 1;
}

// deallocates entire joblist
void free_jobs(joblist *list){
    for (int i = 0; i < list->size; i++)
        free_job(list->item[i]);
    free(list->item);
    free(list);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


bool echo(tokenlist* input, bool check_out, bool check_in, int outpos, int inpos){
    tokenlist* answer = new_tokenlist();    // stores parsed
    FILE *fp;   // file pointer
    char *temp;
    char *file;

    // block to open output file
    if(check_out) {
        fp = fopen(input->items[outpos + 1], "w+");         //open the file name
    }                                                                       //"w+" makes the file readable,
    if(fp == NULL){                                                         //writable, and truncates if theres already
        printf("Error: opening outfile\n");                          // something in the file
        return false;                                                       // if fp==null, file couldnt open.
    }

    for (int i = 1; i < input->size; i++){                                  //get the tokens after the echo command
        add_token(answer, input->items[i]);
    }

    char* home;
    int index;
    int counter = 0;
    const char ch = '$';
    for (int j = 0; j < answer->size; j++){
        if(strchr(answer->items[j],'$') != NULL){               //checking for environmental variables
            temp = answer->items[j];
            if (temp[0] == '$'){
                while(counter < strlen(temp)){
                    temp[counter] = temp[counter+1];
                    counter += 1;
                }
                if(getenv(temp) != NULL){
                    answer->items[j] = getenv(temp);
                }
            }
        }
        counter = 0;
        temp = NULL;
        if(!check_out && !check_in)
            printf("%s ",answer->items[j]);

    }

    if(check_out){                                                 //if output redirection
        for(int i = 0 ; i < outpos-1; i++)                         //print tokens before the ">" into the file indicated
            fprintf(fp, "%s ", answer->items[i]);

        fprintf(fp, "\n");
        fclose(fp);                                                //close the file
    }

    if(!check_out)
        printf("\n");


    return true;
}

int change_direct(tokenlist *input){
    int num;
    if(input->size > 2) {                                           //cd shouldnt have more than one argument
        printf("Too many variables, try again.\n");         // if theres more than one throw an error
        return 1;
    }

    if(input->items[1] == NULL || strcmp(input->items[1], "~") == 0) {                                   //if theres no arguments the default
        chdir(getenv("HOME"));                              // directory to change to is the home directory
        return 0;
    }
    num = chdir(input->items[1]);
    if(num != 0) {                                                  //if chdir doesnt return 0 the change directory failed
        printf("Change directory failed. \n");
        return 1;
    }
    else                                                          //chdir() successfully ran
        return 0;
}

tokenlist *tilde_expand(tokenlist *input, int pos){

    tokenlist *newlist  = new_tokenlist();
    char home[strlen(getenv("HOME"))];
    strcpy(home,getenv("HOME"));	// environmental home variable
    char *e;
    int index;

    char copy[(int)strlen(input->items[pos])]; //copy of the input
    strcpy(copy,input->items[pos]);

    int copy_size = (int) strlen(copy);       	//didnt like when i would just pass
    int home_size = (int) strlen(home);         //strlen(variable), had to set these for use in the loops
    int result_size = copy_size + home_size;    //the resulting string size

    char result[result_size]; //used for if there is something other than ~
    char result2[strlen(home)]; //used for only ~

    e = strchr(copy,'~');   //getting a pointer to the first instance of ~
    index = (int)(e - copy);    //getting the index by subtracting e from the whole string

    if(pos == 0){                       //if pos = 0 then theres only a ~
        printf("%s \n",home);    //all we would need to do is print the home directory
        add_token(newlist, home);       //this gives us some errors, not sure why
        return newlist;
    }
    int count = 1;
    if(index == 0){                               //checks if the tilde is the first item of the token
        if( (int) strlen(copy) > 1){              //if it is, continue with the expansion
            for(int i = 0; i < home_size; i++){   //first set the home variable to the first part of the array
                result[i] = home[i];
            }
            for(int i = strlen(home); i < result_size; i++){ //then set the remainder
                result[i] = copy[count];
                count += 1;
            }

        }
        else if((int) strlen(copy) == 1){                    //if there's just a tilde in the tokens just copy home into the return
            strcpy(result2,home);
        }
    }
    for(int i = 0; i < input->size; i++) { 					//add our new strings into the tokenlist
        if(i == pos) {
            if((int) strlen(copy) > 1)
                add_token(newlist,result);
            else if((int) strlen(copy) == 1)
                add_token(newlist,result2);
        }
        else
            add_token(newlist, input->items[i]);
    }
    return newlist;                                        //return the tokenlist with our string added in
}

char* path_search(tokenlist *input)
{
    // static variable initialization
    char slash[2] = "/";	// slash for command path variable
    const char delim[2] = ":";	// delimiter for path string variable
    const char *path = getenv("PATH");	// environmental path variable
    int check = 1;		// error checking variable that indicates whether the file was found or not
    char copy[strlen(path)];	// local path variable
    strcpy(copy, path);
    char *temp = strtok(copy, delim);	// temp variable to store first directory path

    // dynamic memory allocation
    char *command = (char *) calloc(strlen(slash) + strlen(input->items[0]), sizeof(char));	// full command argument variable
    if(command == NULL){
        return NULL;
    }

    // creating command path
    strcpy(command,slash);
    strcat(command,input->items[0]);

    char *filepath = (char *) calloc(strlen(command) + strlen(temp), sizeof(char));	// full command file path variable
    if (filepath == NULL){
        return NULL;
    }


    // directory search loop
    while(strcmp(temp, ".") != 0 || check != 0 )
    {

        // creating full command file path variable for each directory
        strcpy(filepath, temp);
        strcat(filepath, command);

        // checking if file exists in the directory
        check = access(filepath,F_OK);
        if (check == 0){
            free(command);
            return filepath;
        }

        temp = strtok(NULL, delim);
        filepath = (char *) realloc(filepath, sizeof(char) * (strlen(temp) + strlen(command)));
        if(strcmp(temp, ".") == 0){
            break;
        }
    }

    // memory de-allocation
    free(command);
    //free(token);
    free(filepath);
    //free(file);
    return NULL;
}


bool exec_command(tokenlist *args, bool in_redirection, bool out_redirection, bool background, int inposition, int outposition, int backpos){
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
        // if block for if the input redirection comes first
        if (inposition < outposition){
            index = inposition;
            // loop to add tokens before redirection
            //start of input redirect
            for (int i = 0; i < index; i++){
                add_token(arglist, args->items[i]);
            }
            char *path = path_search(args);
            if(path == NULL){
                return false;
            }
            pid = fork();
            if(pid < 0){
                printf("Error creating child process\n");
                return false;
            }
            if(pid == 0){
                int in = open(infile, O_RDONLY);
                int out = open(outfile, O_RDWR | O_CREAT | O_TRUNC, 0666);
                close(0);
                dup(in);
                close(in);

                close(1);
                dup(out);
                close(out);

                execv(path, arglist->items);
            }
            else
            if(background){
                wait = back_exec(pid, args);
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
        char *path = path_search(args);
        if(path == NULL){
            return false;
        }
        pid = fork();
        if(pid < 0){
            printf("Error creating child process\n");
            return false;
        }
        if(pid == 0){
            int in = open(infile, O_RDONLY);
            close(0);
            dup(in);
            close(in);

            execv(path, arglist->items);
        }
        else
        if(background){
            wait = back_exec(pid, args);
        }
        else
            wait = waitpid(pid, &status, 0);

    }

        // block for if there is only output redirection
    else if (out_redirection == true){
        index = outposition;
        outfile = args->items[outposition+1];
        for (int i = 0; i < index; i++)
            add_token(arglist, args->items[i]);
        char *path = path_search(args);
        if(path == NULL){
            return false;
        }
        pid = fork();
        if(pid < 0){
            printf("Error creating child process\n");
            return false;
        }
        if(pid == 0){
            int out = open(outfile, O_RDWR | O_CREAT | O_TRUNC, 0666);
            close(1);
            dup(out);
            close(out);

            execv(path, arglist->items);
        }
        else
        if(background){
            wait = back_exec(pid, args);
        }
        else
            wait = waitpid(pid, &status, 0);

    }


        // block if there is no redirection statements
    else {
        infile = NULL;
        outfile = NULL;

        if(background){
            for(int i = 0; i < args->size; i++){
                if(i != backpos)
                    add_token(arglist, args->items[i]);
                else{

                }
            }
            char *path = path_search(args);
            if(path == NULL){
                return false;
            }
            pid = fork();
            if(pid < 0){
                printf("Error creating child process\n");
                return false;
            }
            // child process
            if (pid == 0){
                // executing command
                execv(path, arglist->items);
            }
                // parent process
            else {
                // waiting for command execution to return
                wait = back_exec(pid, args);
            }
        }
        else{
            // creating new process
            char *path = path_search(args);
            if(path == NULL){
                return false;
            }
            pid = fork();
            if(pid < 0){
                printf("Error creating child process\n");
                return false;
            }
            // child process
            if (pid == 0){
                // executing command

                execv(path, args->items);
            }
                // parent process
            else {
                wait = waitpid(pid, &status, 0);
            }
        }

    }


    free_tokens(arglist);
    return true;
}

bool pipeline(int pipepos1, int pipepos2, int backpos, bool check_pipe2, bool background, tokenlist *args){
    int fd[4];  // stored fiple pointers
    pid_t pid1; // pid for first command
    pid_t pid2; // pid for second command
    pid_t pid3; // pid for possible third command
    pid_t wait1, wait2, wait3;  // variables that will store what waitpid returns
    int status1, status2, status3;  // hold the status of waitpid
    tokenlist *arglist1 = new_tokenlist();		// parsed argument list variable
    tokenlist *arglist2 = new_tokenlist();		// parsed argument list variable
    tokenlist *arglist3 = new_tokenlist();		// parsed argument list variable
    char *path1, *path2, *path3;    // parsed path variables to pass to path search
    // block to check if there is a second pipe
    if (!check_pipe2){
        path3 = NULL;
    }


    // loops for parsing argument list from input
    for (int i = 0; i < pipepos1; i++){
        add_token(arglist1, args->items[i]);
    }
    // block to parse argument list for two pipes
    if(check_pipe2){
        for (int i = pipepos1 + 1; i < pipepos2; i++ ){
            add_token(arglist2, args->items[i]);
        }
        if(background) {
            for (int i = pipepos2 + 1; i < args->size-1; i++) {
                add_token(arglist3, args->items[i]);
            }
        }
        else
            for (int i = pipepos2 + 1; i < args->size; i++) {
                add_token(arglist3, args->items[i]);
            }
        pipe(fd);
        pipe(fd + 2);
    }

        // block to parse argument list for a single pipe
    else{
        if(background) {
            for (int i = pipepos1 + 1; i < args->size-1; i++) {
                add_token(arglist2, args->items[i]);
            }
        }
        else
            for (int i = pipepos1 + 1; i < args->size; i++) {
                add_token(arglist2, args->items[i]);
            }
        pipe(fd);
    }

    //block to create child processes for two pipes
    if (check_pipe2){
        path1 = path_search(arglist1);
        if(path1 == NULL){
            printf("Error command not found\n");
            return false;
        }

        pid1 = fork();	// stout redirection
        // block for stdout redirection
        if(pid1 < 0){
            printf("Error creating child process\n");
            return false;
        }
        if(pid1 == 0){
            dup2(fd[1], 1);
            close(fd[0]);
            close(fd[1]);
            close(fd[2]);
            close(fd[3]);
            execv(path1, arglist1->items);
        }
        else {
            path2 = path_search(arglist2);
            if(path2 == NULL){
                printf("Error command not found\n");
                return false;
            }
            pid2 = fork();
            if(pid2 < 0){
                printf("Error creating child process\n");
                return false;
            }
            if (pid2 == 0){
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
                if(path3 == NULL){
                    printf("Error command not found\n");
                    return false;
                }
                pid3 = fork();
                if(pid3 < 0){
                    printf("Error creating child process\n");
                    return false;
                }
                if (pid3 == 0){
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

        // block for background processes
        if(background){
            wait1 = waitpid(pid1, &status1, 0);
            wait2 = waitpid(pid2, &status2, 0);
            wait3 = back_exec(pid3, arglist3);
        }
            // block for foreground processes
        else{
            wait1 = waitpid(pid1, &status1, 0);
            wait2 = waitpid(pid2, &status2, 0);
            wait3 = waitpid(pid3, &status3, 0);
        }

    }
        //block to create child processes for one pipe
    else{
        path1 = path_search(arglist1);
        if(path1 == NULL){
            printf("Error command not found\n");
            return false;
        }
        pid1 = fork();	// stout redirection
        // block for stdout redirection
        if(pid1 < 0){
            printf("Error creating child process\n");
            return false;
        }
        if(pid1 == 0){
            dup2(fd[1], 1);
            close(fd[0]);
            close(fd[1]);
            execv(path1, arglist1->items);
        }
        path2 = path_search(arglist2);
        if(path2 == NULL){
            printf("Error command not found\n");
            return false;
        }
        pid2 = fork();
        if(pid2 < 0){
            printf("Error creating child process\n");
            return false;
        }
        if (pid2 == 0){
            dup2(fd[0], 0);
            close(fd[0]);
            close(fd[1]);
            execv(path2, arglist2->items);
        }

        close(fd[0]);
        close(fd[1]);
        // deallocating parsed token lists
        free_tokens(arglist1);
        free_tokens(arglist2);
        // waitpid for background processes
        if(background){
            wait1 = waitpid(pid1, &status1, 0);
            wait2 = back_exec(pid2, arglist2);
        }
            // waitpid for foreground processes
        else{
            wait1 = waitpid(pid1, &status1, 0);
            wait2 = waitpid(pid2, &status2, 0);
        }
    }

    return true;
}

pid_t back_exec(pid_t pid, tokenlist* c){
    pid_t wait; // returns the waitpid of the child process calling it
    wait = waitpid(pid, NULL, WNOHANG);
    jobnum += 1;    // increments job number counter
    add_job(BACKGROUNDLIST, pid, wait, jobnum, c);  // add current process to background list
    printf("[%d] %d\n", BACKGROUNDLIST->item[BACKGROUNDLIST->size-1]->num, BACKGROUNDLIST->item[BACKGROUNDLIST->size-1]->pid);
    return wait;
}

void exit_call(time_t start, int comm_count){

    while (BACKGROUNDLIST->size > 0){
        for (int i = 0; i < BACKGROUNDLIST->size; i++){
            if (waitpid(BACKGROUNDLIST->item[i]->pid, NULL, WNOHANG) != 0){
                remove_job(BACKGROUNDLIST, i);
            }
        }
    }

    time_t end = time(NULL);
    int time_elapsed = (int)difftime(end, start);

    printf("Shell ran for %d seconds and executed %d commands.\n", time_elapsed, comm_count);
}

bool print_jobs(void){
    // block for if there are active
    if(BACKGROUNDLIST->size > 0) {
        // loop to print active background processes
        for (int i = 0; i < BACKGROUNDLIST->size; i++) {
            printf("[%d] [%d] [%s]\n", BACKGROUNDLIST->item[i]->num, BACKGROUNDLIST->item[i]->pid, BACKGROUNDLIST->item[i]->command->items[0]);
        }
        return true;
    }
        // block for no active background processes
    else{
        return false;
    }
}