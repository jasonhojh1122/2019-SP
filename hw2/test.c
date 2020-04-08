#include<stdio.h>
#include<stdlib.h>
int way[16]={0};
int n,m,count = 0;
void make (int now,int a,int n,int m){
    int b=a,c;
    if(now==m+1){
        count++;
        for(c=1;c<=m;c++)
            printf("%d ",way[c]);
        printf("\n");
        return;
    }
    else
        for(b=a;b<=n;b++) {
            way[now]=b;
            make(now+1,b+1,n,m);
        }
}
int main(){
    while(scanf("%d %d",&n,&m)==2){
        make(1,1,n,m);
        printf("%d\n", count);
    }
        
    return 0;
} 