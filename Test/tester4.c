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
#include <assert.h>

#define FIFO_FILE "MYFIFO"
#define MEM_PARTILHADA "organizacao.shm"
#define MEM_PARTILHADA2 "combustivel.shm"
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


/******************************************************************************
 *                                                                            *
 *  FUNCAO criaMemoriaPartilhada                                              *
 *                                                                            *
 *  Cria uma area de memoria partilhada                                       *
 *                                                                            *
 *  nome - Nome da area de memoria partilhada                                 *
 *  tamanho - Tamanho da area de memoria partilhada                           *
 *                                                                            *
 ******************************************************************************/
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
void criaOrganizacaoT1T4T7(Organizacao * org)
{
     setOrganizacao(&org[0], 1, "EquipaA", "Manel", 1, 20);
     setOrganizacao(&org[1], 2, "EquipaA", "Luis", 1, 20);
     setOrganizacao(&org[2], 3, "EquipaB", "Costa", 2, 20);
     setOrganizacao(&org[3], 4, "EquipaB", "Galamba", 2, 20);
     setOrganizacao(&org[4], 5, "EquipaC", "Bobi", 3, 20);
     setOrganizacao(&org[5], 6, "EquipaC", "Tareco", 3, 20);
     setOrganizacao(&org[6], 7, "EquipaC", "Bichano", 3, 20);
     setOrganizacao(&org[7], 8, "EquipaD", "Fiambre", 4, 40);
     setOrganizacao(&org[8], 9, "EquipaD", "Vertate", 4, 40);
}

/// @brief Exemplo 2
void criaOrganizacaoT2T3(Organizacao * org)
{
     setOrganizacao(&org[0], 1, "FCPorta", "Pepepe", 1, 20);
     setOrganizacao(&org[1], 2, "FCPorta", "Refila", 1, 20);
     setOrganizacao(&org[2], 3, "Malfica", "Rafafa", 2, 20);
     setOrganizacao(&org[3], 4, "Malfica", "Xmiiidt", 2, 20);
     setOrganizacao(&org[4], 5, "Sporta", "Gyokky", 3, 20);
     setOrganizacao(&org[5], 6, "Sporta", "Amoras", 3, 20);
     setOrganizacao(&org[6], 7, "Praga", "ReiArt", 3, 20);
     setOrganizacao(&org[7], 8, "Praga", "Seila", 4, 20);
     setOrganizacao(&org[8], 9, "Praga", "Quem??", 4, 20);
}

void criaOrganizacaoT4(Organizacao * org)
{
     setOrganizacao(&org[0], 5, "FCPorta", "Pepepe", 1, 20);
     setOrganizacao(&org[1], 2, "FCPorta", "Refila", 1, 20);
     setOrganizacao(&org[2], 3, "Malfica", "Rafafa", 2, 20);
     setOrganizacao(&org[3], 4, "Malfica", "Xmiiidt", 2, 20);
     setOrganizacao(&org[4], 1, "Sporta", "Gyokky", 3, 20);
     setOrganizacao(&org[5], 7, "Sporta", "Amoras", 3, 20);
     setOrganizacao(&org[6], 6, "Praga", "ReiArt", 3, 20);
     setOrganizacao(&org[7], 8, "Praga", "Seila", 4, 40);
     setOrganizacao(&org[8], 9, "Praga", "Quem??", 4, 40);
}


void criaOrganizacaoT5(Organizacao * org)
{
     setOrganizacao(&org[0], 8, "EquipaA", "Manel", 1, 20);
     setOrganizacao(&org[1], 2, "EquipaA", "Luis", 1, 20);
     setOrganizacao(&org[2], 3, "EquipaB", "Costa", 2, 20);
     setOrganizacao(&org[3], 4, "EquipaB", "Galamba", 2, 20);
     setOrganizacao(&org[4], 5, "EquipaC", "Bobi", 3, 20);
     setOrganizacao(&org[5], 6, "EquipaC", "Tareco", 3, 20);
     setOrganizacao(&org[6], 7, "EquipaC", "Bichano", 3, 20);
     setOrganizacao(&org[7], 9, "EquipaD", "Fiambre", 4, 40);
     setOrganizacao(&org[8], 1, "EquipaD", "Vertate", 4, 40);
}

