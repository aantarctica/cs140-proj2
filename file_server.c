#include "file_server_include.c"

int main(){
    // VARIABLE INITIALIZATIONS // 
    wargs *args = initargs();
    char buffer[150];

    // FILE INITIALIZATIONS //
    initfiles();
    FILE *cmdtxt;

    while(1){
        cmdtxt = fopen("commands.txt", "a");

        // gets user input and stores it to buffer
        memset(buffer, '\0', 150);
        fgets(buffer, 150, stdin);         

        // filters out empty inputs
        if((strcmp(buffer, "\n")) != 0){        
            command* cmd = parsecmd(buffer);    
            pthread_t workerthr;
            
            // update commands.txt
            updatefile("commands.txt", cmd);
            
            // update queues
            enqueue(args->queue, cmd);

            // push command to file command queue
            if(cmd->dir != NULL){
                fqe* file = searchfqe(args->fq, cmd);
                enqueue(file->cmdq, cmd);                               
            }

            // send command to worker thread
            pthread_create(&workerthr, NULL, (void *) worker, args);
            pthread_detach(workerthr);
        }

        fclose(cmdtxt);
        // if(strcmp(buffer, "exit\n") == 0) break;
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