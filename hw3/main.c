#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h> 
#include<sys/wait.h> 

#define SIGUSR3 SIGWINCH

int P, Q, R, sig[10], status;
int fd[2];

void readString(char buf[]);

int main(int argc, char *argv[]){
    pid_t child;
    scanf("%d%d%d", &P, &Q, &R);
    for (int i = 0; i < R; i++)
        scanf("%d", &sig[i]);

    pipe(fd);
    child = fork();
    if (child == 0){
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);
        char tmp[4][3];
        sprintf(tmp[0], "%d", P); sprintf(tmp[1], "%d", Q); sprintf(tmp[2], "3"); sprintf(tmp[3], "0");
        execl("./hw3", "hw3", tmp[0], tmp[1], tmp[2], tmp[3] , (char *) 0);
    }
    else {
        close(fd[1]);
        for (int i = 0; i < R; i++){
            char buf[128];
            for (int i = 0; i < 128; i++) buf[i] = 0;
            sleep(5);

            switch(sig[i]){
                case 1:
                    kill(child, SIGUSR1);
                    readString(buf);
                    break;

                case 2:
                    kill(child, SIGUSR2);
                    readString(buf);
                    break;

                case 3:
                    kill(child, SIGUSR3);
                    readString(buf);
                    printf("%s", buf);
                    break;
            }
            //printf("[parent start]\n");
            // printf("%s", buf);
            //printf("[parent end]\n");
        }
        char buf[1000];
        for (int i = 0; i < 1000; i++) buf[i] = 0;
        readString(buf);
        printf("%s", buf);
        fflush(stdout);
        waitpid(child, &status, 0);
        close(fd[0]);
        return 0;
    }
}

void readString(char buf[]){
    int id = 0;
    while (1){
        read(fd[0], &buf[id], sizeof(char));
        if (buf[id] == '\n' || buf[id] == EOF || buf[id] == '\0') break;
        id++;
    }
}