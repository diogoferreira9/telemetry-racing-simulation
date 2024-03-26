#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define COMPRIMENTO_MENSAGEM 40
#define USER_LEVEL_PERMISSIONS 0666
#define NUM_CARROS 9
#define SHORT_DELAY 100000

typedef struct CarData
{
    int fifoInFD;
    int fifoOutFD;
    int id;
} CarData;

int main(int argc, char const *argv[])
{
    //Require a test number
    if (argc < 2){
        printf("Insufficient arguments for client process\n");
        return -1;
    }
    int testID = atoi(argv[1]);
    char fifoOut[11];
    char fifoIn[11];
    char message[41];
    CarData carData[NUM_CARROS];
    //Wait for server to be ready
    sleep(1);
    for(int i = 0; i < NUM_CARROS; i++){
        carData[i].id = i+1;
        sprintf(fifoIn, "CARRO%d.in", i+1);
        sprintf(fifoOut, "CARRO%d.out", i+1);
        for(int j=0; j < 100; j++) {
            // Check if FIFO file exists
            if (access(fifoOut, F_OK) == 0)
                break;
            if (j == 99){
                printf("Error: Could not find FIFO: %s\n", fifoOut);
                return -1;
            }
            usleep(SHORT_DELAY);
        }
        //Open FIFO
        carData[i].fifoOutFD = open(fifoOut, O_RDONLY);
        if (carData[i].fifoOutFD == -1) {
            printf("Error opening FIFO: %s\n", fifoOut);
            return -1;
        }
        for(int j=0; j < 100; j++) {
            // Check if FIFO file exists
            if (access(fifoIn, F_OK) == 0)
                break;
            if (j == 99){
                printf("Error: Could not find FIFO: %s\n", fifoIn);
                return -1;
            }
            usleep(SHORT_DELAY);
        }
        //Open FIFO
        carData[i].fifoInFD = open(fifoIn, O_WRONLY);
        if (carData[i].fifoInFD  == -1){
            printf("Error opening FIFO: %s\n", fifoIn);
            return -1;
        }
    }
    //Switch between different test setups
    //Additional cases can be defined for more tests
    switch (testID)
    {
    case 1:
        printf("Launching script for test 2.1\n");
        for(int i = 0; i < NUM_CARROS; i++){
            //Write a message to a string
            sprintf(message, "| %d | 10 | 3 | 41.5 | 050 | 060 | Soft |", i+1);
            //Write to the FIFO
            write(carData[i].fifoInFD, message, 40);
            //Sleep to allow threads to operate
            usleep(SHORT_DELAY);
            //Close FIFO
            close(carData[i].fifoInFD);
            close(carData[i].fifoOutFD);
        }
        break;
    case 2:
        printf("Launching script for test 2.2\n");
        for(int i = 0; i < NUM_CARROS; i++){
            //Write a message to a string
            sprintf(message, "| %d | 01 | 3 | 41.5 | 050 | 060 | Soft |", i+1);
            //Write to the FIFO
            write(carData[i].fifoInFD, message, 40);
            //Sleep to allow threads to operate
            usleep(SHORT_DELAY);
            sprintf(message, "| %d | 10 | 3 | 41.5 | 050 | 060 | Soft |", i+1);
            write(carData[i].fifoInFD, message, 40);
            usleep(SHORT_DELAY);
            //Close FIFO
            close(carData[i].fifoInFD);
            close(carData[i].fifoOutFD );
        }
        break;
    default:
        printf("Undefined argument test ID\n");
        return -1;
    }
    return 0;
}
