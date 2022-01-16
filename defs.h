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

// *** F U N C T I O N  I N I T I A L I Z A T I O N S *** //

void worker(fqe* file);

fqe*        searchfqe(fileq* fq, command* cmd);
fqe*        createfqe(fileq* fq, command* cmd);
void        enqueue(cmdq* queue, command* cmd);
command*    dequeue(cmdq* queue);
void        fenqueue(fileq* fq, fqe* fp);
cmdq*       initq();
fileq*      initfq();

command*    parsecmd(char buffer[150]);
void        writecmd(command* cmd);
void        readcmd(command* cmd);
void        emptycmd(command* cmd);
void        invalidcmd();

void initfiles();
void updatefile(char* filename, command* cmd);
int rng(int empty);

// *** G L O B A L  V A R I A B L E (S) *** // 
time_t timestamp;


