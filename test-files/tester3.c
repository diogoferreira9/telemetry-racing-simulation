#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <sys/mman.h>

#define FIFO_FILE "MYFIFO"
#define MEM_PARTILHADA "organizacao.shm"
#define COMPRIMENTO_MENSAGEM 40
#define TAMANHO_NOME 20
#define USER_LEVEL_PERMISSIONS 0666
#define NUM_CARROS 9
#define SHORT_DELAY 100000

typedef struct CarData
{
    int fifoInFD;
    int fifoOutFD;
    int id;
} CarData;

typedef struct OrganizacaoCorrida{
	short id;					/* Numero de carro */
    char nome_equipa[TAMANHO_NOME];          /* Nome da equipa do carro */
    char nome_piloto[TAMANHO_NOME];          /* Nome da equipa do carro */
    int id_equipa; /* Id unico da equipa*/
    int pneus_disponiveis; /* pneus disponives */
} Organizacao;

void * criaMemoriaPartilhada(char * nome, int tamanho)
{
    void *ptr = NULL; 
    int ret = 0;
    int fd = 0;

    /* Cria memoria partilhada */
    fd = shm_open(nome , O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);     
    if (fd == -1)
    {
        perror("Erro a usar shm_open"); 
        exit(1);
    }

    /* Define tamanho da memoria partilhada */
    ret = ftruncate(fd, tamanho); 
    if (ret == -1)
    {
        perror("Erro a usar ftruncate"); 
        exit(2);
    }

    /* Associa memoria partilhada ao ponteiro ptr */
    ptr = mmap(0, tamanho, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); 
    if (ptr == MAP_FAILED)
    {
        perror("shm-mmap"); 
        exit(3);
    }

    return ptr;
}

/******************************************************************************
 *                                                                            *
 *  FUNCAO apagaMemoriaPartilhada                                             *
 *                                                                            *
 *  Apaga uma area de memoria partilhada                                      *
 *                                                                            *
 *  nome - Nome da area de memoria partilhada                                 *
 *  tamanho - Tamanho da area de memoria partilhada                           *
 *  ptr - Ponteiro para area de memoria partilhada                            *
 *                                                                            *
 ******************************************************************************/
void apagaMemoriaPartilhada(char * nome, int tamanho, void * ptr)
{
    int ret = 0;

    ret = munmap(ptr, tamanho); 

    if (ret == -1)
    {
        perror("Erro na funcao munmap"); 
        exit(7);
    }

    ret = shm_unlink(nome); 
    if (ret == -1)
    {
        perror("Erro na funcao shm_unlink"); 
        exit(8);
    }
}


/*
   Preenche dados da estrutura organizacao
*/
void setOrganizacao(Organizacao *organizacao, short id, const char *nome_equipa,
                           const char *nome_piloto, int id_equipa, int pneus_disponiveis) {
    organizacao->id = id;
    strncpy(organizacao->nome_equipa, nome_equipa, TAMANHO_NOME - 1);
    organizacao->nome_equipa[TAMANHO_NOME - 1] = '\0';  
    strncpy(organizacao->nome_piloto, nome_piloto, TAMANHO_NOME - 1);
    organizacao->nome_piloto[TAMANHO_NOME - 1] = '\0';  
    organizacao->id_equipa = id_equipa;
    organizacao->pneus_disponiveis = pneus_disponiveis;
}

/// @brief Exemplo 1
void criaOrganizacaoT1T4(Organizacao * org)
{
     setOrganizacao(&org[0], 1, "EquipaA", "Manel", 1001, 20);
     setOrganizacao(&org[1], 2, "EquipaA", "Luis", 1001, 20);
     setOrganizacao(&org[2], 3, "EquipaB", "Costa", 2000, 20);
     setOrganizacao(&org[3], 4, "EquipaB", "Galamba", 2000, 20);
     setOrganizacao(&org[4], 5, "EquipaC", "Bobi", 555, 20);
     setOrganizacao(&org[5], 6, "EquipaC", "Tareco", 555, 20);
     setOrganizacao(&org[6], 7, "EquipaC", "Bichano", 555, 20);
     setOrganizacao(&org[7], 8, "EquipaD", "Fiambre", 1, 40);
     setOrganizacao(&org[8], 9, "EquipaD", "Vertate", 1, 40);
}

