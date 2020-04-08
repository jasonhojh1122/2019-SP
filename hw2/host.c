#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <sys/wait.h>

int charToInt(char a[], int *end);
void findWinner(int *winnderID, int *winnerBid, char *mes1, char *mes2);
void getRank(int winsCount[], int rank[]);

int main(int argc, char *argv[]){
    int hostID, randomKey, depth;
    int end;
    hostID = charToInt(argv[1], &end);
    randomKey = charToInt(argv[2], &end);
    depth = charToInt(argv[3], &end);

    if (depth == 0){
        int player[8], InFIFO, OutFIFO;
        char fileName[16];
        
        sprintf(fileName, "Host%d.FIFO", hostID);
        InFIFO = open(fileName, O_RDWR);
        OutFIFO = open("Host.FIFO", O_RDWR);

        int fd1p2c[2], fd1c2p[2];
        pipe(fd1c2p); pipe(fd1p2c);
        int pid1;
        pid1 = fork();
        // first child
        if (pid1 == 0){
            dup2(fd1p2c[0], 0); dup2(fd1c2p[1], 1);
            close(fd1p2c[0]); close(fd1p2c[1]);
            close(fd1c2p[0]); close(fd1c2p[1]);
            close(OutFIFO); close(InFIFO);
            char tmp[3][16];
            sprintf(tmp[0], "%d", hostID); sprintf(tmp[1], "%d", randomKey); sprintf(tmp[2], "%d", 1);
            execl("./host", "host", tmp[0], tmp[1], tmp[2], (char *) 0);
        }
        else {
            int fd2p2c[2], fd2c2p[2];
            pipe(fd2c2p); pipe(fd2p2c);
            int pid2;
            pid2 = fork();
            // second child
            if (pid2 == 0) {
                close(fd1p2c[0]); close(fd1p2c[1]); close(fd1c2p[0]); close(fd1c2p[1]);

                dup2(fd2p2c[0], 0); dup2(fd2c2p[1], 1);
                close(fd2p2c[0]); close(fd2p2c[1]);
                close(fd2c2p[0]); close(fd2c2p[1]);
                close(OutFIFO); close(InFIFO);
                char tmp[3][16];
                sprintf(tmp[0], "%d", hostID); sprintf(tmp[1], "%d", randomKey); sprintf(tmp[2], "%d", 1);
                execl("./host", "host", tmp[0], tmp[1], tmp[2], (char *) 0);
            }
            else {
                close(fd1p2c[0]); // close parent to child 1 reading
                close(fd1c2p[1]); // close child 1 to parent writing
                close(fd2p2c[0]); // close parent to child 2 reading
                close(fd2c2p[1]); // close child 2 to parent writing

                char buf[64]; int len;
                while(len = read(InFIFO, buf, 64)){
                    buf[len] = 0;
                    int end = -1;
                    for (int i = 0; i < 8; i++){
                        int tmp = end+1;
                        player[i] = charToInt(&buf[end+1], &end);
                        end += tmp;
                    }
                    if (player[0] == -1) {
                        char *mes = "-1 -1 -1 -1\n";
                        int status;
                        write(fd1p2c[1], mes, strlen(mes));
                        write(fd2p2c[1], mes, strlen(mes));
                        waitpid(pid1, &status, 0);
                        waitpid(pid2, &status, 0);
                        close(fd1c2p[0]);
                        close(fd2c2p[0]);
                        close(fd1p2c[1]);
                        close(fd2p2c[1]);
                        _exit(0);
                    }
                    
                    int len1, len2, childWin, childBid, winsCount[8], playerIndex[16], rank[8];
                    char mes1[32], mes2[32];
                    for (int i = 0; i < 8; i++) {
                        winsCount[i] = 0;
                        playerIndex[player[i]] = i;
                    }

                    // tell childs the player ids
                    sprintf(mes1, "%d %d %d %d\n", player[0], player[1], player[2], player[3]);
                    sprintf(mes2, "%d %d %d %d\n", player[4], player[5], player[6], player[7]);
                    write(fd1p2c[1], mes1, strlen(mes1));
                    write(fd2p2c[1], mes2, strlen(mes2));

                    len1 = read(fd1c2p[0], mes1, 32);
                    len2 = read(fd2c2p[0], mes2, 32);
                    mes1[len1] = 0;
                    mes2[len2] = 0;
                    findWinner(&childWin, &childBid, mes1, mes2);
                    winsCount[playerIndex[childWin]]++;

                    for(int j = 1; j <= 9; j++){
                        char tmp[5];
                        sprintf(tmp, "%d", childWin);
                        if (write(fd1p2c[1], tmp, strlen(tmp)) < 0) {char mes[16]; sprintf(mes, "ERROR1 0, %d", j); write(fd2p2c[1], mes, strlen(mes));}
                        if (write(fd2p2c[1], tmp, strlen(tmp)) < 0) {char mes[16]; sprintf(mes, "ERROR2 0, %d", j); write(fd2p2c[1], mes, strlen(mes));}
                        len1 = read(fd1c2p[0], mes1, 32);
                        len2 = read(fd2c2p[0], mes2, 32);
                        mes1[len1] = 0;
                        mes2[len2] = 0;
                        findWinner(&childWin, &childBid, mes1, mes2);
                        winsCount[playerIndex[childWin]]++;
                    }
                    getRank(winsCount, rank);
                    sprintf(buf, "%d\n%d %d\n%d %d\n%d %d\n%d %d\n%d %d\n%d %d\n%d %d\n%d %d\n", randomKey,player[0],rank[0],player[1],rank[1],player[2],rank[2],player[3],rank[3],player[4],rank[4],player[5],rank[5],player[6],rank[6],player[7],rank[7]);
                    write(OutFIFO, buf, strlen(buf));
                }
            }
        }
    }
    else if (depth == 1){
        int fd1p2c[2], fd1c2p[2];
        pipe(fd1c2p); pipe(fd1p2c);
        int pid1;
        pid1 = fork();
        // first child
        if (pid1 == 0){
            dup2(fd1p2c[0], 0); dup2(fd1c2p[1], 1);
            close(fd1p2c[0]); close(fd1p2c[1]);
            close(fd1c2p[0]); close(fd1c2p[1]);
            char tmp[3][16];
            sprintf(tmp[0], "%d", hostID); sprintf(tmp[1], "%d", randomKey); sprintf(tmp[2], "%d", 2);
            execl("./host", "host", tmp[0], tmp[1], tmp[2], (char *) 0);
        }
        else {
            int fd2p2c[2], fd2c2p[2];
            pipe(fd2c2p); pipe(fd2p2c);
            int pid2;
            pid2 = fork();
            // second child
            if (pid2 == 0) {
                close(fd1p2c[0]); close(fd1p2c[1]); close(fd1c2p[0]); close(fd1c2p[1]);

                dup2(fd2p2c[0], 0); dup2(fd2c2p[1], 1);
                close(fd2p2c[0]); close(fd2p2c[1]);
                close(fd2c2p[0]); close(fd2c2p[1]);
                char tmp[3][16];
                sprintf(tmp[0], "%d", hostID); sprintf(tmp[1], "%d", randomKey); sprintf(tmp[2], "%d", 2);
                execl("./host", "host", tmp[0], tmp[1], tmp[2], (char *) 0);
            }
            else {
                close(fd1p2c[0]); // close parent to child 1 reading
                close(fd1c2p[1]); // close child 1 to parent writing
                close(fd2p2c[0]); // close parent to child 2 reading
                close(fd2c2p[1]); // close child 2 to parent writing

                int player[4];
                while(scanf("%d%d%d%d", &player[0], &player[1], &player[2], &player[3])){
                    if (player[0] == -1){
                        char *mes = "-1 -1\n";
                        int status;
                        write(fd1p2c[1], mes, strlen(mes));
                        write(fd2p2c[1], mes, strlen(mes));
                        waitpid(pid1, &status, 0);
                        waitpid(pid2, &status, 0);
                        close(fd1c2p[0]);
                        close(fd2c2p[0]);
                        close(fd1p2c[1]);
                        close(fd2p2c[1]);
                        _exit(0);
                    }
                    int len1, len2, childWin, childBid;
                    char mes1[32], mes2[32];
                    // tell child the player ids
                    sprintf(mes1, "%d %d\n", player[0], player[1]);
                    sprintf(mes2, "%d %d\n", player[2], player[3]);
                    write(fd1p2c[1], mes1, strlen(mes1));
                    write(fd2p2c[1], mes2, strlen(mes2));

                    len1 = read(fd1c2p[0], mes1, 32);
                    len2 = read(fd2c2p[0], mes2, 32);
                    mes1[len1] = 0;
                    mes2[len2] = 0;
                    findWinner(&childWin, &childBid, mes1, mes2);
                    printf("%d %d\n", childWin, childBid);
                    fflush(stdout);

                    int winnerID;
                    char buf[32];
                    for(int j = 1; j <= 9; j++){
                        read(0, buf, 32);
                        winnerID = charToInt(buf, &end);
                        char tmp[5];
                        sprintf(tmp, "%d", winnerID);
                        if (write(fd1p2c[1], tmp, strlen(tmp)) < 0) {char mes[16]; sprintf(mes, "ERROR1 1, %d", j); write(fd2p2c[1], mes, strlen(mes));}
                        if (write(fd2p2c[1], tmp, strlen(tmp)) < 0) {char mes[16]; sprintf(mes, "ERROR2 1, %d", j); write(fd2p2c[1], mes, strlen(mes));}
                        len1 = read(fd1c2p[0], mes1, 32);
                        len2 = read(fd2c2p[0], mes2, 32);
                        mes1[len1] = 0;
                        mes2[len2] = 0;
                        findWinner(&childWin, &childBid, mes1, mes2);
                        printf("%d %d\n", childWin, childBid);
                        fflush(stdout);
                    }
                }
            }
        }
    }
    else {
        int player[2];
        while (scanf("%d%d", &player[0], &player[1])){
            if (player[0] == -1)
                _exit(0);
            
            int fd1p2c[2], fd1c2p[2];
            pipe(fd1c2p);
            pipe(fd1p2c);
            int pid1;
            pid1 = fork();
            // first child
            if (pid1 == 0){
                dup2(fd1p2c[0], 0); dup2(fd1c2p[1], 1);
                close(fd1c2p[0]); close(fd1c2p[1]);
                close(fd1p2c[0]); close(fd1p2c[1]);
                char tmp[5];
                sprintf(tmp, "%d", player[0]);
                execl("./player", "player", tmp, (char *) 0);
            }
            else {
                int fd2p2c[2], fd2c2p[2], pid2;
                pipe(fd2c2p);
                pipe(fd2p2c);
                // second child
                pid2 = fork();
                if (pid2 == 0) {
                    close(fd1p2c[0]); close(fd1p2c[1]); close(fd1c2p[0]); close(fd1c2p[1]);

                    dup2(fd2p2c[0], 0); dup2(fd2c2p[1], 1);
                    close(fd2p2c[0]); close(fd2p2c[1]);
                    close(fd2c2p[0]); close(fd2c2p[1]);
                    char tmp[5];
                    sprintf(tmp, "%d", player[1]);
                    execl("./player", "player", tmp, (char *) 0);
                }
                else {
                    close(fd1p2c[0]); // close parent to child 1 reading
                    close(fd1c2p[1]); // close child 1 to parent writing
                    close(fd2p2c[0]); // close parent to child 2 reading
                    close(fd2c2p[1]); // close child 2 to parent writing
                    
                    int len1, len2, childWin, childBid;
                    char mes1[32], mes2[32];
                    len1 = read(fd1c2p[0], mes1, 32);
                    len2 = read(fd2c2p[0], mes2, 32);
                    mes1[len1] = 0;
                    mes2[len2] = 0;
                    findWinner(&childWin, &childBid, mes1, mes2);
                    printf("%d %d\n", childWin, childBid);
                    fflush(stdout);

                    int winnerID;
                    char buf[32];
                    for(int j = 1; j <= 9; j++){
                        read(0, buf, 32);
                        winnerID = charToInt(buf, &end);
                        char tmp[5];
                        sprintf(tmp, "%d", winnerID);
                        if (write(fd1p2c[1], tmp, strlen(tmp)) < 0) {char mes[16]; sprintf(mes, "ERROR1 2, %d", j); write(fd2p2c[1], mes, strlen(mes));}
                        if (write(fd2p2c[1], tmp, strlen(tmp)) < 0) {char mes[16]; sprintf(mes, "ERROR2 2, %d", j); write(fd2p2c[1], mes, strlen(mes));}
                        len1 = read(fd1c2p[0], mes1, 32);
                        len2 = read(fd2c2p[0], mes2, 32);
                        mes1[len1] = 0;
                        mes2[len2] = 0;
                        findWinner(&childWin, &childBid, mes1, mes2);
                        printf("%d %d\n", childWin, childBid);
                        fflush(stdout);
                    }
                    int status;
                    waitpid(pid1, &status, 0);
                    waitpid(pid2, &status, 0);
                    close(fd1c2p[0]);
                    close(fd2c2p[0]);
                    close(fd1p2c[1]);
                    close(fd2p2c[1]);
                }
            }      
        }
    }
}