//carrs com  pneus a zero
void criaOrganizacaoT6(Organizacao * org)
{
     setOrganizacao(&org[0], 1, "EquipaA", "Manel", 1, 0);
     setOrganizacao(&org[1], 3, "EquipaA", "Luis", 1, 0);
     setOrganizacao(&org[2], 5, "EquipaB", "Costa", 2, 2);
     setOrganizacao(&org[3], 7, "EquipaB", "Galamba", 2, 2);
     setOrganizacao(&org[4], 9, "EquipaC", "Bobi", 3, 20);
     setOrganizacao(&org[5], 2, "EquipaC", "Tareco", 3, 20);
     setOrganizacao(&org[6], 4, "EquipaC", "Bichano", 3, 20);
     setOrganizacao(&org[7], 8, "EquipaD", "Fiambre", 4, 40);
     setOrganizacao(&org[8], 6, "EquipaD", "Vertate", 4, 40);
}


// carro 1,3,7 sem pneus
void criaOrganizacaoT8(Organizacao * org)
{
     setOrganizacao(&org[0], 1, "EquipaA", "Manel", 1, 40);
     setOrganizacao(&org[1], 2, "EquipaA", "Luis", 1, 20);
     setOrganizacao(&org[2], 3, "EquipaB", "Costa", 2, 43);
     setOrganizacao(&org[3], 4, "EquipaB", "Galamba", 2, 100);
     setOrganizacao(&org[4], 5, "EquipaC", "Bobi", 3, 40);
     setOrganizacao(&org[5], 6, "EquipaC", "Tareco", 3, 5);
     setOrganizacao(&org[6], 7, "EquipaC", "Bichano", 3, 2);
     setOrganizacao(&org[7], 8, "EquipaD", "Fiambre", 4, 222);
     setOrganizacao(&org[8], 9, "EquipaD", "Vertate", 4, 40);
}

// Carro 2 sem pneus
void criaOrganizacaoT9(Organizacao * org)
{
     setOrganizacao(&org[0], 1, "EquipaA", "Manel", 1, 40);
     setOrganizacao(&org[1], 2, "EquipaA", "Luis", 1, 1);
     setOrganizacao(&org[2], 3, "EquipaB", "Costa", 2, 43);
     setOrganizacao(&org[3], 4, "EquipaB", "Galamba", 2, 100);
     setOrganizacao(&org[4], 5, "EquipaC", "Bobi", 3, 40);
     setOrganizacao(&org[5], 6, "EquipaC", "Tareco", 3, 5);
     setOrganizacao(&org[6], 7, "EquipaC", "Bichano", 3, 22);
     setOrganizacao(&org[7], 8, "EquipaD", "Fiambre", 4, 222);
     setOrganizacao(&org[8], 9, "EquipaD", "Vertate", 4, 40);
}


void SendMessageTest21(CarData carData, int order) {
    char message[41];
    char boxMessage[4];
    sprintf(message, "| %d | 01 | 3 | 1%d.%d | 100 | 060 | Soft |", carData.id, order-1, order);
    write(carData.fifoInFD, message, 40);
    usleep(SHORT_DELAY);
    if (carData.id == 3 || carData.id == 5) {
        sprintf(message, "| %d | 05 | 3 | 2%d.%d | 100 | 005 | Soft |", carData.id, order-1, order);
    } else {
        sprintf(message, "| %d | 05 | 3 | 2%d.%d | 100 | 040 | Soft |", carData.id, order-1, order);
    }
    write(carData.fifoInFD, message, 40);
    usleep(SHORT_DELAY);
    if(carData.id == 3 || carData.id == 5)
        read(carData.fifoOutFD, boxMessage, 3);
    return;
}

