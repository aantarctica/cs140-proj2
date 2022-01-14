#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

typedef struct CMD command;
typedef struct CMDQ cmdq;
typedef struct FQ_ELEMENT fqe;
typedef struct FQ fileq;
typedef struct WARGS wargs;

struct CMD {
    char        type[10];
    char        dir[51];
    char        str[51];
    command*    nextcmd;
};  

struct CMDQ {
    command*    head;
    command*    tail;
};

struct FQ_ELEMENT {
    char        file[51];
    sem_t*      lock;
    cmdq*       cmdq;
    fqe*        next;
};

struct FQ {
    fqe*        head;
    fqe*        tail;
};

struct WARGS{
    cmdq*       queue;
    fileq*      fq;
};

// FUNCTION INITIALIZATIONS //

command* parsecmd(char buffer[150]);
void writecmd(command* cmd);
void readcmd(command* cmd);
void emptycmd(command* cmd);

void worker(wargs* args);

void enqueue(cmdq* queue, command* cmd);
command* dequeue(cmdq* queue);
fqe* searchfile(fileq* fq, command* cmd);

void initfiles();
cmdq* initq();
fileq* initfq();
wargs* initargs();
int rng_sleep();
int rng_empty();


// *** FUNCTION DEFINITIONS *** //

// COMMAND FUNCTIONS //

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


// WORKER FUNCTION //
void worker(wargs* args){
    //sleep(rng_sleep());

    command* exec_cmd = dequeue(args->queue);
    fqe* file = searchfile(args->fq, exec_cmd);
    
    command* cmd = dequeue(file->cmdq);
    if(cmd != NULL){
        sem_wait(file->lock);
        if(strcmp(cmd->type, "write") == 0){
            writecmd(cmd);

        } else if(strcmp(cmd->type, "read") == 0){
            readcmd(cmd);

        } else if(strcmp(cmd->type, "empty") == 0){
            emptycmd(cmd);

        } else printf("Command not found\n");

        free(cmd);
        sem_post(file->lock);
    }
}


// QUEUE FUNCTIONS //

cmdq* initq(){
    cmdq *queue = (cmdq*) malloc(sizeof(cmdq));
    queue->head = NULL;
    queue->tail = NULL;
    return queue;
}

fileq* initfq(){
    fileq *fq = (fileq*) malloc(sizeof(fileq));
    fq->head = NULL;
    fq->tail = NULL;
    return fq;
}

void enqueue(cmdq* queue, command* cmd){
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

command* dequeue(cmdq* queue){
    command *cmd = queue->head;
    if(cmd != NULL) queue->head = cmd->nextcmd;
    return cmd;
}

void fenqueue(fileq* fq, fqe* fp){ 
    /* 
    if queue is empty, 
    set head and tail to fp
    then let fp point to null 
    */
    if(fq->head == NULL){
        fq->head = fp;
        fq->tail = fp;
        fp->next = NULL;
    } 
    /*
    else, let fp point to NULL
    let queue tail point to fp
    then set tail to fp
    */
    
    else{
        fp->next = NULL;
        fq->tail->next = fp;
        fq->tail = fp;
    }
}

fqe* searchfile(fileq* fq, command* cmd){
    fqe* fp = fq->head;
    while(fp != NULL){
        if(strcmp(fp->file, cmd->dir) == 0){
            return fp;
        }
        fp = fp->next;
    }

    // CREATE A NEW FILE QUEUE ELEMENT TO ENQUEUE
    fqe* nfqe = (fqe*) malloc(sizeof(fqe));

    nfqe->cmdq = initq();

    sem_t *lock = (sem_t*) malloc(sizeof(sem_t));
    nfqe->lock = lock;    
    sem_init(nfqe->lock, 0, 1);

    strcpy(nfqe->file, cmd->dir);
    enqueue(nfqe->cmdq, cmd);
    fenqueue(fq, nfqe);
    return nfqe;
}

// MISCELLANEOUS // 
void initfiles(){
    FILE *initcmdtxt, *initreadtxt, *initemptytxt; 
    initcmdtxt = fopen("commands.txt", "w");
    initreadtxt = fopen("read.txt", "w");
    initemptytxt = fopen("empty.txt", "w");

    fclose(initcmdtxt);
    fclose(initreadtxt);
    fclose(initemptytxt);
}

wargs* initargs(){
    cmdq *queue = initq();
    fileq *fq = initfq();

    wargs *args = (wargs*) malloc(sizeof(wargs));
    args->queue = queue;
    args->fq = fq;
    return args;
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
