#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include <memory.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

#define IMAGE_SIZE      784 // 28*28
#define IMAGE_WIDTH     28
#define IMAGE_HEIGHT    28
#define NUM_TRAIN       60000
#define NUM_TEST        10000
#define NUM_CLASS       10


double  X[NUM_TRAIN][IMAGE_SIZE];
int     Y[NUM_TRAIN][NUM_CLASS];

double  Y_HAT[NUM_TRAIN][NUM_CLASS];
double  W[IMAGE_SIZE][NUM_CLASS];
double  W_GRAD[IMAGE_SIZE][NUM_CLASS];

double  X_TEST[NUM_TEST][IMAGE_SIZE];
int     Y_TEST[NUM_TEST];
int     Y_ORIGIN[NUM_TRAIN];

double RESULT[NUM_TEST][2];

typedef struct ThreadInfo{
    int id;
    int batch;
    double **ret;
}ThreadInfo;


void *train(void *arg);
void *evaluate(void *arg);
void loadImage(char *path, int imageNum, double X[][IMAGE_SIZE]);
void loadLabelOneHot(char *path, int imageNum, int Y[][NUM_CLASS]);
void loadLabelInt(char *path, int imageNum, int Y[]);
void mnist(char *xPath, char *yPath, char *xTestPath, char *yTestPath);
void saveWeight(char *name);
void saveCSV();
void initWeight();
int  charToInt(char *str);

const int       ITERATION       = 10;
const double    LEARNING_RATE   = 0.0005;
const int       BATCH_SIZE      = 1000;
int             NUM_BATCH;
int threadNum, jobPerThread;

int main(int argc, char *argv[]){
    NUM_BATCH = NUM_TRAIN / BATCH_SIZE;
    char *X_train_path, *Y_train_path, *X_test_path, *Y_test_path;
    if (argc == 1){
        X_train_path    = "X_train";
        Y_train_path    = "y_train";
        X_test_path     = "X_test";
        Y_test_path     = "y_test";
        threadNum       = 1;
    }    
    else if (argc == 5){
        X_train_path    = argv[1];
        Y_train_path    = argv[2];
        X_test_path     = argv[3];
        Y_test_path     = 0;
        threadNum = charToInt(argv[4]);
    }
    else {
        printf("[Error] Usage: ./hw4 [X_train] [Y_train] [X_test] [number of threads]\n");
        exit(-1);
    }
    
    jobPerThread = BATCH_SIZE / threadNum;
    mnist(X_train_path, Y_train_path, X_test_path, Y_test_path);
    
    initWeight();

    pthread_t *threads = malloc(threadNum * sizeof(pthread_t));
    ThreadInfo *threadInfo = malloc(threadNum * sizeof(ThreadInfo));
    double **dGrad[threadNum]; 
    for (int i = 0; i < threadNum; ++i){
        dGrad[i] = (double **)malloc(IMAGE_SIZE * sizeof(double *));
        for (int j = 0; j < IMAGE_SIZE; ++j){
            dGrad[i][j] = (double *)malloc(NUM_CLASS * sizeof(double));
        }
    }

    printf("[INFO] Start training\n");
    printf("[Info] Thread Number: %d, Total Iteration: %d, Batch Size: %d\n", threadNum, ITERATION, BATCH_SIZE);
    for (int iter = 0; iter < ITERATION; ++iter){
        for (int b = 0; b < NUM_BATCH; ++b){
            printf("\r[INFO] Iteration %d, Batch %d  ", iter+1, b+1);
            fflush(stdout);

            for (int t = 0; t < threadNum; ++t){
                threadInfo[t].id = t;
                threadInfo[t].batch = b;
                threadInfo[t].ret = dGrad[t];
                pthread_create(&threads[t], NULL, train, (void*) &threadInfo[t]);
            }
            for (int t = 0; t < threadNum; ++t)
                pthread_join(threads[t], NULL);
            
            for (int t = 0; t < threadNum; ++t)
                for (int i = 0; i < IMAGE_SIZE; ++i)
                    for (int j = 0; j < NUM_CLASS; ++j)
                        W[i][j] -= LEARNING_RATE * dGrad[t][i][j];
        }
    }
    printf("\n[INFO] Done training\n");
    #ifdef SAVE_WEIGHT
    saveWeight("weight.txt");
    #endif
    
    printf("[INFO] Start evaluating\n");
    jobPerThread = NUM_TEST / threadNum;
    void *cnt[threadNum];
    for (int t = 0; t < threadNum; ++t){
        threadInfo[t].id = t;
        pthread_create(&threads[t], NULL, evaluate, (void*) &threadInfo[t]);
    }
    for (int t = 0; t < threadNum; ++t)
        pthread_join(threads[t], &cnt[t]);
    
    if (Y_test_path == NULL)
        saveCSV();
    else {
        int correctness = 0;
        for (int t = 0; t < threadNum; ++t)
            correctness += *(int*)cnt[t];
        saveCSV();
        printf("[INFO] %d correct of %d test data. Accuracy %lf\n", correctness, NUM_TEST, (double)correctness / (double)NUM_TEST);
    }    
    
    return 0;
}

