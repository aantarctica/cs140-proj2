#include "file_server_include.c"

int main(){
    // VARIABLE INITIALIZATIONS // 
    wargs *args = initargs();

    char buffer[150];
    time_t timestamp;

    // FILE INITIALIZATIONS //
    initfiles();
    FILE *cmdtxt;
    
    while(1){
        cmdtxt = fopen("commands.txt", "a");

        // gets user input and stores it to buffer
        memset(buffer, '\0', 150);
        fgets(buffer, 150, stdin); 
        time(&timestamp);

        // filters out empty inputs
        if((strcmp(buffer, "\n")) != 0){        
            command* cmd = parsecmd(buffer);    
            pthread_t workerthr;
            
            // update commands.txt
            fprintf(cmdtxt, "%s %s", ctime(&timestamp), buffer);
            
            // update queues
            enqueue(args->queue, cmd);

            // push command to file command queue
            if(cmd->dir != NULL){
                fqe* file = searchfile(args->fq, cmd);
                enqueue(file->cmdq, cmd);                
            }

            // send command to worker thread
            pthread_create(&workerthr, NULL, (void *) worker, args);
            pthread_detach(workerthr);
        }

        fclose(cmdtxt);
    }
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

*/