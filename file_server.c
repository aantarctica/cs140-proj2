#include "functions.c"

int main(){
    // INITIALIZATIONS // 
    fileq *fq = initfq();
    char buffer[150];
    initfiles();
    while(1){
        // get user input and store to buffer
        memset(buffer, '\0', 150);
        fgets(buffer, 150, stdin);         

        // filter out empty inputs
        if(strcmp(buffer, "\n") == 0) continue;

        // parse buffer into command
        command* cmd = parsecmd(buffer);
        
        // update commands.txt
        updatefile("commands.txt", cmd);

        // add command to file command queue
        fqe* file = searchfqe(fq, cmd);
        enqueue(file->cmdq, cmd);                               

        // send command to worker thread
        pthread_t workerthr;
        pthread_create(&workerthr, NULL, (void *) worker, file);
        pthread_detach(workerthr);
    }

    return 0;
}
