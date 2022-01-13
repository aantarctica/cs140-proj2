#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

typedef struct CMD command;
struct CMD {
    char type[10];
    char dir[51];
    char str[51];
    command* nextcmd;
};  

typedef struct QUEUE cmdqueue;
struct QUEUE {
    command* head;
    command* tail;
};

// function initializations
command* parsecmd(char buffer[150]);
void writecmd(command* cmd);
void readcmd(command* cmd);
void emptycmd(command* cmd);
void printcmd(command* cmd);
void worker(cmdqueue* queue);
void enqueue(cmdqueue* queue, command* cmd);
command* dequeue(cmdqueue* queue);

void initfiles();
int rng_sleep();
int rng_empty();

// global variables
sem_t trial_lock;

int main(){
    // variable initializations
    cmdqueue *queue = (cmdqueue*) malloc(sizeof(cmdqueue));
    queue->head = NULL;
    queue->tail = NULL;
    sem_init(&trial_lock, 0, 1);
    char buffer[150];
    time_t timestamp;

    // file initializations
    initfiles();
    FILE *cmdtxt;

    while(1){
        cmdtxt = fopen("commands.txt", "a");
        memset(buffer, '\0', 150);
        
        //printf("> ");
        fgets(buffer, 150, stdin);  // gets user input and stores it to buffer
        
        time(&timestamp);

        if((strcmp(buffer, "\n")) != 0){        // filters out empty inputs
            command* cmd = parsecmd(buffer);    // sends user input to parsecmd
            fprintf(cmdtxt, "%s %s", ctime(&timestamp), buffer);
            pthread_t* workerthr = (pthread_t*) malloc(sizeof(pthread_t));

            //update queue
            enqueue(queue, cmd);

            // sending command to worker threads
            pthread_create(workerthr, NULL, (void *) worker, queue);
            pthread_detach(*workerthr);
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
    FILE *cmddir;
    cmddir = fopen(cmd->dir, "a");
    int i = 0;

    //fputs(cmd->str, cmddir);
    while((cmd->str)[i] != '\0'){
        sleep(0.025);
        fprintf(cmddir, "%c", (cmd->str)[i]);
        i++;
    }

    fclose(cmddir);
}

void readcmd(command* cmd){
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
}

void emptycmd(command* cmd){
    FILE *emptytxt, *cmddir;
    cmddir = fopen(cmd->dir, "r");
    emptytxt = fopen("empty.txt", "a"); // append to empty.txt
    char rc;                            // read character variable

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
        sleep(rng_empty());
    } else{
        fprintf(emptytxt, "%s %s:\tFILE ALREADY EMPTY\n", cmd->type, cmd->dir);
    }

    fclose(emptytxt);
}

void worker(cmdqueue* queue){
    sleep(rng_sleep());
    sem_wait(&trial_lock);
    command* cmd = dequeue(queue);
    
    if(cmd != NULL){
        if(strcmp(cmd->type, "write") == 0){
            writecmd(cmd);

        } else if(strcmp(cmd->type, "read") == 0){
            readcmd(cmd);

        } else if(strcmp(cmd->type, "empty") == 0){
            emptycmd(cmd);

        } else printf("Command not found\n");

        free(cmd);
    }
    sem_post(&trial_lock);
}

void enqueue(cmdqueue* queue, command* cmd){
    if(queue->head == NULL){
        queue->head = cmd;
        queue->tail = cmd;
        cmd->nextcmd = NULL;
    } else{
        cmd->nextcmd = NULL;
        queue->tail->nextcmd = cmd;
        queue->tail = cmd;
    }

}

command* dequeue(cmdqueue* queue){
    command *cmd = queue->head;
    if(cmd != NULL) queue->head = cmd->nextcmd;
    return cmd;
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

int rng_sleep(){
    srand(time(0));
	if(rand()%100 >= 80){
        return 6; // return 6 20% of the time
    }
    return 1;
}

int rng_empty(){
    srand(time(0));
	return (7 + (rand()%4)); // return a random integer between 7 and 10
}

/* 
remaining tasks:
1. fix timestamp dapat one line
2. sscanf to strtok
3. locks
*/