/// @brief Exemplo 2
void criaOrganizacaoT2T3(Organizacao * org)
{
     setOrganizacao(&org[0], 1, "FCPorta", "Pepepe", 1001, 20);
     setOrganizacao(&org[1], 2, "FCPorta", "Refila", 1001, 20);
     setOrganizacao(&org[2], 3, "Malfica", "Rafafa", 2000, 20);
     setOrganizacao(&org[3], 4, "Malfica", "Xmiiidt", 2000, 20);
     setOrganizacao(&org[4], 5, "Sporta", "Gyokky", 555, 20);
     setOrganizacao(&org[5], 6, "Sporta", "Amoras", 555, 20);
     setOrganizacao(&org[6], 7, "Praga", "ReiArt", 555, 20);
     setOrganizacao(&org[7], 8, "Praga", "Seila", 1, 40);
     setOrganizacao(&org[8], 9, "Praga", "Quem??", 1, 40);
}

//carro 3 e 7 com poucos pneus
void criaOrganizacaoT5(Organizacao * org)
{
     setOrganizacao(&org[0], 1, "EquipaA", "Manel", 1001, 20);
     setOrganizacao(&org[1], 2, "EquipaA", "Luis", 1001, 20);
     setOrganizacao(&org[2], 3, "EquipaB", "Costa", 2000, 20);
     setOrganizacao(&org[3], 4, "EquipaB", "Galamba", 2000, 20);
     setOrganizacao(&org[4], 5, "EquipaC", "Bobi", 555, 20);
     setOrganizacao(&org[5], 6, "EquipaC", "Tareco", 555, 20);
     setOrganizacao(&org[6], 7, "EquipaC", "Bichano", 555, 20);
     setOrganizacao(&org[7], 8, "EquipaD", "Fiambre", 1, 40);
     setOrganizacao(&org[8], 9, "EquipaD", "Vertate", 1, 40);
}

//carrs com  pneus a zero
void criaOrganizacaoT6(Organizacao * org)
{
     setOrganizacao(&org[0], 1, "EquipaA", "Manel", 1001, 0);
     setOrganizacao(&org[1], 2, "EquipaA", "Luis", 1001, 20);
     setOrganizacao(&org[2], 3, "EquipaB", "Costa", 2000, 2);
     setOrganizacao(&org[3], 4, "EquipaB", "Galamba", 2000, 2);
     setOrganizacao(&org[4], 5, "EquipaC", "Bobi", 555, 20);
     setOrganizacao(&org[5], 6, "EquipaC", "Tareco", 555, 20);
     setOrganizacao(&org[6], 7, "EquipaC", "Bichano", 555, 20);
     setOrganizacao(&org[7], 8, "EquipaD", "Fiambre", 1, 2);
     setOrganizacao(&org[8], 9, "EquipaD", "Vertate", 1, 40);
}


// carro 1,3,7 sem pneus
void criaOrganizacaoT8(Organizacao * org)
{
     setOrganizacao(&org[0], 1, "EquipaA", "Manel", 1001, 0);
     setOrganizacao(&org[1], 2, "EquipaA", "Luis", 1001, 20);
     setOrganizacao(&org[2], 3, "EquipaB", "Costa", 2000, 3);
     setOrganizacao(&org[3], 4, "EquipaB", "Galamba", 2000, 100);
     setOrganizacao(&org[4], 5, "EquipaC", "Bobi", 555, 4);
     setOrganizacao(&org[5], 6, "EquipaC", "Tareco", 555, 5);
     setOrganizacao(&org[6], 7, "EquipaC", "Bichano", 555, 2);
     setOrganizacao(&org[7], 8, "EquipaD", "Fiambre", 1, 222);
     setOrganizacao(&org[8], 9, "EquipaD", "Vertate", 1, 40);
}


