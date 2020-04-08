#include <stdio.h> 
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

#define MaxHost 16
#define MaxPlayer 16
#define MaxRand 65535

int way[16] = {0};
int randKey[MaxHost] = {0}, pid[MaxHost];
int usedHost[MaxHost] = {0};
int InFIFO, OutFIFO[MaxHost];
int score[MaxPlayer];
int hostNum, playerNum;

int charToInt(char a[], int *end);
int readOneInt();
void comb(int now,int a,int n,int m);
void runCompetition(char mes[]);
int processRootMes();
int getRand();
int getUsedHost();

int main(int argc, char *argv[]){
    srand(time(NULL));
    int end;
    hostNum = charToInt(argv[1], &end);
    playerNum = charToInt(argv[2], &end);

    getRand();

    mkfifo("Host.FIFO", 0644);
    InFIFO = open("Host.FIFO", O_RDWR);
    for (int i = 1; i <= hostNum; i++){
        char name[16];
        sprintf(name, "Host%d.FIFO", i);
        mkfifo(name, 0644);
        OutFIFO[i] = open(name, O_RDWR);
        pid[i] = fork();
        if (pid[i] == 0){
            for (int j = 1; j <= i; j++)
                close(OutFIFO[i]);
            close(InFIFO);

            char tmp[3][16];
            sprintf(tmp[0], "%d", i); sprintf(tmp[1], "%d", randKey[i]); sprintf(tmp[2], "%d", 0);
            execl("./host", "host", tmp[0], tmp[1], tmp[2], (char *) 0);
        }
    }

    comb(1, 1, playerNum, 8);

    while (getUsedHost()){
        processRootMes();
    }
    
    printf("HI\n");
    close(InFIFO);
    for (int i = 1; i <= hostNum; i++){
        close(OutFIFO[i]);
        char *mesg = "-1 -1 -1 -1 -1 -1 -1 -1\n";
        int status;
        write(OutFIFO[i], mesg, strlen(mesg));
        waitpid(pid[i], &status, 0);
    }
    remove("Host.FIFO");
    for (int i = 1; i <= hostNum; i++){
        char name[16];
        sprintf(name, "Host%d.FIFO", i);
        remove(name);
    }
    for (int i = 1; i <= playerNum; i++){
        printf("%d %d\n", i, score[i]);
    }
}

int charToInt(char a[], int *end){
    int r = 0, i = 0;
    while(a[i] >= '0' && a[i] <= '9'){
        r *= 10;
        r += a[i] - '0';
        i++;
    }
    *end = i;
    return r;
}

int readOneInt(){
    char b, r = 0;
    read(InFIFO, &b, 1);
    if (b == EOF) return -1;
    while (b < '0' || b > '9') {
        read(InFIFO, &b, 1);
        if (b == EOF) return -1;
    }
    while (b >= '0' && b <= '9'){
        r *= 10;
        r += b - '0';
        read(InFIFO, &b, 1);
    }
    return r;
}

int processRootMes(){
    int key;
    key = readOneInt();
    //printf("key %d\n", key);
    if (key == -1) return -1;
    for (int i = 0; i < 8; i++){
        int player, rank;
        player = readOneInt();
        rank = readOneInt();
        //printf("p: %d r %d\n", player, rank);
        score[player] += 8 - rank;
    }
    for (int i = 1; i <= hostNum; i++){
        if (key == randKey[i]){
            usedHost[i] = 0;
            return i;
        }
    }
}

void runCompetition(char mes[]){
    for (int i = 1; i <= hostNum; i++){
        if (usedHost[i] == 0){
            usedHost[i] = 1;
            write(OutFIFO[i], mes, strlen(mes));
            return;
        }
    }
    int hostID;
    hostID = processRootMes();
    write(OutFIFO[hostID], mes, strlen(mes));
    usedHost[hostID] = 1;
}

void comb (int now,int a,int n,int m){
    int b=a,c;
    if(now==m+1){
        char mes[32];
        sprintf(mes, "%d %d %d %d %d %d %d %d\n", way[1], way[2], way[3], way[4], way[5], way[6], way[7], way[8]);
        runCompetition(mes);
        return;
    }
    else
        for(b=a;b<=n;b++) {
            way[now]=b;
            comb(now+1,b+1,n,m);
        }
}

int getRand(){
    int in, im;
    im = 0;
    for (in = 0; in < MaxRand && im < MaxHost; ++in) {
    int rn = MaxRand - in;
    int rm = MaxHost - im;
    if (rand() % rn < rm)
        randKey[im++] = in + 1; /* +1 since your range begins from 1 */
    }
}

int getUsedHost(){
    for (int i = 1; i <= hostNum; i++)
        if (usedHost[i]) return i;
    return 0;
}