void SendMessageTest22(CarData carData, int order) {
    char message[41];
    sprintf(message, "| %d | 10 | 3 | 3%d.%d | 100 | 060 | Soft |", carData.id, order-1, order);
    write(carData.fifoInFD, message, 40);
    usleep(SHORT_DELAY);
    close(carData.fifoInFD); 
    close(carData.fifoOutFD);
    return;
}

void SendMessageTest31(CarData carData, int order) {
    char message[41];
    char boxMessage[4];
    sprintf(message, "| %d | 01 | 3 | 1%d.%d | 100 | 060 | Soft |", carData.id, order-1, order);
    write(carData.fifoInFD, message, 40);
    usleep(SHORT_DELAY);
    if (carData.id == 3 || carData.id == 5) {
        sprintf(message, "| %d | 05 | 3 | 2%d.%d | 100 | 005 | Soft |", carData.id, order-1, order);
    } else {
        sprintf(message, "| %d | 05 | 3 | 2%d.%d | 100 | 040 | Soft |", carData.id, order-1, order);
    }
    write(carData.fifoInFD, message, 40);
    usleep(SHORT_DELAY);
    if(carData.id == 3 || carData.id == 5)
        read(carData.fifoOutFD, boxMessage, 3);
    return;
}

void SendMessageTest32(CarData carData, int order) {
    char message[41];
    char boxMessage[4];

    if (carData.id == 7) {
        sprintf(message, "| %d | 8 | 3 | 99.9 | 100 | 040 | Soft |", carData.id);
        write(carData.fifoInFD, message, 40);
        usleep(SHORT_DELAY);
    }
    else
    {
        sprintf(message, "| %d | 7 | 3 | 4%d.%d | 100 | 030 | Soft |", carData.id, order-1, order);
        write(carData.fifoInFD, message, 40);
        usleep(SHORT_DELAY);
    }
    if (carData.id == 2 || carData.id == 3 || carData.id == 5 || carData.id == 8) {
        sprintf(message, "| %d | 8 | 3 | 5%d.%d | 100 | 005 | Soft |", carData.id, order-1, order);
        write(carData.fifoInFD, message, 40);
        usleep(SHORT_DELAY);
        read(carData.fifoOutFD, boxMessage, 3);
    }
    return;
}

void SendMessageTest33(CarData carData, int order) {
    if (carData.id == 7) //despiste
        return;
    char message[41];
    sprintf(message, "| %d | 10 | 3 | 6%d.%d | 100 | 060 | Soft |", carData.id, order-1, order);
    write(carData.fifoInFD, message, 40);
    usleep(SHORT_DELAY);
    close(carData.fifoInFD); 
    close(carData.fifoOutFD);
    return;
}



void SendMessageTest41(CarData carData, int order) {
    char message[41];
    char boxMessage[4];
    sprintf(message, "| %d | 01 | 3 | 0%d.%d | 100 | 060 | Soft |", carData.id, order, order+1);
    write(carData.fifoInFD, message, 40);
    usleep(SHORT_DELAY);
    if (carData.id == 1 || carData.id == 4) {
        sprintf(message, "| %d | 05 | 3 | 2%d.%d | 100 | 005 | Soft |", carData.id, order-1, order);
    } else {
        sprintf(message, "| %d | 05 | 3 | 2%d.%d | 100 | 040 | Soft |", carData.id, order-1, order);
    }
    write(carData.fifoInFD, message, 40);
    usleep(SHORT_DELAY);
    if(carData.id == 1 || carData.id == 4)
        read(carData.fifoOutFD, boxMessage, 3);
    return;
}

