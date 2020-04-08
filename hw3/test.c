#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int queue[10];

void push(int n){
    int zero;
    for (int i = 0; i < 10; ++i){
        if (queue[i] == n) return;
        if (queue[i] == 0) zero = i;
    }
    queue[zero] = n;
}
void pop(int n){
    for (int i = 0; i < 10; ++i){
        if (queue[i] == n) queue[i] = 0;
    }
}

void printQueue(){
    for (int i = 0; i < 9; ++i){
        for (int j = i+1; j < 10; ++j){
            if (queue[i] > queue[j]){
                int tmp = queue[i];
                queue[i] = queue[j];
                queue[j] = tmp;
            }
        }
    }
    for (int i = 0; i < 10; ++i){
        if (queue[i] == 0) continue;
        printf("%d ", queue[i]);
    }
    printf("\n");
}

int main(){
    int n;
    while (scanf("%d",&n) != -1){
        if (n == 0) printQueue();
        else if (n == 1){
            scanf("%d", &n);
            pop(n);
        }
        else {
            push(n);
        }
    }

}