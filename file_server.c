#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
// #include <pthread.h>
// #include <semaphore.h>

typedef struct CMD command;
struct CMD {
    char type[10];
    char dir[51];
    char str[51];
};  

command* parsecmd(char buffer[150]);
void writecmd(command* cmd);
void readcmd(command* cmd);
void emptycmd(command* cmd);
void printcmd(command* cmd);

int main(){
    char buffer[150]; 

    while(1){
        memset(buffer, '\0', 150);
        printf("> ");
        fgets(buffer, 150, stdin);  // gets user input and stores it to buffer

        if((strcmp(buffer, "\n")) != 0){
            command* cmd = parsecmd(buffer); //sends user input to parsecmd

            // sending command to worker threads
            if(strcmp(cmd->type, "write") == 0){
                writecmd(cmd);
            } else if(strcmp(cmd->type, "read") == 0){
                readcmd(cmd);
            } else if(strcmp(cmd->type, "empty") == 0){
                emptycmd(cmd);
            } else printf("Command not found\n");
        }
    }
    return 0;
}

command* parsecmd(char buffer[150]){
    // initialize local variables
    command *cmd = (command*) malloc(sizeof(command));

    char type[10];
    char dir[51];
    char str[51];
    char argstr[150];

    memset(type, '\0', 10);
    memset(dir, '\0', 51);
    memset(str, '\0', 51);
    memset(argstr,'\0', 150);

    // parse buffer to get cmd type
    sscanf(buffer, "%s %[^\n]s", type, argstr);
    
    // update cmd struct
    if(strcmp(type, "write") == 0){
        sscanf(argstr, "%s %[^\n]s", dir, str); //parse remaining arguments into dir and str
        strcpy(cmd->type, type); 
        strcpy(cmd->dir, dir);                  
        strcpy(cmd->str, str);                  
        
    } else if(strcmp(type, "read") == 0){
        strcpy(cmd->type, type); 
        strcpy(cmd->dir, argstr);
        strcpy(cmd->str, str);
        
    } else if(strcmp(type, "empty") == 0){
        strcpy(cmd->type, type); 
        strcpy(cmd->dir, argstr);
        strcpy(cmd->str, str);
    }

    return cmd;
}

void writecmd(command* cmd){
    printf("%s command called\n", cmd->type);
}

void readcmd(command* cmd){
    printf("%s command called\n", cmd->type);
}

void emptycmd(command* cmd){
    printf("%s command called\n", cmd->type);
}

void printcmd(command* cmd){
    printf("%s\n%s\n%s\n", cmd->type, cmd->dir, cmd->str);
}