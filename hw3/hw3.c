#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <setjmp.h>
#include <signal.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include "scheduler.h"

#define SIGUSR3 SIGWINCH

int mutex = 0;
int idx, P, Q, T, B;
char arr[10000];
char msg[128];
char ACK[5] = "ACK\n";
char *Unblock = "[DEBUG] Mask unblock\n";
char *msgRec1 = "[DEBUG] Receive sigusr1\n";
char *msgRec2 = "[DEBUG] Receive sigusr2\n";
char *msgRec3 = "[DEBUG] Receive sigusr3\n";
char *msgSig1 = "[DEBUG] In sigusr1\n";
char *msgSig2 = "[DEBUG] In sigusr2\n";
char *msgSig3 = "[DEBUG] In sigusr3\n";

jmp_buf MAIN, SCHEDULER;
FCB_ptr Current, Head;
FCB fcb[5];

sigset_t blockMask, pendingMask, oldMask, emptyMask;

int queue[5];

void push(int n);
void pop(int n);

void sigusr1(int signo);
void sigusr2(int signo);
void sigusr3(int signo);

void funct3_1(int name);
void funct3_2(int name);
void funct3_3(int name);
void funct3_4(int name);

int charToInt(char *str);

int main(int argc, char *argv[]){
    P = charToInt(argv[1]); Q = charToInt(argv[2]); T = charToInt(argv[3]); B = charToInt(argv[4]);

    fcb[0].Next = &fcb[1];
    Current = &fcb[0];
    Head = &fcb[1];

    switch(T){
        case 1:
        case 2:
            if (setjmp(MAIN) == 0)
                funct_5(0);
            Scheduler();
            break;

        case 3:
            sigemptyset(&blockMask); sigemptyset(&pendingMask); sigemptyset(&emptyMask);
            sigaddset(&blockMask, SIGUSR1); sigaddset(&blockMask, SIGUSR2); sigaddset(&blockMask, SIGUSR3);

            sigprocmask(SIG_BLOCK, &blockMask, &oldMask);

            struct sigaction act1, act2, act3;
            act1.sa_handler = sigusr1; act1.sa_mask = blockMask;
            act2.sa_handler = sigusr2; act2.sa_mask = blockMask;
            act3.sa_handler = sigusr3; act3.sa_mask = blockMask;
            sigaction(SIGUSR1, &act1, NULL); sigaction(SIGUSR2, &act2, NULL); sigaction(SIGUSR3, &act3, NULL);
            // signal(SIGUSR1, sigusr1); signal(SIGUSR2, sigusr2); signal(SIGUSR3, sigusr3);
            
            if (setjmp(MAIN) == 0)
                funct_5(0);
            Scheduler();
            break;
    }
    return 0;
}

void funct_1(int name){
    int i, j;
    if (setjmp(fcb[name].Environment) == 0){
        int nameNext = ((name + 1) > 4) ? 1 : name + 1;
        int namePre  = ((name - 1) < 1) ? 4 : name - 1;       
        fcb[name].Name      = name;
        fcb[name].Next      = &fcb[nameNext];
        fcb[name].Previous  = &fcb[namePre];
        if (name == 4) longjmp(MAIN, 1);
        else funct_5(name);
    }
    for (i = 0; i < P; i++){
        for (j = 0; j < Q; j++){
            sleep(1);
            arr[idx++] = name + '0';
        }
    }
    longjmp(SCHEDULER, -2);
}

void funct_2(int name){
    int i = 0, j, c = 0;
    if (setjmp(fcb[name].Environment) == 0){
        int nameNext = ((name + 1) > 4) ? 1 : name + 1;
        int namePre  = ((name - 1) < 1) ? 4 : name - 1;
        fcb[name].Name      = name;
        fcb[name].Next      = &fcb[nameNext];
        fcb[name].Previous  = &fcb[namePre];
        if (name == 4) longjmp(MAIN, 1);
        else funct_5(name);
    }
    if (mutex != 0){
        if (setjmp(fcb[name].Environment) == 0){
            longjmp(SCHEDULER, 1);
        }
    }
    mutex = name;
    for (; i < P; i++){
        if (c == B){
            c = 0;
            if (setjmp(fcb[name].Environment) == 0){
                mutex = 0;
                longjmp(SCHEDULER, 1);
            } else {
                mutex = name;
            }
        }
        c++;
        for (j = 0; j < Q; j++){
            sleep(1);
            arr[idx++] = name + '0';
        }
    }
    longjmp(SCHEDULER, -2);
}