void SendMessageTest3(CarData carData, int order) {
    char message[41];
    sprintf(message, "| %d | 10 | 3 | 45.%d | 050 | 060 | Soft |", carData.id, order);
    write(carData.fifoInFD, message, 40);
    usleep(SHORT_DELAY);
    close(carData.fifoInFD); 
    close(carData.fifoOutFD);
    return;
}

void SendMessageTest4(CarData carData) {
    char message[41];
    sprintf(message, "| %d | 01 | 3 | 41.5 | 050 | 060 | Soft |", carData.id);
    write(carData.fifoInFD, message, 40);
    usleep(SHORT_DELAY);
    sprintf(message, "| %d | 10 | 3 | 41.5 | 050 | 060 | Soft |", carData.id);
    write(carData.fifoInFD, message, 40);
    usleep(SHORT_DELAY);
    close(carData.fifoInFD); 
    close(carData.fifoOutFD);
    return;
}

void SendMessageTest51e61(CarData carData) {
    char message[41];
    int id = carData.id;
    sprintf(message, "| %d | 01 | 3 | 02.%d | 050 | 060 | Soft |", id, (id+3) % 10);
    write(carData.fifoInFD, message, 40);
    usleep(SHORT_DELAY);
    sprintf(message, "| %d | 03 | 3 | 04.%d | 030 | 060 | Soft |", id, (id + 1) % 10);
    write(carData.fifoInFD, message, 40);
    usleep(SHORT_DELAY);
    return;
}

void SendMessageTest52e62(CarData carData, Organizacao * org) {
    char message[41];
    char boxMessage[3];
    int id = carData.id;
    // Aqui envia dados que requerem mensagem BOX
    sprintf(message, "| %d | 05 | 3 | 0%d.3 | 005 | 060 | Soft |", id, id % 4 + 4);
    write(carData.fifoInFD, message, 40);
    usleep(SHORT_DELAY);
    sprintf(message, "| %d | 07 | 3 | 1%d.%d | 050 | 060 | Soft |", id, (id + 7) % 4, id % 10);
    write(carData.fifoInFD, message, 40);
    usleep(SHORT_DELAY);
    // Leitura dos dados BOX
    // nao faz read de carro sem pneus
    if(org[id-1].pneus_disponiveis >= 4)
        read(carData.fifoOutFD, boxMessage, 3);
    sprintf(message, "| %d | 09 | 3 | %d.%d | 050 | 060 | Soft |", id, (id + 2) % 10 + 14, id % 10);
    write(carData.fifoInFD, message, 40);
    usleep(SHORT_DELAY);
    close(carData.fifoInFD); 
    close(carData.fifoOutFD);
    return;
}