void *train(void *arg) {
    ThreadInfo *info = ((ThreadInfo *)arg);
    int start, end;
    start = info->batch * NUM_BATCH + jobPerThread * info->id;
    end = start + jobPerThread;
    // X * W
    for (int i = start; i < end; ++i){
        for (int c = 0; c < NUM_CLASS; ++c){
            Y_HAT[i][c] = 0.0;
            for (int j = 0; j < IMAGE_SIZE; ++j){
                Y_HAT[i][c] += X[i][j] * W[j][c];
            }
        }
    }
    // softmax(X*W), Y_HAT = (Y_HAT - Y)
    double expo[NUM_CLASS], sum;
    for (int i = start; i < end; ++i){
        sum = 0.0;
        for (int j = 0; j < NUM_CLASS; ++j){
            expo[j] = exp(Y_HAT[i][j]);
            sum += expo[j];
        }
        for (int j = 0; j < NUM_CLASS; ++j) {
            Y_HAT[i][j] = expo[j] / sum; // softmax
            Y_HAT[i][j] -= Y[i][j]; // Y_HAT - Y
        }
    }
    // X_TRAN * (Y_HAT - Y)
    for (int i = 0; i < IMAGE_SIZE; ++i){
        for (int c = 0; c < NUM_CLASS; ++c){
            info->ret[i][c] = 0.0;
            for (int j = start; j < end; ++j){
                info->ret[i][c] += X[j][i] * Y_HAT[j][c];
            }
        }
    }
    
    pthread_exit(NULL);
}

void *evaluate(void *arg){
    ThreadInfo *info = ((ThreadInfo *)arg);
    int start, end;
    start = jobPerThread * info->id;
    end = (start + jobPerThread > NUM_TEST) ? NUM_TEST : start + jobPerThread;

    for (int i = start; i < end; ++i){
        for (int c = 0; c < NUM_CLASS; ++c){
            Y_HAT[i][c] = 0.0;
            for (int j = 0; j < IMAGE_SIZE; ++j){
                Y_HAT[i][c] += X_TEST[i][j] * W[j][c];
            }
        }
    }
    int *cnt = malloc(sizeof(int));
    cnt[0] = 0;
    double expo[NUM_CLASS], sum;
    for (int i = start; i < end; ++i){
        sum = 0.0;
        for (int j = 0; j < NUM_CLASS; ++j){
            expo[j] = exp(Y_HAT[i][j]);
            sum += expo[j];
        }
        double maxP = -10000; int class = -1;
        for (int j = 0; j < NUM_CLASS; ++j) {
            Y_HAT[i][j] = expo[j] / sum;
            if (Y_HAT[i][j] > maxP){
                maxP = Y_HAT[i][j];
                class = j;
            }
        }
        RESULT[i][0] = class;
        RESULT[i][1] = maxP;
        if (class == Y_TEST[i])
            cnt[0]++;
    }
    pthread_exit((void *)cnt);
}