void funct_3(int name){
    if (setjmp(fcb[name].Environment) == 0){
        int nameNext = ((name + 1) > 4) ? 1 : name + 1;
        int namePre  = ((name - 1) < 1) ? 4 : name - 1;
        fcb[name].Name      = name;
        fcb[name].Next      = &fcb[nameNext];
        fcb[name].Previous  = &fcb[namePre];
        if (name == 4) longjmp(MAIN, 1);
        else funct_5(name);
    }
    while (mutex != 0 && mutex != name){
        push(name);
        if (setjmp(fcb[name].Environment) == 0){
            longjmp(SCHEDULER, 1);
        }
    }
    mutex = name;
    pop(name);
    for (int i = 0; i < P; i++){
        while(mutex != 0 && mutex != name){
            push(name);
            if (setjmp(fcb[name].Environment) == 0)
                longjmp(SCHEDULER, 1);
        }
        mutex = name;
        pop(name);

        for (int j = 0; j < Q; j++){
            sleep(1);
            arr[idx++] = name + '0';
        }
        int pend = 0;
        sigemptyset(&pendingMask);
        sigpending(&pendingMask);
        if (sigismember(&pendingMask, SIGUSR1)) {
            #ifdef DEBUG
            write(2, msgRec1, sizeof(char) * strlen(msgRec1));
            #endif
            pend = 1;
        }
        else if (sigismember(&pendingMask, SIGUSR2)){
            #ifdef DEBUG
            write(2, msgRec2, sizeof(char) * strlen(msgRec2));
            #endif
            mutex = 0;
            pend = 1;
        }
        else if (sigismember(&pendingMask, SIGUSR3)){
            #ifdef DEBUG
            write(2, msgRec3, sizeof(char) * strlen(msgRec3));
            #endif
            pend = 1;
        }
        if (pend == 1){
            #ifdef DEBUG
            write(2, Unblock, sizeof(char) * strlen(Unblock));
            #endif
            if (setjmp(fcb[name].Environment) == 0){
                sigprocmask(SIG_SETMASK, &emptyMask, NULL);
            }
        }
    }
    
    mutex = 0;
    longjmp(SCHEDULER, -2);
}

void funct_3_2(int name){
    if (setjmp(fcb[name].Environment) == 0){
        int nameNext = ((name + 1) > 4) ? 1 : name + 1;
        int namePre  = ((name - 1) < 1) ? 4 : name - 1;
        fcb[name].Name      = name;
        fcb[name].Next      = &fcb[nameNext];
        fcb[name].Previous  = &fcb[namePre];
        if (name == 4) longjmp(MAIN, 1);
        else funct_5(name);
    }
    
    while (mutex != 0 && mutex != name){
        push(name);
        if (setjmp(fcb[name].Environment) == 0){
            longjmp(SCHEDULER, 1);
        }
    }
    mutex = name;
    pop(name);
    for (int i = 0; i < P; i++){
        while(mutex != 0 && mutex != name){
            push(name);
            if (setjmp(fcb[name].Environment) == 0)
                longjmp(SCHEDULER, 1);
        }
        mutex = name;
        pop(name);

        for (int j = 0; j < Q; j++){
            sleep(1);
            arr[idx++] = name + '0';
        }
        int pend = 0;
        sigemptyset(&pendingMask);
        sigpending(&pendingMask);
        if (sigismember(&pendingMask, SIGUSR1)) {
            #ifdef DEBUG
            write(2, msgRec1, sizeof(char) * strlen(msgRec1)); fsync(2);
            #endif
            pend = 1;
        }
        else if (sigismember(&pendingMask, SIGUSR2)){
            #ifdef DEBUG
            write(2, msgRec2, sizeof(char) * strlen(msgRec2)); fsync(2);
            #endif
            mutex = 0;
            pend = 1;
        }
        else if (sigismember(&pendingMask, SIGUSR3)){
            #ifdef DEBUG
            write(2, msgRec3, sizeof(char) * strlen(msgRec3)); fsync(2);
            #endif
            pend = 1;
        }
        if (pend == 1){
            #ifdef DEBUG
            write(2, Unblock, sizeof(char) * strlen(Unblock));
            #endif
            if (setjmp(fcb[name].Environment) == 0){
                sigprocmask(SIG_SETMASK, &emptyMask, NULL);
            }
        }
    }
    
    mutex = 0;
    longjmp(SCHEDULER, -2);
}

