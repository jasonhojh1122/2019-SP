#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include <memory.h>
#include <string.h>

#define IMAGE_SIZE      784 // 28*28
#define IMAGE_WIDTH     28
#define IMAGE_HEIGHT    28
#define NUM_TRAIN       60000
#define NUM_TEST        10000
#define NUM_CLASS       10

const int     ITERATION       = 1;
const double  LEARNING_RATE   = 0.001;

double  X[NUM_TRAIN][IMAGE_SIZE];
int     Y[NUM_TRAIN][NUM_CLASS];

double  Y_HAT[NUM_TRAIN][NUM_CLASS];
double  W[IMAGE_SIZE][NUM_CLASS];
double  W_GRAD[IMAGE_SIZE][NUM_CLASS];

double  X_TEST[NUM_TEST][IMAGE_SIZE];
int     Y_TEST[NUM_TEST];
int     Y_ORIGIN[NUM_TRAIN];

double EVA[NUM_TEST][NUM_CLASS];
double RESULT[NUM_TEST][2];

typedef struct ThreadInfo{
    int id;
    double **ret;
    int *correct;
}ThreadInfo;

int threadNum, jobPerThread;

void loadImage(char *path, int imageNum, double X[][IMAGE_SIZE]);
void loadLabelOneHot(char *path, int imageNum, int Y[][NUM_CLASS]);
void loadLabelInt(char *path, int imageNum, int Y[]);
void mnist(char *xPath, char *yPath, char *xTestPath, char *yTestPath);
void saveX(char *name);
void saveXTest(char *name);
void saveWeight(char *name);
void initWeight();
int  charToInt(char *str);

int main(int argc, char *argv[]){
    
    mnist("X_train", "y_train", "X_test", "y_test");
    
    initWeight();

    printf("[INFO] Start training ...\n");
    for (int iter = 0; iter < ITERATION; ++iter){
        printf("[INFO] Iteration %d\n", iter+1);
        
        // Y_Hat = softmax(X * W)
        printf("[INFO] Calculating Y_Hat\n");
        for (int i = 0; i < NUM_TRAIN; ++i){
            for (int c = 0; c < NUM_CLASS; ++c){
                Y_HAT[i][c] = 0.0;
                for (int j = 0; j < IMAGE_SIZE; ++j){
                    Y_HAT[i][c] += X[i][j] * W[j][c];
                }
            }
        }
        double sum = 0.0;
        double SOFT[NUM_CLASS];
        for (int i = 0; i < NUM_TRAIN; ++i){
            sum = 0.0;
            for (int j = 0; j < NUM_CLASS; ++j){
                SOFT[j] = 
                (Y_HAT[i][j]);
                sum += SOFT[j];
            }
            for (int j = 0; j < NUM_CLASS; ++j){
                Y_HAT[i][j] = SOFT[j] / sum;
                Y_HAT[i][j] -= Y[i][j];
            }
        }
        
        printf("[INFO] Updating Weight\n");
        for (int i = 0; i < IMAGE_SIZE; ++i){
            for (int j = 0; j < NUM_CLASS; ++j){
                W_GRAD[i][j] = 0.0;
                for (int k = 0; k < NUM_TRAIN; ++k){
                    W_GRAD[i][j] += X[k][i] * Y_HAT[k][j];
                }
            }
        }
        for (int i = 0; i < IMAGE_SIZE; ++i){
            for (int j = 0; j < NUM_CLASS; ++j){
                W[i][j] -= LEARNING_RATE * W_GRAD[i][j];
            }
        }
        saveWeight("weight.txt");
        /*
        for (int t = 0; t < threadNum; ++t){
            for (int i = 0; i < IMAGE_SIZE; ++i){
                for (int j = 0; j < NUM_CLASS; ++j){
                    W[i][j] -= LEARNING_RATE * dGrad[t][i][j];
                }
            }
        }*/
        fflush(stdout);
    }

    printf("[INFO] Done training\n");
    
    return 0;
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
            if (fread(&tmp, sizeof(char), 1, file) != 1) {
                printf("[ERROR] fread error.\n");
                exit(-1);
            }
            int value = 0;
            for (int k = 0; k < 8; ++k)
                if (tmp & (1 << k)) value += (1 << k);
            
            X[i][j] = (double)value / (double)255;
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
        if (fread(&tmp, sizeof(char), 1, file) != 1) {
            printf("[ERROR] fread error.\n");
            exit(-1);
        }
        int label = 0;
        for (int j = 0; j < 4; ++j)
            if (tmp & (1 << j)) label += (1 << j);

        Y_ORIGIN[i] = label;
        for (int j = 0; j < 10; ++j){
            if (j == label) Y[i][j] = 1;
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
        int label = 0;
        for (int j = 0; j < 4; ++j)
            if (tmp & (1 << j)) label += (1 << j);

        Y[i] = label;
    }
    fclose(file);
} 

void mnist(char *xPath, char *yPath, char *xTestPath, char *yTestPath){
    printf("[INFO] Loading mnist dataset ...\n");
    loadImage(xPath, NUM_TRAIN, X);
    loadLabelOneHot(yPath, NUM_TRAIN, Y);
    loadImage(xTestPath, NUM_TEST, X_TEST);
    if (yTestPath != NULL);
        loadLabelInt(yTestPath, NUM_TEST, Y_TEST);
    
    printf("[INFO] Mnist dataset loaded.\n");
}

void saveX(char *name){
    FILE *file;
    file = fopen(name, "w");
    for (int k = 0; k < 10; ++k){
        for (int i = 0; i < IMAGE_HEIGHT; ++i){
            for (int j = 0; j < IMAGE_WIDTH; ++j){
                fprintf(file, "%lf ", X[k][i*IMAGE_HEIGHT + j]);
            }
            if (i == 0) fprintf(file, "%d", Y_ORIGIN[k]);
            fprintf(file, "\n");
        }
        fprintf(file, "\n");
    }
    fclose(file);
}

void saveXTest(char *name){
    FILE *file;
    file = fopen(name, "w");
    for (int k = 0; k < 10; ++k){
        for (int i = 0; i < IMAGE_HEIGHT; ++i){
            for (int j = 0; j < IMAGE_WIDTH; ++j){
                fprintf(file, "%lf ", X_TEST[k][i*IMAGE_HEIGHT + j]);
            }
            if (i == 0) fprintf(file, "%d", Y_TEST[k]);
            fprintf(file, "\n");
        }
        fprintf(file, "\n");
    }
    fclose(file);
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
    printf("[INFO] Initializing weight.\n");
    for (int i = 0; i < IMAGE_SIZE; ++i){
        for (int j = 0; j < NUM_CLASS; ++j){
            W[i][j] = (double)1 / (double)IMAGE_SIZE;
        }
    }
}