void SendMessageTest42(CarData carData, int order) {
    char message[41];
    char boxMessage[4];
    sprintf(message, "| %d | 7 | 3 | 3%d.%d | 100 | 030 | Soft |", carData.id, order-1, order);
    write(carData.fifoInFD, message, 40);
    usleep(SHORT_DELAY);
    if (carData.id == 1 || carData.id == 4) {
        sprintf(message, "| %d | 8 | 3 | 4%d.%d | 100 | 005 | Soft |", carData.id, order-1, order);
        write(carData.fifoInFD, message, 40);
        usleep(SHORT_DELAY);
        read(carData.fifoOutFD, boxMessage, 3);
    }
    sprintf(message, "| %d | 10 | 3 | 7%d.%d | 100 | 030 | Soft |", carData.id, (carData.id+3 )% 9, (carData.id+2 )% 9);
    write(carData.fifoInFD, message, 40);
    usleep(SHORT_DELAY);
    close(carData.fifoInFD); 
    close(carData.fifoOutFD);
    return;
}


void SendMessageTest51(CarData carData) {
    char message[41];
    int id = carData.id;
    sprintf(message, "| %d | 01 | 3 | 02.%d | 100 | 060 | Soft |", id, (id+3) % 10);
    write(carData.fifoInFD, message, 40);
    usleep(SHORT_DELAY);
    sprintf(message, "| %d | 03 | 3 | 04.%d | 100 | 060 | Soft |", id, (id + 1) % 10);
    write(carData.fifoInFD, message, 40);
    usleep(SHORT_DELAY);
    return;
}

void SendMessageTest52(CarData carData) {
    char message[41];
    char boxMessage[3];
    int id = carData.id;
    // Aqui envia dados que requerem mensagem BOX
    if (id == 2 || id == 5 || id == 9) {
        sprintf(message, "| %d | 05 | 3 | 0%d.3 | 100 | 005 | Soft |", id, id % 4 + 4);
    }
    else{
        sprintf(message, "| %d | 05 | 3 | 0%d.3 | 100 | 060 | Soft |", id, id % 4 + 4);
    }
    write(carData.fifoInFD, message, 40);
    usleep(SHORT_DELAY);
    sprintf(message, "| %d | 07 | 3 | 1%d.%d | 100 | 060 | Soft |", id, (id + 7) % 4, id % 10);
    write(carData.fifoInFD, message, 40);
    usleep(SHORT_DELAY);
    // Leitura dos dados BOX
    // nao faz read de carro sem pneus
    if(id == 2 || id == 5 || id == 9)
        read(carData.fifoOutFD, boxMessage, 3);
    sprintf(message, "| %d | 09 | 3 | %d.%d | 100 | 060 | Soft |", id, (id + 2) % 10 + 14, id % 10);
    write(carData.fifoInFD, message, 40);
    usleep(SHORT_DELAY);
    close(carData.fifoInFD); 
    close(carData.fifoOutFD);
    return;
}