void funct_3_3(int name){
    if (setjmp(fcb[name].Environment) == 0){
        int nameNext = ((name + 1) > 4) ? 1 : name + 1;
        int namePre  = ((name - 1) < 1) ? 4 : name - 1;
        fcb[name].Name      = name;
        fcb[name].Next      = &fcb[nameNext];
        fcb[name].Previous  = &fcb[namePre];
        if (name == 4) longjmp(MAIN, 1);
        else funct_5(name);
    }
    
    while (mutex != 0 && mutex != name){
        push(name);
        if (setjmp(fcb[name].Environment) == 0){
            longjmp(SCHEDULER, 1);
        }
    }
    mutex = name;
    pop(name);
    for (int i = 0; i < P; i++){
        while(mutex != 0 && mutex != name){
            push(name);
            if (setjmp(fcb[name].Environment) == 0)
                longjmp(SCHEDULER, 1);
        }
        mutex = name;
        pop(name);

        for (int j = 0; j < Q; j++){
            sleep(1);
            arr[idx++] = name + '0';
        }
        int pend = 0;
        sigemptyset(&pendingMask);
        sigpending(&pendingMask);
        if (sigismember(&pendingMask, SIGUSR1)) {
            #ifdef DEBUG
            write(2, msgRec1, sizeof(char) * strlen(msgRec1)); fsync(2);
            #endif
            pend = 1;
        }
        else if (sigismember(&pendingMask, SIGUSR2)){
            #ifdef DEBUG
            write(2, msgRec2, sizeof(char) * strlen(msgRec2)); fsync(2);
            #endif
            mutex = 0;
            pend = 1;
        }
        else if (sigismember(&pendingMask, SIGUSR3)){
            #ifdef DEBUG
            write(2, msgRec3, sizeof(char) * strlen(msgRec3)); fsync(2);
            #endif
            pend = 1;
        }
        if (pend == 1){
            #ifdef DEBUG
            write(2, Unblock, sizeof(char) * strlen(Unblock));
            #endif
            if (setjmp(fcb[name].Environment) == 0){
                sigprocmask(SIG_SETMASK, &emptyMask, NULL);
            }
        }
    }
    
    mutex = 0;
    longjmp(SCHEDULER, -2);
}

