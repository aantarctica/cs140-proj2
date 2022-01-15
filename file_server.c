#include "file_server_include.c"

int main(){
    // INITIALIZATIONS // 
    fileq *fq = initfq();
    char buffer[150];
    initfiles();
    while(1){
        // gets user input and stores it to buffer
        memset(buffer, '\0', 150);
        fgets(buffer, 150, stdin);         

        // filters out empty inputs
        if(strcmp(buffer, "\n") == 0){
            continue;
        }

        command* cmd = parsecmd(buffer);
        
        // update commands.txt
        updatefile("commands.txt", cmd);

        fqe* file = searchfqe(fq, cmd);
        enqueue(file->cmdq, cmd);                               

        // send command to worker thread
        pthread_t workerthr;
        pthread_create(&workerthr, NULL, (void *) worker, file);
        pthread_detach(workerthr);
    }
    // traverseFILES(args->fq);

    return 0;
}


/*
Level 4 Implementation
1. make structs
2. search file queue
3. put command in file queue element
4. checking occurs after command is popped from queue

--- debugging notes ---
check initializations of fq and fqe

-- finalizing --
check newline printing write

*/