int charToInt(char a[], int *end){
    if (a[0] == '-') return -1;
    int r = 0, i = 0;
    while(a[i] >= '0' && a[i] <= '9'){
        r *= 10;
        r += a[i] - '0';
        i++;
    }
    *end = i;
    return r;
}

void findWinner(int *winnderID, int *winnerBid, char *mes1, char *mes2){
    int id1 = 0, bid1 = 0, id2 = 0, bid2 = 0;
    int end = 0;
    id1 = charToInt(mes1, &end);
    bid1 = charToInt(&mes1[end+1], &end);
    id2 = charToInt(mes2, &end);
    bid2 = charToInt(&mes2[end+1], &end);
    if (bid1 > bid2){
        *winnderID = id1;
        *winnerBid = bid1;
    }
    else {
        *winnderID = id2;
        *winnerBid = bid2;
    }
    return;
}

void getRank(int winsCount[], int rank[]){
    int order[8];
    for (int i = 0; i < 8; i++) order[i] = i;

    for (int i = 0; i < 7; i++){
        for (int j = i; j < 8; j++){
            if (winsCount[j] > winsCount[i]){
                int tmp = winsCount[j];
                winsCount[j] = winsCount[i];
                winsCount[i] = tmp;
                tmp = order[j];
                order[j] = order[i];
                order[i] = tmp;
            }
        }
    }
    int tmp[8], same = 0, pre = -1;
    tmp[0] = 1;
    for (int i = 1; i < 8; i++){
        if (winsCount[i] == pre){
            same++;
            tmp[i] = tmp[i-1];
        }
        else{
            pre = winsCount[i];
            tmp[i] = tmp[i-1] + 1 + same;
            same = 0;
        }
    }
    for (int i = 0; i < 8; i++) rank[order[i]] = tmp[i];
}