void funct_3_4(int name){
    if (setjmp(fcb[name].Environment) == 0){
        int nameNext = ((name + 1) > 4) ? 1 : name + 1;
        int namePre  = ((name - 1) < 1) ? 4 : name - 1;
        fcb[name].Name      = name;
        fcb[name].Next      = &fcb[nameNext];
        fcb[name].Previous  = &fcb[namePre];
        if (name == 4) longjmp(MAIN, 1);
        else funct_5(name);
    }
    
    while (mutex != 0 && mutex != name){
        push(name);
        if (setjmp(fcb[name].Environment) == 0){
            longjmp(SCHEDULER, 1);
        }
    }
    mutex = name;
    pop(name);
    for (int i = 0; i < P; i++){
        while(mutex != 0 && mutex != name){
            push(name);
            if (setjmp(fcb[name].Environment) == 0)
                longjmp(SCHEDULER, 1);
        }
        mutex = name;
        pop(name);

        for (int j = 0; j < Q; j++){
            sleep(1);
            arr[idx++] = name + '0';
        }
        int pend = 0;
        sigemptyset(&pendingMask);
        sigpending(&pendingMask);
        if (sigismember(&pendingMask, SIGUSR1)) {
            #ifdef DEBUG
            write(2, msgRec1, sizeof(char) * strlen(msgRec1)); fsync(2);
            #endif
            pend = 1;
        }
        else if (sigismember(&pendingMask, SIGUSR2)){
            #ifdef DEBUG
            write(2, msgRec2, sizeof(char) * strlen(msgRec2)); fsync(2);
            #endif
            mutex = 0;
            pend = 1;
        }
        else if (sigismember(&pendingMask, SIGUSR3)){
            #ifdef DEBUG
            write(2, msgRec3, sizeof(char) * strlen(msgRec3)); fsync(2);
            #endif
            pend = 1;
        }
        if (pend == 1){
            #ifdef DEBUG
            write(2, Unblock, sizeof(char) * strlen(Unblock));
            #endif
            if (setjmp(fcb[name].Environment) == 0){
                sigprocmask(SIG_SETMASK, &emptyMask, NULL);
            }
        }
    }
    
    mutex = 0;
    longjmp(SCHEDULER, -2);
}

void funct_4(int name){
    //
}

void funct_5(int name){ //dummy function
    int a[10000];
    if (T == 1) funct_1(name+1);
    else if (T == 2) funct_2(name+1);
    else if (T == 3) funct_3(name+1);/*
    else if (T == 3) {
        switch(name){
            case 0:
                funct_3_1(name+1);
                break;
            case 1:
                funct_3_2(name+1);
                break;
            case 2:
                funct_3_3(name+1);
                break;
            case 3:
                funct_3_4(name+1);
                break;
        }
    }*/
}

void sigusr1(int signo){

    #ifdef DEBUG
    write(2, msgSig1, sizeof(char) * strlen(msgSig1)); fsync(2);
    #endif

    write(1, ACK, sizeof(char) * 4);
    fsync(1);
    
    sigprocmask(SIG_BLOCK, &blockMask, NULL);
    longjmp(SCHEDULER, 1);
}

void sigusr2(int signo){

    #ifdef DEBUG
    write(2, msgSig2, sizeof(char) * strlen(msgSig2)); fsync(2);
    #endif

    write(1, ACK, sizeof(char) * 4);
    fsync(1);
    
    sigprocmask(SIG_BLOCK, &blockMask, NULL);
    longjmp(SCHEDULER, 1);
}

void sigusr3(int signo){
    
    #ifdef DEBUG
    write(2, msgSig3, sizeof(char) * strlen(msgSig3)); fsync(2);
    #endif

    int cnt = 0;
    for (int i = 1; i <= 4; i++){
        if (queue[i] == 1){
            msg[cnt] = '0'+i;
            msg[cnt+1] = ' ';
            cnt += 2;
        }
    }
    msg[cnt] = '\n';
    msg[cnt+1] = '\0';
    for (int i = 0; i < strlen(msg)-1; i++){
        assert(msg[i] != '\n');
    }
    write(1, msg, sizeof(char) * strlen(msg));
    fsync(1);
    
    fcb[0].Next = Current;
    Current = &fcb[0];
    sigprocmask(SIG_BLOCK, &blockMask, NULL);
    longjmp(SCHEDULER, 1);
}

int charToInt(char *str){
    int ret = 0, i = 0;
    while (str[i] >= '0' && str[i] <= '9'){
        ret *= 10;
        ret += str[i] - '0';
        i++;
    }
    return ret;
}

void push(int n){
    if (queue[n] == 0) queue[n] = 1;
}
void pop(int n){
    if (queue[n] == 1) queue[n] = 0;
}