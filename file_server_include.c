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
    command*    next;
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

void worker(wargs* args);

fqe* searchfqe(fileq* fq, command* cmd);
fqe* createfqe(fileq* fq, command* cmd);
void enqueue(cmdq* queue, command* cmd);
command* dequeue(cmdq* queue);
void fenqueue(fileq* fq, fqe* fp);
cmdq* initq();
fileq* initfq();

command* parsecmd(char buffer[150]);
void writecmd(command* cmd);
void readcmd(command* cmd);
void emptycmd(command* cmd);
void invalidcmd(command* cmd);

void initfiles();
void updatefile(char* filename, command* cmd);
wargs* initargs();
int rng_sleep();
int rng_empty();


void traverseFILES(fileq* fq);
void traversecmdq(cmdq* queue);

time_t timestamp;

// *** FUNCTION DEFINITIONS *** //

// WORKER FUNCTION //
void worker(wargs* args){
    sleep(rng_sleep());

    command* exec_cmd = dequeue(args->queue);
    fqe* file = searchfqe(args->fq, exec_cmd);  

    sem_wait(file->lock);
    
    command* cmd = dequeue(file->cmdq);

    if(strcmp(cmd->type, "write") == 0){
        writecmd(cmd);

    } else if(strcmp(cmd->type, "read") == 0){
        readcmd(cmd);

    } else if(strcmp(cmd->type, "empty") == 0){
        emptycmd(cmd);

    } else invalidcmd(cmd);
    
    // update done.txt
    updatefile("done.txt", cmd);
    free(cmd);

    sem_post(file->lock);
}


// QUEUE FUNCTIONS //
fqe* searchfqe(fileq* fq, command* cmd){
    fqe* fp = fq->head;
    while(fp != NULL){
        if(strcmp(fp->file, cmd->dir) == 0){
            return fp;
        }
        fp = fp->next;
    }

    // CREATE A NEW FILE QUEUE ELEMENT TO ENQUEUE
    return createfqe(fq, cmd);
}

fqe* createfqe(fileq* fq, command* cmd){
    fqe* nfqe = (fqe*) malloc(sizeof(fqe));

    nfqe->cmdq = initq();

    sem_t *lock = (sem_t*) malloc(sizeof(sem_t));
    nfqe->lock = lock;    
    sem_init(nfqe->lock, 0, 1);

    strcpy(nfqe->file, cmd->dir);
    fenqueue(fq, nfqe);
    
    return nfqe;
}

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
    command* ncmd = (command*) malloc(sizeof(command));
    strcpy(ncmd->type, cmd->type);
    strcpy(ncmd->dir, cmd->dir);
    strcpy(ncmd->str, cmd->str);
    ncmd->next = NULL;

    if(queue->head == NULL){
        queue->head = ncmd;
        queue->tail = ncmd;
    } else{
        queue->tail->next = ncmd;
        queue->tail = ncmd;
    }
}

command* dequeue(cmdq* queue){
    command *cmd = (command*) malloc(sizeof(command));
    strcpy(cmd->type, queue->head->type);
    strcpy(cmd->dir, queue->head->dir);
    strcpy(cmd->str, queue->head->str);
    
    if(cmd != NULL) queue->head = queue->head->next;
    return cmd;
}

void fenqueue(fileq* fq, fqe* fp){ 
    fqe* nf = (fqe*) malloc(sizeof(fqe));
    strcpy(nf->file, fp->file);
    nf->cmdq = fp->cmdq;
    nf->lock = fp->lock;
    nf->next = NULL;

    if(fq->head == NULL){
        fq->head = nf;
        fq->tail = nf;
    } else{
        fq->tail->next = nf;
        fq->tail = nf;
    }
}


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
        sscanf(argstr, "%s %[^\n]s", dir, str);                     
    } else if(strcmp(type, "read") == 0){ 
        strcpy(dir, argstr);        
    } else if(strcmp(type, "empty") == 0){ 
        strcpy(dir, argstr);
    } else{
        strcpy(cmd->type, "Invalid");                
    }
    strcpy(cmd->type, type); 
    strcpy(cmd->dir, dir);                  
    strcpy(cmd->str, str);

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
    
    // temp checker
    fprintf(cmddir, "\n");

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

void invalidcmd(command* cmd){
    printf("Command not found\n");
}

// MISCELLANEOUS // 
void initfiles(){
    FILE *initcmdtxt, *initreadtxt, *initemptytxt, *initdonetxt; 
    initcmdtxt = fopen("commands.txt", "w");
    initreadtxt = fopen("read.txt", "w");
    initemptytxt = fopen("empty.txt", "w");
    initdonetxt = fopen("done.txt", "w");

    fclose(initcmdtxt);
    fclose(initreadtxt);
    fclose(initemptytxt);
    fclose(initdonetxt);
}

wargs* initargs(){
    cmdq *queue = initq();
    fileq *fq = initfq();

    wargs *args = (wargs*) malloc(sizeof(wargs));
    args->queue = queue;
    args->fq = fq;
    return args;
}

void updatefile(char* filename, command* cmd){
    time(&timestamp);
    char *t = ctime(&timestamp);
    t[strlen(t) - 1] = 0;
    
    FILE *stream = fopen(filename, "a");
    fprintf(stream, "[%s]\t%s %s %s\n", t, cmd->type, cmd->dir, cmd->str);
    fclose(stream);
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

void traverseFILES(fileq* fq){
    fqe* fp = fq->head;
    while(fp != NULL){
        traversecmdq(fp->cmdq);
        fp = fp->next;
    }
}

void traversecmdq(cmdq* queue){
    command* cmd = queue->head;
    while(cmd != NULL){
        printf("%s %s %s\n", cmd->type, cmd->dir, cmd->str);
        cmd = cmd->next;
    }
}



/* 
remaining tasks:
1. fix timestamp dapat one line
2. sscanf to strtok
3. locks

--debugging--
1. make a separate function that traverses the file queue
one for the main
and one for worker thread

*/
