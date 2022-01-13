#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
// #include <semaphore.h>

typedef struct CMD command;
struct CMD {
    char type[10];
    char dir[51];
    char str[51];
};  

// function initializations
command* parsecmd(char buffer[150]);
void writecmd(command* cmd);
void readcmd(command* cmd);
void emptycmd(command* cmd);
void printcmd(command* cmd);
void initfiles();



int main(){
    char buffer[150];
    time_t timestamp;

    // file initializations
    initfiles();
    FILE *cmdtxt;
    
    while(1){
        cmdtxt = fopen("commands.txt", "a");
        memset(buffer, '\0', 150);
        
        printf("> ");
        fgets(buffer, 150, stdin);  // gets user input and stores it to buffer
        
        time(&timestamp);

        if((strcmp(buffer, "\n")) != 0){        // filters out empty inputs
            command* cmd = parsecmd(buffer);    // sends user input to parsecmd
            pthread_t* worker = (pthread_t*) malloc (sizeof(pthread_t*));

            // sending command to worker threads
            if(strcmp(cmd->type, "write") == 0){
                fprintf(cmdtxt, "%s %s", ctime(&timestamp), buffer);
                pthread_create(worker, NULL, (void *) writecmd, cmd);

            } else if(strcmp(cmd->type, "read") == 0){
                fprintf(cmdtxt, "%s %s", ctime(&timestamp), buffer);
                pthread_create(worker, NULL, (void *) readcmd, cmd);

            } else if(strcmp(cmd->type, "empty") == 0){
                fprintf(cmdtxt, "%s %s", ctime(&timestamp), buffer);
                pthread_create(worker, NULL, (void *) emptycmd, cmd);

            } else printf("Command not found\n");
        }

        fclose(cmdtxt);
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
    // initialize thread

    // lock
    FILE *cmddir;
    cmddir = fopen(cmd->dir, "a");

    //fputs(cmd->str, cmddir);
    fprintf(cmddir, "%s\n", cmd->str);
    fclose(cmddir);

    // unlock
}

void readcmd(command* cmd){
    // initialize thread

    // use read lock
    
    FILE *readtxt, *cmddir;
    cmddir = fopen(cmd->dir, "r");
    readtxt = fopen("read.txt", "a"); // append to read.txt
    char rc; // read character variable

    if(cmddir){
        fprintf(readtxt, "%s %s:\t", cmd->type, cmd->dir);

        // read the entire file
        while((rc = fgetc(cmddir)) != EOF) 
            fprintf(readtxt, "%c", rc);
        fprintf(readtxt, "\n");
        fclose(cmddir);
    } else{
        fprintf(readtxt, "%s %s:\tFILE DNE\n", cmd->type, cmd->dir);
    }

    fclose(readtxt);

    // unlock
}

void emptycmd(command* cmd){
    FILE *emptytxt, *cmddir;
    cmddir = fopen(cmd->dir, "r");
    emptytxt = fopen("empty.txt", "a"); // append to empty.txt
    char rc; // read character variable

    if(cmddir){
        fprintf(emptytxt, "%s %s:\t", cmd->type, cmd->dir);

        // read the entire file
        while((rc = fgetc(cmddir)) != EOF) 
            fprintf(emptytxt, "%c", rc);
        fprintf(emptytxt, "\n");
        fclose(cmddir);

        //empty file contents
        cmddir = fopen(cmd->dir, "w");
        fclose(cmddir);
    } else{
        fprintf(emptytxt, "%s %s:\tFILE ALREADY EMPTY\n", cmd->type, cmd->dir);
    }

    fclose(emptytxt);
}

void printcmd(command* cmd){
    printf("%s\n%s\n%s\n", cmd->type, cmd->dir, cmd->str);
}

void initfiles(){
    FILE *initcmdtxt, *initreadtxt, *initemptytxt; 
    initcmdtxt = fopen("commands.txt", "w");
    initreadtxt = fopen("read.txt", "w");
    initemptytxt = fopen("empty.txt", "w");

    fclose(initcmdtxt);
    fclose(initreadtxt);
    fclose(initemptytxt);
}