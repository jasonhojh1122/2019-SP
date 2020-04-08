#include <unistd.h>
#include <stdio.h>

int charToInt(char a[]){
    int r = 0, i = 0;
    while(a[i] >= '0' && a[i] <= '9'){
        r *= 10;
        r += a[i] - '0';
        i++;
    }
    return r;
}

int main(int argc, char *argv[]){
    int playerID, winnerID;
    playerID = charToInt(argv[1]);
    printf("%d %d\n", playerID, playerID*100);
    fflush(stdout);

    char buf[32];
    for(int j = 1; j <= 9; j++){
        read(0, buf, 32);
        printf("%d %d\n", playerID, playerID*100);
        fflush(stdout);
    }
    _exit(0);
}