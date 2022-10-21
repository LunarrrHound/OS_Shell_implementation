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
int shell_launch(char ***args, int *len);
int shell_execute(char ***args, int *len);
void shell_loop(void);
char ***shell_split_line(char *line, int *len);
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
    // printf("%s\n", args[0]);
    // printf("%s\n", args[1]);
    if(args[1] != NULL) {
        if(strcmp(args[1], "-n") == 0){
            if(args[2] == NULL){
                fprintf(stderr, "my_shell: expect an argument\n");
            }else {
            // printf("%s", args[2]);
                int i = 2;
                while(args[i])
                    printf("%s ", args[i++]);
            }
        }else {
            if(args[1] == NULL) {
                fprintf(stderr, "my_shell: expect an argument\n");
            }else {
                // printf("%s\n", args[1]);
                int i = 1;
                while(args[i])
                    printf("%s ", args[i++]);
                printf("\n");
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

int shell_execute(char ***args, int *len) {
    // if(args[0] == NULL) {
    //     return 1;
    // }
    // for(int i = 0; i < len_builtin(); i++) {
    //     if(strcmp(args[0], builtin_str[i]) == 0) {
    //         return (*builtin_func[i])(args);
    //     }
    // }
    // return shell_launch(args);
    if(args == NULL){
        return 1;
    }
    for(int i = 0; i < (*len); i++) {
        for(int j = 0; j < len_builtin(); j++) {
            if(strcmp(args[i][0], builtin_str[j]) == 0) {
                return (*builtin_func[j])(*args);
            }
        }
    }
    return shell_launch(args, len);
}

int shell_launch(char ***args, int *len) {
    int fd[2];
    pid_t pid;
    int fdd = 0;
    while(*args != NULL) {
        pipe(fd);
        if((pid = fork()) == -1) {
            perror("fork");
            exit(1);
        }else if(pid == 0) {
            dup2(fdd, 0);
            if(*(args + 1) != NULL) {
                dup2(fd[1], 1);
            }
            close(fd[0]);
            if(execvp((*args)[0], *args) < 0) exit(1); //do some function
        }else {
            wait(NULL);
            close(fd[1]);
            fdd = fd[0];
            args++;
        }
        wait(NULL);
    }
    wait(NULL);
    return 1;
}

void shell_loop(void) {
    char *line;
    char ***args;
    int status;
    int* cmd_len;
    do {
        (*cmd_len) = 0;
        printf(">>> $ ");
        line = shell_read_line(); //read command from standard input
        args = shell_split_line(line, cmd_len); //Seperate string
        // for(int i = 0; i < cmd_len; i++){
        //     printf("%s\n", args[i][0]);
        // }
        status = shell_execute(args, cmd_len); //run command
        free(line);
        free(args);
    }while(status);
}

#define SHELL_TOK_DELIM " \t\r\n\a"
char ***shell_split_line(char *line, int *len) {
    int size = TOK_BUF_SIZE;
    int pos = 0;
    // int cmd_count = 0;
    if(line != NULL) {
        *len = 1;
        for(int i = 0; i < strlen(line) ; i++) 
            if(line[i] == '|') 
                (*len)++;
    }
    char*** tokens = (char***)malloc(sizeof(char**)*(*len));
    char *s = malloc(sizeof(char)*size);
    int i = 0, j = 0, k = 0;
    if(!tokens) {
        fprintf(stderr, "my_shell: allocation error\n");
        exit(EXIT_FAILURE);
    }
    tokens[k] = (char**)malloc(sizeof(char*)*size);
    for(pos = 0; pos < strlen(line) + 1; pos++) {
        if(line[pos] == ' ' || line[pos] == '\0') {
            if(strcmp(s, "|") == 0) {
                tokens[++k] = (char**)malloc(sizeof(char*)*size);
                s = malloc(sizeof(char)*size);
                i = 0;
                j = 0;
            }else {
                tokens[k][i] = malloc(sizeof(char*)*strlen(s));
                // tokens[k][i++] = s;
                strcpy(tokens[k][i++], s);
                s = malloc(sizeof(char)*size);
                j = 0;
            }
        }else {
            s[j++] = line[pos];
        }
        // tokens[k][i] = NULL;
    }
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