void loadImage(char *path, int imageNum, double X[][IMAGE_SIZE]){
    FILE *file;
    file = fopen(path, "rb");
    if (file == NULL){
        printf("[ERROR] Can't load file %s\n", path);
        exit(-1);
    }
    unsigned char tmp;
    for (int i = 0; i < imageNum; ++i){
        for (int j = 0; j < IMAGE_SIZE; ++j){           
            if (fread(&tmp, sizeof(unsigned char), 1, file) != 1) {
                printf("[ERROR] fread error.\n");
                exit(-1);
            }
            X[i][j] = (double)tmp / (double) 255;
        }
    }
    fclose(file);
}

void loadLabelOneHot(char *path, int imageNum, int Y[][NUM_CLASS]){
    FILE *file;
    file = fopen(path, "rb");
    if (file == NULL){
        printf("[ERROR] Can't load file %s\n", path);
        exit(-1);
    }
    unsigned char tmp;
    for (int i = 0; i < imageNum; ++i){
        if (fread(&tmp, sizeof(unsigned char), 1, file) != 1) {
            printf("[ERROR] fread error.\n");
            exit(-1);
        }
        Y_ORIGIN[i] = (int)tmp;
        for (int j = 0; j < 10; ++j){
            if (j == (int)tmp) Y[i][j] = 1;
            else Y[i][j] = 0;
        }
    }
    fclose(file);
}

void loadLabelInt(char *path, int imageNum, int Y[]){
    FILE *file;
    file = fopen(path, "rb");
    if (file == NULL){
        printf("[ERROR] Can't load file %s\n", path);
        exit(-1);
    }
    unsigned char tmp;
    for (int i = 0; i < imageNum; ++i){
        if (fread(&tmp, sizeof(char), 1, file) != 1) {
            printf("[ERROR] fread error.\n");
            exit(-1);
        }
        Y[i] = (int)tmp;
    }
    fclose(file);
} 

void mnist(char *xPath, char *yPath, char *xTestPath, char *yTestPath){
    printf("[INFO] Loading mnist dataset\n");
    loadImage(xPath, NUM_TRAIN, X);
    loadLabelOneHot(yPath, NUM_TRAIN, Y);
    loadImage(xTestPath, NUM_TEST, X_TEST);
    if (yTestPath != 0)
        loadLabelInt(yTestPath, NUM_TEST, Y_TEST);
    printf("[INFO] Mnist dataset loaded\n");
}

void saveWeight(char *name){
    FILE *file;
    file = fopen(name, "w");
    for (int k = 0; k < 10; ++k){
        for (int i = 0; i < IMAGE_HEIGHT; ++i){
            for (int j = 0; j < IMAGE_WIDTH; ++j){
                fprintf(file, "%lf ", W[i*IMAGE_HEIGHT + j][k]);
            }
            if (i == 0) fprintf(file, "%d", k);
            fprintf(file, "\n");
        }
        fprintf(file, "\n");
    }
    fclose(file);
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

void initWeight(){
    printf("[INFO] Initializing weight\n");
    for (int i = 0; i < IMAGE_SIZE; ++i){
        for (int j = 0; j < NUM_CLASS; ++j){
            W[i][j] = 0.0001;
        }
    }
}

void saveCSV(){
    char *name = "result.csv";
    FILE *file;
    file = fopen(name, "w");
    if (file == NULL){
        printf("[ERROR] Can't open file %s\n", name);
        exit(-1);
    }
    fprintf(file, "id,label\n");
    for (int i = 0; i < NUM_TEST; ++i)
        fprintf(file, "%d,%d\n", i, (int)RESULT[i][0]);
    fclose(file);
    printf("[INFO] Saved result to result.csv\n");
}