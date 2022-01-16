#include "defs.h"

// * * * F U N C T I O N   D E F I N I T I O N S  * * * //

// *** W O R K E R  F U N C T I O N *** //

// executes the worker threads
void worker(fqe* file){
    sleep(rng(0));  

    sem_wait(file->lock);
    
    command* cmd = dequeue(file->cmdq);
    if(strcmp(cmd->type, "write") == 0){
        writecmd(cmd);

    } else if(strcmp(cmd->type, "read") == 0){
        readcmd(cmd);

    } else if(strcmp(cmd->type, "empty") == 0){
        emptycmd(cmd);

    } else invalidcmd();
    
    // update done.txt
    updatefile("done.txt", cmd);
    free(cmd);

    sem_post(file->lock);
}

// *** Q U E U E  F U N C T I O N S *** //

// traverses the queue to search 
// for a file if it exists, 
// else creates a new one
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

// creates a new file queue element
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

// initializes the command queue
cmdq* initq(){
    cmdq *queue = (cmdq*) malloc(sizeof(cmdq));
    queue->head = NULL;
    queue->tail = NULL;
    return queue;
}

// initializes the file queue
fileq* initfq(){
    fileq *fq = (fileq*) malloc(sizeof(fileq));
    fq->head = NULL;
    fq->tail = NULL;
    return fq;
}

// adds new command into the command queue
void enqueue(cmdq* queue, command* cmd){
    // create new command pointer
    // ncmd and copy values of cmd to it
    command* ncmd = (command*) malloc(sizeof(command));
    strcpy(ncmd->type, cmd->type);
    strcpy(ncmd->dir, cmd->dir);
    strcpy(ncmd->str, cmd->str);
    ncmd->next = NULL;

    // if queue is empty, assign
    // ncmd to head and tail
    // else assign ncmd to 
    // tail->next then tail
    if(queue->head == NULL){
        queue->head = ncmd;
        queue->tail = ncmd;
    } else{
        queue->tail->next = ncmd;
        queue->tail = ncmd;
    }
}

// pops the queue head from the command queue
command* dequeue(cmdq* queue){
    command *cmd = (command*) malloc(sizeof(command));
    if(queue->head != NULL){
        strcpy(cmd->type, queue->head->type);
        strcpy(cmd->dir, queue->head->dir);
        strcpy(cmd->str, queue->head->str);
    
        queue->head = queue->head->next;
    }
    
    return cmd;
}

// adds new file queue element into the file queue
void fenqueue(fileq* fq, fqe* fp){ 
    fqe* nf = (fqe*) malloc(sizeof(fqe));

    // copy contents of fp to new file
    strcpy(nf->file, fp->file);
    nf->cmdq = fp->cmdq;
    nf->lock = fp->lock;
    nf->next = NULL;

    // if file queue is empty, 
    // assign new file to head and tail
    // else assign new file 
    // to tail->next then tail
    if(fq->head == NULL){
        fq->head = nf;
        fq->tail = nf;
    } else{
        fq->tail->next = nf;
        fq->tail = nf;
    }
}


// *** C O M M A N D  F U N C T I O N S *** //

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
    
    // update cmd values
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
    FILE *cmddir = fopen(cmd->dir, "a");
    int i = 0;

    // append string to file character by character
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
        // log full command into read.txt
        fprintf(readtxt, "%s %s:\t", cmd->type, cmd->dir);

        // log file contents into read.txt
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
        // log full command into empty.txt
        fprintf(emptytxt, "%s %s:\t", cmd->type, cmd->dir);

        // log file contents into empty.txt
        while((rc = fgetc(cmddir)) != EOF) 
            fprintf(emptytxt, "%c", rc);
        fprintf(emptytxt, "\n");
        fclose(cmddir);

        // empty file contents
        cmddir = fopen(cmd->dir, "w");
        fclose(cmddir);

        // call sleep after operations
        sleep(rng(1));
    } else{
        fprintf(emptytxt, "%s %s:\tFILE ALREADY EMPTY\n", cmd->type, cmd->dir);
    }

    fclose(emptytxt);
}

void invalidcmd(){
    printf("Command not found\n");
}

// *** M I S C E L L A N E O U S *** // 
void initfiles(){
    FILE *initcmdtxt, *initreadtxt, *initemptytxt, *initdonetxt;

    // empty files before main execution of commands
    initcmdtxt = fopen("commands.txt", "w");
    initreadtxt = fopen("read.txt", "w");
    initemptytxt = fopen("empty.txt", "w");
    initdonetxt = fopen("done.txt", "w");

    fclose(initcmdtxt);
    fclose(initreadtxt);
    fclose(initemptytxt);
    fclose(initdonetxt);
}

// logs update onto filename
void updatefile(char* filename, command* cmd){    
    // get time of update
    time(&timestamp);
    char *t = ctime(&timestamp);
    // remove newline from time string
    t[strlen(t) - 1] = 0;
    
    FILE *stream = fopen(filename, "a");
    fprintf(stream, "[%s]\t%s %s %s\n", t, cmd->type, cmd->dir, cmd->str);
    fclose(stream);
}

// returns the random value for sleep()
int rng(int empty){
    srand(time(0));
    int t;

    if(empty) t = 7 + rand()%4;
    else{
        if(rand()%100 >= 80) t = 6;
        else t = 1;
    } 

	return t;
}