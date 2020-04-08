#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <sys/wait.h>

#define M 10
#define N 65535

int readOneInt(){
    char b, r = 0;
    read(0, &b, 1);
    if (b == EOF) return -1;
    while (b < '0' || b > '9') {
        read(0, &b, 1);
        if (b == EOF) return -1;
    }
    while (b >= '0' && b <= '9'){
        r *= 10;
        r += b - '0';
        read(0, &b, 1);
    }
    return r;
}

int main(){
    while (1){
    int a = readOneInt();
    printf("%d\n", a);    
    }
    
}