int main(int argc, char const *argv[])
{
    if (argc < 3){
        printf("Insufficient arguments for client process\n");
        return -1;
    }
    int serverPID = atoi(argv[1]);
    int testID = atoi(argv[2]);
    char fifoOut[11];
    char fifoIn[11];
    char message[41];
    int idOrder[NUM_CARROS] = {3, 4, 1, 2, 9, 8, 6, 7, 5};
    int idOrder2[NUM_CARROS] = {5, 3, 7, 1, 2, 6, 9, 8, 4};
    CarData carData[NUM_CARROS];    
    char* masterFifo = "MASTER.in";
    int masterFifoFd = 0;
    Organizacao *org;

	/* Abre memoria partilhada */
    org = criaMemoriaPartilhada(MEM_PARTILHADA, sizeof(Organizacao)*NUM_CARROS);
    
    sleep(1);
    for(int i = 0; i < NUM_CARROS; i++){
        carData[i].id = i+1;
        sprintf(fifoIn, "CARRO%d.in", i+1);
        sprintf(fifoOut, "CARRO%d.out", i+1);
        for(int j=0; j < 100; j++) {
            if (access(fifoOut, F_OK) == 0)
                break;
            if (j == 99){
                printf("Error: Could not find FIFO: %s\n", fifoOut);
                system("rm CAR*");
                return -1;
            }
            usleep(SHORT_DELAY);
        }

        carData[i].fifoOutFD = open(fifoOut, O_RDONLY);
        if (carData[i].fifoOutFD == -1) {
            printf("Error opening FIFO: %s\n", fifoOut);
            return -1;
        }
        for(int j=0; j < 100; j++) {
            if (access(fifoIn, F_OK) == 0)
                break;
            if (j == 99){
                printf("Error: Could not find FIFO: %s\n", fifoIn);
                system("rm CAR*");
                return -1;
            }
            usleep(SHORT_DELAY);
        }
        carData[i].fifoInFD = open(fifoIn, O_WRONLY);
        if (carData[i].fifoInFD  == -1){
            printf("Error opening FIFO: %s\n", fifoIn);
            return -1;
        }
    }

    for(int j=0; j < 100; j++) {
        if (access(masterFifo, F_OK) == 0)
            break;
        if (j == 99){
            printf("Error: Could not find FIFO: %s\n", masterFifo);
            system("rm CAR*");
            return -1;
        }
        usleep(SHORT_DELAY);
    }

    masterFifoFd = open(masterFifo, O_WRONLY);

    switch (testID)
    {
    case 1:
        printf("Launching script for test 2.1\n");
        criaOrganizacaoT1T4(org); // dados mem partilhada
        
        for(int i = 0; i < NUM_CARROS; i++){
            usleep(SHORT_DELAY);
            sprintf(message, "| %d | 10 | 3 | 41.5 | 050 | 060 | Soft |", i+1);
            write(carData[i].fifoInFD, message, 40);
            close(carData[i].fifoInFD);
            close(carData[i].fifoOutFD);
        }
        break;
    case 2:
        printf("Launching script for test 2.2\n");
        criaOrganizacaoT2T3(org); // dados mem partilhada
        for(int i = 0; i < NUM_CARROS; i++){
            sprintf(message, "| %d | 01 | 3 | 41.5 | 050 | 060 | Soft |", i+1);
            write(carData[i].fifoInFD, message, 40);
            usleep(SHORT_DELAY);
            sprintf(message, "| %d | 10 | 3 | 41.5 | 050 | 060 | Soft |", i+1);
            write(carData[i].fifoInFD, message, 40);
            usleep(SHORT_DELAY);
            close(carData[i].fifoInFD);
            close(carData[i].fifoOutFD );
        }
        break;
    case 3:
        printf("Launching script for test 2.3\n");
        criaOrganizacaoT2T3(org);
        for(int i = 0; i < NUM_CARROS; i++){
            SendMessageTest3(carData[idOrder[i]-1], idOrder[i]);
        }
        break;
    case 4:
        printf("Launching script for test 2.4\n");
        criaOrganizacaoT1T4(org);
        for(int i = 0; i < NUM_CARROS; i++){
            SendMessageTest4(carData[idOrder2[i]-1]);
            if(i == 1 || i == 5) {
                kill(serverPID, SIGUSR1);
                sleep(1);
            } else if(i == 3 || i == 7) {
                kill(serverPID, SIGUSR2);
                sleep(1);
            }
        }
        break;
    case 5: //teste para msg box
        printf("Launching script for test 2.5\n");
        criaOrganizacaoT5(org);
        for(int i = 0; i < NUM_CARROS; i++){
            SendMessageTest51e61(carData[idOrder[i]-1]);
        }
        for(int i = 0; i < NUM_CARROS; i++){
            SendMessageTest52e62(carData[idOrder2[i]-1], org);
        }
        break;
    case 6: //teste msg box sem pneus
        criaOrganizacaoT6(org);
        printf("Launching script for test 2.6\n");
        for(int i = 0; i < NUM_CARROS; i++){
            SendMessageTest51e61(carData[idOrder[i]-1]);
        }
        for(int i = 0; i < NUM_CARROS; i++){
            SendMessageTest52e62(carData[idOrder[i]-1], org);
        }
        break;    
    default:
        printf("Undefined argument test ID\n");
        return -1;
    }
    system("rm CAR*");
    system("rm MASTER.in");
    return 0;
}
