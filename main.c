#define FIFO_FILE "MYFIFO"
#define MEM_PARTILHADA "organizacao.shm"
#define COMPRIMENTO_MENSAGEM 40
#define TAMANHO_NOME 20
#define USER_LEVEL_PERMISSIONS 0666
#define TEM_ERRO -1
#define NUM_CARROS 9
#define MASTER_FIFO "MASTER.in"
#define COMPRIMENTO_MENSAGEM_MASTER 9

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
#include <errno.h>
#include <signal.h>
#include <sys/mman.h>
#include <semaphore.h>

typedef struct StatusCarro
{
    short id;  /* Numero de carro */
    int volta; /* Volta na pista */
    int setor;
    float tempo;
    double combustivel;
    int pneus;
    char tipo_pneu[TAMANHO_NOME]; /* tipo pneu */
} Status;

typedef struct OrganizacaoCorrida
{
    short id;                       /* Numero de carro */
    char nome_equipa[TAMANHO_NOME]; /* Nome da equipa do carro */
    char nome_piloto[TAMANHO_NOME]; /* Nome da equipa do carro */
    int id_equipa;                  /* Id unico da equipa*/
    int pneus_disponiveis;          /* pneus disponives */
} Organizacao;

void *criaMemoriaPartilhada(char *nome, int tamanho)
{
    void *ptr = NULL;
    int ret = 0;
    int fd = 0;

    /* Cria memoria partilhada */
    fd = shm_open(nome, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
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

void apagaMemoriaPartilhada(char *nome, int tamanho, void *ptr)
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
// função usada para encontrar e abrir um semáforo específico associado a uma equipa de corrida com base no ID do carro
sem_t *procuraSemEquipa(int id)
{
    // acessa a memória partilhada que contém informações sobre as organizações
    Organizacao *organizacao = criaMemoriaPartilhada(MEM_PARTILHADA, sizeof(Organizacao) * NUM_CARROS);
    // percorre a lista de organizações para encontrar a correspondente ao 'id' fornecido
    for (int i = 0; i < NUM_CARROS; i++)
    {
        // caso o 'id' da organização não corresponder, continua a busca
        if (organizacao[i].id != id)
            continue;
        // constrói o nome do semáforo baseado no ID da equipa e tenta abri-lo ou criá-lo se ainda não existir
        char semNome[TAMANHO_NOME];
        snprintf(semNome, sizeof(semNome), "/Equipa%d.sem", organizacao[i].id_equipa);
        return sem_open(semNome, O_CREAT, 0666, 1);
    }
    // senão encontrar uma equipe correspondente retorna NULL
    return NULL;
}
// função para buscar o número de pneus disponíveis para um carro
int buscaPneusDisponiveis(int id)
{
    // define o nome da memória partilhada a ser acedida
    char nome[TAMANHO_NOME] = MEM_PARTILHADA;
    // ponteiro para a estrutura de organização, que vai ser mapeada para a memória partilhada
    Organizacao *organizacao = NULL;
    // abre a memoria partilhada
    organizacao = criaMemoriaPartilhada(nome, sizeof(Organizacao) * NUM_CARROS);
    // vai iterar sobre todas as organizações para encontrar a correspondente ao ID do carro fornecido.
    for (int i = 0; i < NUM_CARROS; i++)
    {
        // caso o ID do carro na organização corresponda ao ID fornecido, retorna o número de pneus disponíveis
        if (id == organizacao[i].id)
        {
            return organizacao[i].pneus_disponiveis;
        }
    }

    // caso o ID do carro não for encontrado, imprime uma mensagem de erro e retorna -1
    printf("Pneus de Piloto nao foi encontrado");
    return -1;
}
// função responsável por encontrar o nome do piloto associado a um determinado carro na corrida
char *buscaNomePiloto(int id)
{
    // define o nome da memória partilhada a ser utilizada
    char nome[TAMANHO_NOME] = MEM_PARTILHADA;
    // ponteiro para a estrutura Organizacao, que será mapeada para a memória partilhada
    Organizacao *organizacao = NULL;
    // acessa a memória partilhada que contém as informações das equipas
    organizacao = criaMemoriaPartilhada(nome, sizeof(Organizacao) * NUM_CARROS);
    // procura na lista de organizações a que corresponde ao ID do carro fornecido
    for (int i = 0; i < NUM_CARROS; i++)
    {
        // se encontrar o carro com o ID correspondente, retorna o nome do piloto
        if (id == organizacao[i].id)
        {
            return organizacao[i].nome_piloto;
        }
    }

    // se o carro com o ID fornecido não for encontrado, imprime uma mensagem de erro e retorna NULL
    printf("Piloto nao foi encontrado");
    return NULL;
}
// função vai buscar o nome da equipa de um carro específico na corrida
char *buscaEquipa(int id)
{
    // define o nome da memória partilhada a ser utilizada
    char nome[TAMANHO_NOME] = MEM_PARTILHADA;
    // ponteiro para a estrutura Organizacao, que será mapeada para a memória partilhada
    Organizacao *organizacao = NULL;
    // acessa a memória partilhada que contém as informações das equipas
    organizacao = criaMemoriaPartilhada(nome, sizeof(Organizacao) * NUM_CARROS);

    // procura na lista de organizações a que corresponde ao ID do carro fornecido
    for (int i = 0; i < NUM_CARROS; i++)
    {
        // se encontrar o carro com o ID correspondente, retorna o nome da equipa
        if (id == organizacao[i].id)
        {
            return organizacao[i].nome_equipa;
        }
    }

    // se o carro com o ID fornecido não for encontrado, imprime uma mensagem de erro e retorna NULL
    printf("Equipa nao foi encontrada");
    return NULL;
}
// critério de comparação para ordenar um array de estruturas StatusCarro
int compara(const void *a, const void *b)
{
    // converte os ponteiros void para ponteiros da struct StatusCarro
    const struct StatusCarro *structA = (const struct StatusCarro *)a;
    const struct StatusCarro *structB = (const struct StatusCarro *)b;

    // compara os tempos dos carros: menor, maior ou igual
    if (structA->tempo < structB->tempo)
        return -1; // A é mais rápido que B
    if (structA->tempo > structB->tempo)
        return 1; // A é mais lento que B
    return 0;     // A e B têm o mesmo tempo
}
// imprime os resultados da corrida
void imprimeResultados(Status tab[])
{
    // acessa a memória partilhada para o total de combustível
    double *deposito = criaMemoriaPartilhada("combustivel.shm", sizeof(double));
    // ordena o array de carros com base no tempo usando a função 'compara'
    qsort(tab, NUM_CARROS, sizeof(Status), compara);
    // imprime o cabeçalho dos resultados
    printf("Posicao\tId\tNome\tEquipa\tVolta\tTempo\n");
    // itera e imprime detalhes de cada carro na corrida
    for (int t = 0; t < NUM_CARROS; t++)
    {
        printf("%d\t%d\t%s\t%s\t%d\t%.3f\n",
               t + 1,                      // posição na corrida
               tab[t].id,                  // ID do carro
               buscaNomePiloto(tab[t].id), // nome do piloto
               buscaEquipa(tab[t].id),     // nome da equipa
               tab[t].volta,               // número da volta
               tab[t].tempo);              // tempo total
    }
    // exibe o total de combustível no depósito
    printf("O deposito de reserva da corrida e %0.2f litros.", *deposito);
}
// responsável por escrever uma mensagem num FIFO específico
void escreveTC(int fd, const char *mensagem)
{
    // escreve a mensagem fornecida no FIFO identificado por 'fd'
    ssize_t bytes_written = write(fd, mensagem, strlen(mensagem));
    // verifica se a escrita no FIFO foi bem-sucedida
    if (bytes_written == -1)
    {
        // em caso de erro na escrita, exibe a mensagem de erro e fecha o FIFO
        perror("Erro ao escrever no fifo");
        close(fd);
        exit(EXIT_FAILURE); // encerra o programa indicando falha
    }
}
//  função usada para diminuir a quantidade de combustível num depósito compartilhado, representando o consumo de combustível durante a corrida
void tiraUmCentilitro(double *deposito)
{
    // pode abrir ou criar um semáforo para controlar o acesso ao recurso de reabastecimento
    sem_t *semaforoReabastecimento = sem_open("/Reabastecimento.sem", O_CREAT, 0666, 1);

    // espera até conseguir acesso ao semáforo de reabastecimento
    sem_wait(semaforoReabastecimento);

    // realiza a operação de decremento no valor do depósito de combustível
    double decremento = *deposito;
    decremento = decremento - 0.01; // diminui 0.01 litro do depósito
    *deposito = decremento;

    // libera o semáforo após a operação de decremento
    sem_post(semaforoReabastecimento);
}
// funçao que processa a telemetria de um carro em uma simulação de corrida, lê dados do carro (como combustível e pneus) de um FIFO, decide se o carro precisa ir ao box e usa semáforos para sincronização com outras partes da simulação.
void *lerTelemetria(void *carro)
{
    int fd = 0;                                                                   // identificador do FIFO de leitura
    char mensagem[COMPRIMENTO_MENSAGEM + 1];                                      // armazena a mensagem lida do FIFO
    char nome[TAMANHO_NOME] = MEM_PARTILHADA;                                     // nome da memória partilhada
    int tamanho_msg = 0;                                                          // tamanho em bytes da mensagem lida
    int fd2 = 0;                                                                  // identificador do FIFO de escrita
    Status *car_stat = (Status *)carro;                                           // converte o ponteiro para o tipo Status
    int escreveuMensagemBOX = 0;                                                  // flag para controle de mensagem de BOX
    sem_t *semMaster = sem_open("/InicioCorrida.sem", O_CREAT, 0666, NUM_CARROS); // semáforo para controle da corrida
    double *deposito = criaMemoriaPartilhada("combustivel.shm", sizeof(double));  // acesso ao depósito de combustível

    // cria e abre FIFO para escrita das mensagens
    sprintf(nome, "CARRO%d.out", car_stat->id + 1);
    if (mkfifo(nome, USER_LEVEL_PERMISSIONS) == TEM_ERRO)
    {
        printf("Erro ao criar FIFO %s\n", nome);
        pthread_exit(NULL);
    }
    fd2 = open(nome, O_WRONLY);
    if (fd2 == TEM_ERRO)
    {
        printf("Erro no OPEN de %s \n", nome);
        pthread_exit(NULL);
    }

    // cria e abre FIFO para leitura das mensagens
    sprintf(nome, "CARRO%d.in", car_stat->id + 1);
    if (mkfifo(nome, USER_LEVEL_PERMISSIONS) == TEM_ERRO)
    {
        printf("Erro ao criar FIFO %s\n", nome);
        pthread_exit(NULL);
    }
    fd = open(nome, O_RDONLY);
    if (fd == TEM_ERRO)
    {
        printf("Erro no OPEN de %s \n", nome);
        pthread_exit(NULL);
    }

    sem_t *semEquipa = procuraSemEquipa(car_stat->id); // semáforo para sincronização da equipe
    sem_wait(semMaster);                               // espera pelo início da corrida

    // processamento das mensagens de telemetria
    while (car_stat->volta != 10)
    {
        tamanho_msg = read(fd, mensagem, COMPRIMENTO_MENSAGEM);
        if (tamanho_msg == -1)
        {
            perror("Erro a ler do FIFO");
            close(fd);
            pthread_exit(NULL);
        }
        // adiciona um caractere nulo ao final da mensagem para formar uma string C válida
        mensagem[tamanho_msg] = '\0';
        // Verifica se a mensagem lida não é vazia
        if (tamanho_msg != 0)
        {
            // se uma mensagem para ir ao BOX foi enviada anteriormente, libera o semáforo da equipa
            if (escreveuMensagemBOX == 1 && semEquipa != NULL)
            {
                sem_post(semEquipa); // libera o semáforo da equipe para outras threads/processos
            }

            // lê os dados da telemetria da mensagem
            sscanf(mensagem, "| %hd | %d | %d | %f | %lf | %d | %s |",
                   &car_stat->id, &car_stat->volta, &car_stat->setor,
                   &car_stat->tempo, &car_stat->combustivel, &car_stat->pneus, car_stat->tipo_pneu);

            // verifica condições para ir ao BOX (pneus ou combustível baixos)
            if ((car_stat->pneus < 10) || (car_stat->combustivel < 10))
            {
                if (buscaPneusDisponiveis(car_stat->id) < 4)
                {
                    printf("Carro %d nao tem pneus disponiveis\n", car_stat->id);
                }
                else
                {
                    if (semEquipa != NULL)
                    {
                        sem_wait(semEquipa);
                    }

                    // envia carro para o BOX
                    escreveTC(fd2, "BOX");
                    escreveuMensagemBOX = 1;

                    // reabastece o carro
                    while (car_stat->combustivel < 100.00)
                    {
                        tiraUmCentilitro(deposito);
                        car_stat->combustivel += 0.01;
                    }
                }
            }
        }
    }

    // finaliza o processamento para este carro
    printf("Corrida terminou para carro numero %d\n", car_stat->id);

    // fecha e remove os FIFOs
    close(fd2);
    close(fd);
    sprintf(nome, "CARRO%d.out", car_stat->id + 1);
    unlink(nome);
    sprintf(nome, "CARRO%d.in ", car_stat->id + 1);
    unlink(nome);
    pthread_exit(NULL);
}
// função que inicializa uma tabela de status para cada carro participante da corrida
void initTabela(Status tab[])
{
    // itera sobre cada carro na tabela de status
    for (int t = 0; t < NUM_CARROS; t++)
    {
        // define o ID do carro (começa de 0 e vai até NUM_CARROS - 1)
        tab[t].id = t;
        // inicializa o número da volta em que o carro está como 0
        tab[t].volta = 0;
        // inicializa o setor em que o carro está como 0
        tab[t].setor = 0;
        // inicializa o tempo total do carro na corrida como 0.0
        tab[t].tempo = 0.0;
        // inicializa o nível de combustível do carro como 0
        tab[t].combustivel = 0;
        // inicializa o estado dos pneus do carro como 0
        tab[t].pneus = 0;
        // limpa a string do tipo de pneu, definindo todos os caracteres para 0
        memset(tab[t].tipo_pneu, 0, 5);
    }
}
// thread principal na simulação da corrida, responsável por controlar o estado geral da corrida e responder a eventos específicos
void *threadMaster(void *threads_arg)
{
    // inicialização de variáveis e associação de argumentos de thread
    char nome[TAMANHO_NOME];
    pthread_t *threads = (pthread_t *)threads_arg; // array de threads para cada carro
    char mensagem[COMPRIMENTO_MENSAGEM_MASTER]; // buffer para mensagens recebidas
    int fd; // file descriptor para o FIFO
    int comecouCorrida = 0; // flag para indicar se a corrida começou

    // cria um FIFO chamado 'MASTER_FIFO' para comunicação de controle da corrida
    if (mkfifo(MASTER_FIFO, USER_LEVEL_PERMISSIONS) == TEM_ERRO)
    {
        printf("Erro ao criar FIFO %s\n", MASTER_FIFO);
        pthread_exit(NULL); // Encerra a thread em caso de erro
    }

    // abre o FIFO para leitura
    fd = open(MASTER_FIFO, O_RDONLY);

    // loop principal da thread master
    while (1)
    {
        // lê mensagens do FIFO
        int tamanho_msg = read(fd, mensagem, COMPRIMENTO_MENSAGEM_MASTER);
        if (tamanho_msg == -1)
        {
            perror("Erro a ler do FIFO");
            close(fd);
            pthread_exit(NULL); // encerra a thread em caso de erro na leitura
        }

        // certifica-se de que a mensagem é uma string C válida
        mensagem[tamanho_msg] = '\0';

        // processa a mensagem recebida
        if (tamanho_msg != 0)
        {
            // se a mensagem for "Partida!!", inicia a corrida
            if (strcmp(mensagem, "Partida!!") == 0)
            {
                // abre ou cria um semáforo para controlar o início da corrida
                sem_t *semMASTER = sem_open("/InicioCorrida.sem", O_CREAT, 0666, NUM_CARROS);
                printf("A corrida foi iniciada.\n");
                comecouCorrida = 1;

                // libera o semáforo para todas as threads dos carros, iniciando a corrida
                for (int i = 0; i < NUM_CARROS; i++)
                {
                    sem_post(semMASTER);
                }
            }
            else if (comecouCorrida == 1) // se a corrida já começou, processa outras mensagens
            {
                // trata mensagens de despiste de carro
                int car_id = 0;
                sscanf(mensagem, "despiste%d", &car_id);
                printf("Carro %d despistou-se\n", car_id);

                // cancela a thread do carro que despistou
                pthread_cancel(threads[car_id - 1]);

                // remove os FIFOs associados ao carro despistado
                sprintf(nome, "CARRO%d.out", car_id);
                unlink(nome);
                sprintf(nome, "CARRO%d.in ", car_id);
                unlink(nome);
            }
        }
    }
}
// função responsável por encerrar a corrida antes do seu término natural por exemplo quando um limite de tempo é atingido
void terminaCorridaAbruptamente()
{
    char nome[TAMANHO_NOME]; // buffer para armazenar nomes de FIFO
    char semNomeTeam[TAMANHO_NOME]; // buffer para armazenar nomes de semáforos
    // imprime uma mensagem indicando o fim antecipado da corrida
    printf("Fim da corrida por tempo esgotado\n");
    // loop para limpar recursos para cada carro
    for (int i = 0; i < NUM_CARROS; i++)
    {
        // remove os FIFOs associados a cada carro
        sprintf(nome, "CARRO%d.out", i + 1);
        unlink(nome); // Remove o FIFO de saída
        sprintf(nome, "CARRO%d.in", i + 1);
        unlink(nome); // Remove o FIFO de entrada

        // remove o semáforo associado a cada equipe
        sprintf(semNomeTeam, "/Equipa%d.sem", i + 1);
        sem_unlink(semNomeTeam); // Desvincula o semáforo da equipe
    }

    // remove o FIFO principal usado pela thread master
    unlink(MASTER_FIFO);

    // desvincula os semáforos globais de início de corrida e reabastecimento
    sem_unlink("/InicioCorrida.sem");
    sem_unlink("/Reabastecimento.sem");

    // encerra o programa
    exit(0);
}

void bandeiraAmarelaLevantada()
{
    printf("Bandeira amarela levantada\n");
}

void bandeiraAmarelaRecolhida()
{
    printf("Bandeira amarela recolhida\n");
}

int main(int argc, char **argv)
{
    // declaração de variáveis
    char semNomeTeam[TAMANHO_NOME];
    sem_t *sems[NUM_CARROS];
    pthread_t threads[NUM_CARROS];
    Status tabela[NUM_CARROS];
    char comando[COMPRIMENTO_MENSAGEM];
    int rc;

    // inicialização e limpeza de semáforos para cada equipe
    for (int i = 0; i < NUM_CARROS; i++)
    {
        sprintf(semNomeTeam, "/Equipa%d.sem", i + 1);
        sem_unlink(semNomeTeam); // Remove um semáforo existente
        sems[i] = sem_open(semNomeTeam, O_CREAT, 0666, 1); // Cria um novo semáforo
    }

    // remove semáforos existentes
    sem_unlink("/InicioCorrida.sem");
    sem_unlink("/Reabastecimento.sem");

    // configuração de sinais e alarme para eventos da corrida
    signal(SIGUSR1, bandeiraAmarelaLevantada);
    signal(SIGUSR2, bandeiraAmarelaRecolhida);
    signal(SIGALRM, terminaCorridaAbruptamente);
    alarm(15); // define um alarme para terminar a corrida após 15 segundos

    // executa comando externo se um argumento for fornecido
    if (argc >= 2)
    {
        sprintf(comando, "bash testscript.sh %d %s 2>&1", getpid(), argv[1]);
        FILE *pipe = popen(comando, "r");
        if (!pipe)
        {
            perror("Erro ao executar comando:");
            return 1; // encerra a função em caso de erro
        }
    }

    // cria semáforos
    sem_t *semReabastecimento = sem_open("/Reabastecimento.sem", O_CREAT, 0666, 1);
    sem_t *semMaster = sem_open("/InicioCorrida.sem", O_CREAT, 0666, NUM_CARROS);

    // criação da thread master para controlar a corrida
    pthread_t master;
    rc = pthread_create(&master, NULL, threadMaster, (void *)threads);
    if (rc)
    {
        fprintf(stderr, "Erro ao criar thread master; erro %d\n", rc);
        return 1; // encerra a função em caso de erro
    }

    // criação de threads para cada carro
    for (int t = 0; t < NUM_CARROS; t++)
    {
        initTabela(tabela); // inicializa a tabela de status do carro
        rc = pthread_create(&threads[t], NULL, lerTelemetria, (void *)&tabela[t]);
        if (rc)
        {
            fprintf(stderr, "Erro ao criar thread %d; erro %d\n", t, rc);
            return 1; // encerra a função em caso de erro
        }
    }

    // espera todas as threads dos carros terminarem
    for (int t = 0; t < NUM_CARROS; t++)
    {
        rc = pthread_join(threads[t], NULL);
        if (rc)
        {
            fprintf(stderr, "Erro no join da thread %d; erro %d\n", t, rc);
            return 1; // encerra a função em caso de erro
        }
    }

    // finalização e limpeza
    imprimeResultados(tabela); // imprime os resultados da corrida
    pthread_cancel(master); // cancela a thread master
    unlink(MASTER_FIFO); // remove o FIFO master
    sem_unlink("/InicioCorrida.sem"); // remove o semáforo de início da corrida
    sem_unlink("/Reabastecimento.sem"); // remove o semáforo de reabastecimento
    for (int i = 0; i < NUM_CARROS; i++)
    {
        sprintf(semNomeTeam, "/Equipa%d.sem", i + 1);
        sem_unlink(semNomeTeam); // Remove os semáforos das equipes
    }

    sleep(1); // Espera curta para garantir que todas as operações de limpeza sejam concluídas
    return 0; // Encerra o programa
}