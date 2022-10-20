#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUF_SIZE 1024
#define TOK_BUF_SIZE 64

int shell_help(char **args);
int shell_cd(char **args);
int shell_echo(char **args);
int shell_exit(char **args);

int len_builtin();
int shell_launch(char **args);
int shell_execute(char **args);
void shell_loop(void);
char **shell_split_line(char *line);
char *shell_read_line(void);

char *builtin_str[] = {
    "help",
    "cd",
    "echo",
    "exit",
    "record",
    "replay",
    "mypid"
};

int (*builtin_func[])(char**) = {
    &shell_help,
    &shell_cd,
    &shell_echo,
    &shell_exit
};

int len_builtin() {
    return sizeof(builtin_str) / sizeof(char*);
}

int shell_help(char **args) {
    printf("Type command and argument, and hit enter\n");
    printf("=========================================\n");
    printf("The following are built-in functions\n");
    for(int i = 0; i < len_builtin(); i++){
        printf("%d: %s\n",i, builtin_str[i]);
    }
    printf("=========================================\n");
    return 1;
}

int shell_cd(char **args) {
    if(args[1] == NULL) {
        fprintf(stderr, "my_shell: expect an argument after cd\n");
    }else {
        if(chdir(args[1]) != 0) {
            perror("my_shell");
        }
    }
    return 1;
}   

int shell_echo(char **args) {
    if(args[1] != NULL) {
        if(strcmp(args[1], "-n") == 0){
            if(args[2] == NULL){
                fprintf(stderr, "my_shell: expect an argument\n");
            }else {
            printf("%s", args[2]);
            }
        }else {
            if(args[1] == NULL) {
                fprintf(stderr, "my_shell: expect an argument\n");
            }else {
                printf("%s\n", args[1]);
            }
        }
    }else {
        fprintf(stderr, "my_shell: expect an argument\n");
    }
    
    return 1;
}

int shell_exit(char **args) {
    printf("Exiting my_shell...\n");
    return EXIT_SUCCESS;
}

int shell_execute(char **args) {
    if(args[0] == NULL) {
        return 1;
    }
    for(int i = 0; i < len_builtin(); i++) {
        if(strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }
    return shell_launch(args);
}

int shell_launch(char **args) {

    int status;

    pid_t pid = fork();
    if(pid == 0) { //child process
        if(execvp(args[0], args) == -1) {
            perror("my_shell");
        }
        exit(EXIT_FAILURE);
    }
    else if(pid < 0) {
        perror("my_shell");
    }
    else { //main process
        do {
            waitpid(pid, &status, WUNTRACED);
        }while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

void shell_loop(void) {
    char *line;
    char **args;
    int status;
    do {
        printf(">>> $ ");
        line = shell_read_line(); //read command from standard input
        args = shell_split_line(line); //Seperate string
        status = shell_execute(args); //run command
        free(line);
        free(args);
    }while(status);
}

#define SHELL_TOK_DELIM " \t\r\n\a"
char **shell_split_line(char *line) {
    int size = BUF_SIZE;
    int position = 0;
    char **tokens = malloc(sizeof(char*)*size);
    char *token, **tokens_backup;
    if(!tokens) {
        fprintf(stderr, "my_shell: allocation error\n");
        exit(EXIT_FAILURE);
    }
    token = strtok(line, SHELL_TOK_DELIM);
    while (token != NULL) {
        tokens[position++] = token;
        if(position >= size) {
            size += TOK_BUF_SIZE;
            tokens_backup = tokens;
            tokens = realloc(tokens, size*sizeof(char*));
            if(!tokens) {
                free(tokens_backup);
                fprintf(stderr, "my_shell: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, SHELL_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

char *shell_read_line(void) {
    int size = BUF_SIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char)*size);
    int c;

    if(!buffer) {
        fprintf(stderr, "my_shell: allocation error\n");
        exit(EXIT_FAILURE);
    }
    while(1) {
        c = getchar();
        if(c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        }
        else {
            buffer[position++] = c;
        }
    }
    if(position > size) {
        size += BUF_SIZE;
        buffer = realloc(buffer, size);
        if(!buffer) {
            fprintf(stderr, "my_shell: allocation error");
        }
    }
}

int main(int argc, char **argv) {
    
    printf("===========================================\n");
    printf("* Welcome to my shell                     *\n");
    printf("*                                         *\n");
    printf("* Type 'help' to check built-in functions *\n");
    printf("===========================================\n");
    shell_loop();
    return EXIT_SUCCESS;
}