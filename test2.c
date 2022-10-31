#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUF_SIZE 1024
#define TOK_BUF_SIZE 64
#define record_size 16

void shell_help(char **args);
void shell_cd(char **args);
void shell_echo(char **args);
void shell_exit(char **args);
void shell_record();
void shell_replay();

int len_builtin();
void shell_launch(char ***args, int *len);
void shell_loop(void);
char ***shell_split_line(char *line, int *len);
char *shell_read_line(void);
void record_cmd(char *cmd);

int cmd_count = 0;
char cmd_list[20][TOK_BUF_SIZE];

char *builtin_str[] = {
    "help",
    "cd",
    "echo",
    "exit",
    "record",
    "replay",
    "mypid"
};

int len_builtin() {
    return sizeof(builtin_str) / sizeof(char*);
}

void shell_replay() {
    
}

void shell_record() {
    printf("Commands History\n");
    for(int i = 1; i <= cmd_count; i++) {
        printf("%d: %s\n", i, cmd_list[i]);
    }
}

void shell_help(char **args) {
    printf("Type command and argument, and hit enter\n");
    printf("=========================================\n");
    printf("The following are built-in functions\n");
    for(int i = 0; i < len_builtin(); i++){
        printf("%d: %s\n",i, builtin_str[i]);
    }
    printf("=========================================\n");
}

void shell_cd(char **args) {
    if(args[1] == NULL) {
        fprintf(stderr, "my_shell: expect an argument after cd\n");
    }else {
        if(chdir(args[1]) != 0) {
            perror("my_shell");
        }
    }
}   

void shell_echo(char **args) {
    if(args[1] != NULL) {
        if(strcmp(args[1], "-n") == 0){
            if(args[2] == NULL){
                fprintf(stderr, "my_shell: expect an argument\n");
            }else {
                int i = 2;
                while(args[i])
                    printf("%s ", args[i++]);
            }
        }else {
            if(args[1] == NULL) {
                fprintf(stderr, "my_shell: expect an argument\n");
            }else {
                int i = 1;
                while(args[i])
                    printf("%s ", args[i++]);
                printf("\n");
            }
        }
    }else {
        fprintf(stderr, "my_shell: expect an argument\n");
    }
}

void shell_exit(char **args) {
    printf("Exiting my_shell...\n");
    exit(0);
}

void shell_launch(char ***args, int *len) {

    int p[2];
    pid_t pid;
    int fd_in = 0;
    if(!strcmp((*args)[0], "cd")) {
        shell_cd(*args);
        args++;
    }
    else if(!strcmp((*args)[0], "exit")) {
        shell_exit(*args);
    } 
    else {
        for(int i = 0; i < (*len); i++) {
            pipe(p);
            pid = fork();
            if(pid == -1) {
                perror("fork error");
                exit(-1);
            }
            else if(pid == 0) {
                if(!strcmp((*args)[0], "help")) {
                    shell_help(*args);
                    exit(1);
                }
                else if(!strcmp((*args)[0], "echo")) {
                    shell_echo(*args);
                    exit(1);
                }
                else if(!strcmp((*args)[0], "record")) {
                    shell_record();
                    exit(1);
                }
                else if(!strcmp((*args)[0], "replay")) {

                }
                else {
                    dup2(fd_in, STDIN_FILENO);
                    if(i + 1 < (*len)) {
                        dup2(p[1],STDOUT_FILENO);
                    }
                    close(p[0]);
                    execvp((*args)[0], *args);
                    exit(0);
                }
            }
            else {
                waitpid(pid, NULL, 0);
                close(p[1]);
                fd_in = p[0];
                args++;
            }
        }
    }

}

void record_cmd(char *cmd) {
    if(cmd_count > 16) {
        for(int i = 2; i <= 16; i++) {
            strcpy(cmd_list[i - 1], cmd_list[i]);
        }
        cmd_count--;
        strcpy(cmd_list[cmd_count], cmd);
    } else {
        strcpy(cmd_list[cmd_count], cmd);
    }
}

void shell_loop(void) {
    char *line;
    char ***args;
    int *cmd_len;
    
    do {
        (*cmd_len) = 0;
        printf(">>> $ ");
        line = shell_read_line(); //read command from standard input
        if(line) cmd_count++;
        args = shell_split_line(line, cmd_len); //Seperate string
        //record command
        record_cmd(line);
        shell_launch(args, cmd_len);
        free(line);
        free(args);
    }while(1);
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