void SendMessageTest61(CarData carData) {
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

void SendMessageTest62(CarData carData, Organizacao * org) {
    char message[41];
    char boxMessage[3];
    int id = carData.id;
    // Aqui envia dados que requerem mensagem BOX
    if (id == 1 || id == 3 || id == 8) {
        sprintf(message, "| %d | 05 | 3 | 0%d.3 | 100 | 005 | Soft |", id, id % 4 + 4);
    }
    else{
        sprintf(message, "| %d | 05 | 3 | 0%d.3 | 100 | 060 | Soft |", id, id % 4 + 4);
    }
    write(carData.fifoInFD, message, 40);
    usleep(SHORT_DELAY);
    sprintf(message, "| %d | 07 | 3 | 1%d.%d | 100 | 060 | Soft |", id, (id + 7) % 4, id % 10);
    write(carData.fifoInFD, message, 40);
    usleep(SHORT_DELAY);
    // Leitura dos dados BOX
    // nao faz read de carro sem pneus
    if(org[id-1].pneus_disponiveis >= 4 && (id == 1 || id == 3 || id == 8))
        read(carData.fifoOutFD, boxMessage, 3);
    sprintf(message, "| %d | 10 | 3 | %d.%d | 100 | 060 | Soft |", id, (id + 2) % 10 + 14, id % 10);
    write(carData.fifoInFD, message, 40);
    usleep(SHORT_DELAY);
    close(carData.fifoInFD); 
    close(carData.fifoOutFD);
    return;
}

void enviaMsg (int fd, int id, char* msg)
{
    char message[41]; // mensagem
    sprintf(message, "| %d %s", id, msg);
    write(fd, message, 40);
}

void sendMessageTest7(CarData carData[], Organizacao *org)
{
    char boxMessage[3] = "XXX";  // mensagem box

    //Volta 1
    for(int i = 0; i < NUM_CARROS; i++)
    {
        enviaMsg(carData[i].fifoInFD, i+1, "| 01 | 1 | 01.1 | 100 | 100 | Soft |");        
    }
    // Volta 2, sector 2
     enviaMsg(carData[0].fifoInFD, 1, "| 02 | 2 | 02.3 | 090 | 045 | Soft |");
     enviaMsg(carData[6].fifoInFD, 7, "| 02 | 2 | 02.7 | 080 | 066 | Soft |");
     enviaMsg(carData[7].fifoInFD, 8, "| 02 | 2 | 02.8 | 100 | 077 | Soft |");

     // Volta 3, carro 9 sem combustivel
     enviaMsg(carData[2].fifoInFD, 3, "| 03 | 3 | 13.1 | 044 | 045 | Soft |");
     enviaMsg(carData[8].fifoInFD, 9, "| 03 | 2 | 22.2 | 055 | 008 | Soft |");
     enviaMsg(carData[1].fifoInFD, 2, "| 03 | 1 | 14.5 | 033 | 077 | Soft |");

     // ler box de carro carro 6
     read(carData[8].fifoOutFD, boxMessage, 3);

     //Volta 4
     enviaMsg(carData[2].fifoInFD, 3, "| 04 | 2 | 33.3 | 055 | 090 | Soft |");
     enviaMsg(carData[0].fifoInFD, 1, "| 04 | 2 | 22.5 | 033 | 077 | Soft |");

     // Carros 4 e 5 sem pneus e muda nome Equipa! 
     setOrganizacao(&org[5], 6, "EquipaX", "Tareco", 555, 20);
     setOrganizacao(&org[6], 7, "EquipaX", "Bichano", 555, 20);

     // Volta 5 gasta pneus em carro 6, nao esperamos box
     enviaMsg(carData[3].fifoInFD, 4, "| 05 | 3 | 53.3 | 055 | 005 | Soft |");
     enviaMsg(carData[5].fifoInFD, 6, "| 05 | 2 | 62.2 | 055 | 005 | Soft |");
     enviaMsg(carData[0].fifoInFD, 1, "| 05 | 1 | 22.5 | 033 | 007 | Soft |");
     enviaMsg(carData[8].fifoInFD, 9, "| 05 | 1 | 22.2 | 055 | 080 | Hard |");

    // Volta 10 
     enviaMsg(carData[0].fifoInFD, 1, "| 10 | 3 | 45.2 | 011 | 011 | Soft |");     
     enviaMsg(carData[1].fifoInFD, 2, "| 10 | 3 | 75.2 | 011 | 011 | Soft |");     
     enviaMsg(carData[2].fifoInFD, 3, "| 10 | 3 | 25.2 | 011 | 011 | Soft |");     
     enviaMsg(carData[3].fifoInFD, 4, "| 10 | 3 | 25.3 | 011 | 011 | Soft |");
     enviaMsg(carData[4].fifoInFD, 5, "| 10 | 3 | 26.2 | 021 | 075 | Hard |");
     enviaMsg(carData[5].fifoInFD, 6, "| 10 | 3 | 27.2 | 044 | 073 | Soft |");
     enviaMsg(carData[6].fifoInFD, 7, "| 10 | 3 | 58.8 | 055 | 031 | Soft |");
     enviaMsg(carData[7].fifoInFD, 8, "| 10 | 3 | 28.8 | 055 | 031 | Soft |");
     enviaMsg(carData[8].fifoInFD, 9, "| 10 | 3 | 11.2 | 066 | 080 | Hard |");
}



void enviadespiste(int id, int masterFifoFd) {
    char message[10];
    sprintf(message, "despiste%d", id);
    write(masterFifoFd, message, 9);
    usleep(SHORT_DELAY);
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
    double *dep;

	/* Abre memoria partilhada */
    org = criaMemoriaPartilhada(MEM_PARTILHADA, sizeof(Organizacao)*NUM_CARROS);
    dep = criaMemoriaPartilhada(MEM_PARTILHADA2, sizeof(double));
    *dep = 1000.0;
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
    write(masterFifoFd, "Partida!!", 9);
    switch (testID)
    {
    case 1:
        printf("Launching script for test 2.1\n");
        criaOrganizacaoT1T4T7(org); // dados mem partilhada
        *dep = 1000;
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
        criaOrganizacaoT2T3(org);
        for(int i = 0; i < NUM_CARROS; i++){
            SendMessageTest21(carData[idOrder[i]-1], idOrder[i]);
        }
        for(int i = 0; i < NUM_CARROS; i++){
            SendMessageTest22(carData[idOrder[i]-1], idOrder[i]);
        }
        break;
    case 3:
        printf("Launching script for test 2.3\n");
        criaOrganizacaoT2T3(org);
        for(int i = 0; i < NUM_CARROS; i++){
            SendMessageTest31(carData[idOrder[i]-1], idOrder[i]);
        }
        for(int i = 0; i < NUM_CARROS; i++){
            SendMessageTest32(carData[idOrder[i]-1], idOrder[i]);
        }
        enviadespiste(7, masterFifoFd);
        for(int i = 0; i < NUM_CARROS; i++){
            SendMessageTest33(carData[idOrder[i]-1], idOrder[i]);
        }
        break;
    case 4:
        printf("Launching script for test 2.4\n");
        criaOrganizacaoT1T4T7(org);
        for(int i = 0; i < NUM_CARROS; i++){
            SendMessageTest41(carData[idOrder2[i]-1], idOrder2[i]);
            if(i == 2 || i == 6) {
                kill(serverPID, SIGUSR1);
                sleep(1);
            } else if(i == 4 || i == 8) {
                kill(serverPID, SIGUSR2);
                sleep(1);
            }
        }
        for (int i = 0; i < NUM_CARROS; i++) {
            SendMessageTest42(carData[idOrder2[i]-1], idOrder2[i]);
        }
        break;
    case 5: 
        printf("Launching script for test 2.5\n");
        criaOrganizacaoT5(org);
        for(int i = 0; i < NUM_CARROS; i++){
            SendMessageTest51(carData[idOrder[i]-1]);
        }
        for(int i = 0; i < NUM_CARROS; i++){
            SendMessageTest52(carData[idOrder2[i]-1]);
        }
        enviadespiste(2, masterFifoFd);
        enviadespiste(4, masterFifoFd);
        break;
    case 6: 
        criaOrganizacaoT6(org);
        printf("Launching script for test 2.6\n");
        for(int i = 0; i < NUM_CARROS; i++){
            SendMessageTest61(carData[idOrder[i]-1]);
            if(i == 1 || i == 5) {
                kill(serverPID, SIGUSR1);
                sleep(1);
            } else if(i == 2 || i == 6) {
                kill(serverPID, SIGUSR2);
                sleep(1);
            }
        }
        for(int i = 0; i < NUM_CARROS; i++){
            SendMessageTest62(carData[idOrder[i]-1], org);
        }
        break;
    case 7: // termina abruptamente com msg box e mem partilhada dinamica
        criaOrganizacaoT1T4T7(org);
        sendMessageTest7(carData, org);
        break;

    default:
        printf("Undefined argument test ID\n");
        return -1;
    }
    system("rm CAR*");
    system("rm MASTER.in");
    return 0;
}
