#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

// Para compilar no visual studio
#define _CRT_SECURE_NO_WARNINGS

//Constantes
#define TAM_PATH 100 
#define MAX_NODE 200
#define MAX_HUBS 20
#define ALPHA 0.75
#define MAX_TIME 300

// Structs
typedef struct
{
    double x;
    double y;
} Coordinate;

typedef struct
{
    int nodeQuantity;
    Coordinate coordinates[MAX_NODE];
} InstanceEntries;

typedef struct
{
    int node;
    double cost;
} HigherCost;

typedef struct
{
    int allocations[MAX_NODE];
    double FO;
} Solution;

// Váriaveis definida pelo usuário
int amountInstances = 7;
int amountIteration = 3;
char instances[][TAM_PATH] = {"inst10", "inst20", "inst25", "inst40", "inst50", "inst100", "inst200" };
int amountHubsInstance[] = { 2, 2, 3, 4, 4, 10, 10 };

char pathInstace[] = "Instances";
char pathSolution[] = "Solutions";

int hubChangePercentage = 26;
double T0 = 4366.7713;
double Tc = 0.01;
double alpha = 0.9266;
int SAMax = 9565;

// Variáveis do programa
InstanceEntries instanceEntries;
double costMatrix[MAX_NODE][MAX_NODE];
double rk[MAX_NODE];
int hubs[MAX_HUBS];
HigherCost higherCost[MAX_NODE];
Solution solution;
double timeGetSolution;

char instanceFile[TAM_PATH];
char solutionFile[TAM_PATH];

int amountHubs;
int iteration;
int SAMaxCalc;

// Funções
int readInstance();
int calcCostMatriz();
int calcRk();
int calcFO();
int heuristic();
int changeHub();
int changeNode();
int generateNeighbor();
int simulatedAnnealing();
int readParameter(int argc, char* argv[]);
int printSolution();

int main(int argc, char* argv[])
{
    for (int inst = 0; inst < amountInstances; inst++) {
        memset(instanceFile, '\0', TAM_PATH);
        snprintf(instanceFile, sizeof(instanceFile), "%s/%s.txt", pathInstace, instances[inst]);

        amountHubs = amountHubsInstance[inst];

        readInstance();
        SAMaxCalc = instanceEntries.nodeQuantity * SAMax;
        calcCostMatriz();

        for (iteration = 1; iteration <= amountIteration; iteration++) {

            srand(time(NULL));

            memset(solutionFile, '\0', TAM_PATH);
            snprintf(solutionFile, sizeof(solutionFile), "%s/%s_%d.txt", pathSolution, instances[inst], iteration);

            simulatedAnnealing();
            printSolution();
        }
    }
}

int printSolution() {
    FILE* arq = fopen(solutionFile, "w");

    if (arq == NULL) {  // Verifica se o arquivo abriu corretamente
        perror("Erro ao abrir o arquivo");
        return 1; // Retorna erro
    }

    fprintf(arq, "FO: %.2lf\n", solution.FO);
    fprintf(arq, "TIME (seg): %.2lf\n", timeGetSolution);
    fprintf(arq, "\nALLOCATIONS\n");

    for (int i = 0; i < instanceEntries.nodeQuantity; i++) {
        fprintf(arq, "%d %d\n", i + 1, solution.allocations[i] + 1);
    }

    fclose(arq);

    return 0;
}

int readParameter(int argc, char* argv[]) {
    char* endptr;

    // SEED
    if (argc < 3) {
        fprintf(stderr, "Erro: Número insuficiente de argumentos.\n");
        return 1;
    }

    /*seed = (int)strtod(argv[3], &endptr);
    if (*endptr != '\0') {
        fprintf(stderr, "Erro ao ler a seed\n");
        return 1;
    }*/

    // INSTANCE
    if (argc < 4) {
        fprintf(stderr, "Erro: Argumento da instância não fornecido.\n");
        return 1;
    }

    char* separator = strchr(argv[4], '_');
    if (separator == NULL) {
        fprintf(stderr, "Erro: Formato inválido da instância.\n");
        return 1;
    }

    size_t pathLength = separator - argv[4];
    strncpy(instanceFile, argv[4], pathLength);
    instanceFile[pathLength] = '\0';

    char* hubStr = separator + 1;
    amountHubs = (int)strtod(hubStr, &endptr);
    if (*endptr != '\0') {
        fprintf(stderr, "Erro ao ler o número de hubs\n");
        return 1;
    }

    // Percorre os argumentos opcionais
    for (int i = 5; i < argc - 1; i += 2) {  // Começa em 5 para evitar sobrescrever `argv[4]`
        if (argv[i + 1] == NULL) {
            fprintf(stderr, "Erro: Argumento esperado após '%s'.\n", argv[i]);
            return 1;
        }

        char* endptr;
        double value = strtod(argv[i + 1], &endptr);

        if (endptr == argv[i + 1] || *endptr != '\0') {
            fprintf(stderr, "Erro ao ler o valor de '%s': '%s' não é válido.\n", argv[i], argv[i + 1]);
            return 1;
        }

        if (!strcmp(argv[i], "--HCP")) {
            hubChangePercentage = (int)value;
        }
        else if (!strcmp(argv[i], "--T0")) {
            T0 = value;
        }
        else if (!strcmp(argv[i], "--TC")) {
            Tc = value;
        }
        else if (!strcmp(argv[i], "--ALPHA")) {
            alpha = value;
        }
        else if (!strcmp(argv[i], "--SAMAX")) {
            SAMax = (int)value;
        }
    }

    return 0;
}

int readInstance()
{
    // Inicialização padrão
    instanceEntries.nodeQuantity = 0;
    memset(instanceEntries.coordinates, 0, MAX_NODE);

    // Abrir o arquivo
    FILE* file = fopen(instanceFile, "r");
    if (file == NULL)
    {
        perror("Erro ao abrir o arquivo");
        printf("Caminho: %s\n", instanceFile);
        return 1;
    }

    // Ler a quantidade de hubs
    if (fscanf(file, "%d", &instanceEntries.nodeQuantity) != 1)
    {
        printf("Erro ao ler a quantidade de hubs no arquivo.\n");
        fclose(file);
        return 1;
    }

    // Ler as coordenadas
    for (int i = 0; i < instanceEntries.nodeQuantity; i++)
    {
        if (fscanf(file, "%lf %lf", &instanceEntries.coordinates[i].x, &instanceEntries.coordinates[i].y) != 2)
        {
            printf("Erro ao ler as coordenadas do hub %d.\n", i + 1);
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

int calcCostMatriz() {
    int size = instanceEntries.nodeQuantity; // Número de coordenadas
    Coordinate* coordinates = instanceEntries.coordinates;

    memset(costMatrix, 0, sizeof(costMatrix));

    // Preencher a matriz com os custos (distâncias)
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (i == j) {
                costMatrix[i][j] = 0.0; // Distância de um ponto para ele mesmo é 0
            }
            else {
                double dx = coordinates[i].x - coordinates[j].x;
                double dy = coordinates[i].y - coordinates[j].y;
                costMatrix[i][j] = sqrt(dx * dx + dy * dy); // Distância Euclidiana
            }
        }
    }

    return 0;
}

int calcRk() {
    memset(rk, 0, sizeof(rk)); // zerando o array rk

    for (int k = 0; k < instanceEntries.nodeQuantity; k++) {
        if (costMatrix[k][solution.allocations[k]] > rk[solution.allocations[k]]) {
            rk[solution.allocations[k]] = costMatrix[k][solution.allocations[k]];
        }
    }

    return 0;
}

int calcFO() {
    calcRk();
    solution.FO = 0;
    double calc;

    for (int m = 0; m < instanceEntries.nodeQuantity; m++) {
        for (int k = 0; k <= m; k++) {
            calc = rk[k] + rk[m] + ALPHA * costMatrix[k][m];

            if (calc > solution.FO) {
                solution.FO = calc;
            }
        }
    }

    return 0;
}

int OrdCoastCres(const void* a, const void* b) {
    // Acessa os custos dos dois elementos
    double custoA = ((HigherCost*)a)->cost;
    double custoB = ((HigherCost*)b)->cost;

    // Ordena de forma crescente pelo custo
    if (custoA < custoB) return -1;
    if (custoA > custoB) return 1;
    return 0;
}

int OrdNodeCres(const void* a, const void* b) {
    // Acessa os custos dos dois elementos
    double custoA = ((HigherCost*)a)->node;
    double custoB = ((HigherCost*)b)->node;

    // Ordena de forma crescente pelo custo
    if (custoA < custoB) return -1;
    if (custoA > custoB) return 1;
    return 0;
}

int getHubMenorCusto(int no) {
    int hub = higherCost[hubs[0]].node;

    for (int k = 1; k < amountHubs; k++) {
        if (costMatrix[no][higherCost[hubs[k]].node] < costMatrix[no][hub]) {
            hub = higherCost[hubs[k]].node;
        }
    }

    return hub;
}

int heuristic() {
    // Encontrando os maiores custo para cada nó
    for (int k = 0; k < instanceEntries.nodeQuantity; k++) {
        higherCost[k].node = k;
        higherCost[k].cost = costMatrix[0][k];

        for (int i = 1; i < instanceEntries.nodeQuantity; i++) {
            if (higherCost[k].cost < costMatrix[i][k]) {
                higherCost[k].cost = costMatrix[i][k];
            }
        }
    }

    // Ordenando os maiores custos para cada destino de forma crescente
    qsort(higherCost, instanceEntries.nodeQuantity, sizeof(higherCost[0]), OrdCoastCres);

    // Zerando a solução
    memset(solution.allocations, -1, sizeof(solution.allocations));

    // Definindo os nós que serão hubs, com base no menor custo
    for (int h = 0; h < amountHubs; h++) {
        solution.allocations[higherCost[h].node] = higherCost[h].node;
        hubs[h] = higherCost[h].node;
    }

    // Valta o array ao normal
    qsort(higherCost, instanceEntries.nodeQuantity, sizeof(higherCost[0]), OrdNodeCres);

    // Conectando os nós aos hubs
    for (int i = 0; i < instanceEntries.nodeQuantity; i++) {
        if (solution.allocations[i] == -1) {
            solution.allocations[i] = getHubMenorCusto(i);
        }
    }

    calcFO();

    return 0;
}

int isHub(int node) {
    for (int i = 0; i < amountHubs; i++)
    {
        if (hubs[i] == node) return 1;
    }

    return 0;
}

int changeHub() {
    int oldHub = rand() % amountHubs;
    int newHub = rand() % instanceEntries.nodeQuantity;

    do
    {
        newHub = rand() % instanceEntries.nodeQuantity;
    } while (isHub(newHub)); // loop até o newHub não ser um nó que está alocado como hub

    hubs[oldHub] = newHub;

    // Zerando a solução
    memset(solution.allocations, -1, sizeof(solution.allocations));

    // Definindo os nós que serão hubs
    for (int h = 0; h < amountHubs; h++) {
        solution.allocations[hubs[h]] = hubs[h];
    }

    // Conectando os nós aos hubs
    for (int i = 0; i < instanceEntries.nodeQuantity; i++) {
        if (solution.allocations[i] == -1) {
            solution.allocations[i] = getHubMenorCusto(i);
        }
    }

    return 0;
}

int changeNode() {
    int node;
    int newHub;

    do
    {
        node = rand() % instanceEntries.nodeQuantity;
    } while (isHub(node)); // loop até o node não ser um hub

    do
    {
        newHub = rand() % amountHubs;
    } while (hubs[newHub] == solution.allocations[node]);

    solution.allocations[node] = hubs[newHub];

    return 0;
}

int generateNeighbor() {
    if (rand() % 100 > hubChangePercentage) {
        changeNode();
    }
    else {
        changeHub();
    }

    calcFO();
    return 0;
}

int simulatedAnnealing() {
    clock_t hI, hF;

    hI = clock();

    heuristic();

    Solution bestSolution = solution;
    Solution lastSolution = solution;
    double T = T0;

    while (T > Tc) {
        for (int iterT = 0; iterT < SAMaxCalc; iterT++)
        {
            hF = clock();
            timeGetSolution = ((double)(hF - hI)) / CLOCKS_PER_SEC;
            if (timeGetSolution >= MAX_TIME) {
                break;
            }

            generateNeighbor();
            double delta = solution.FO - lastSolution.FO;

            if (delta < 0) {
                lastSolution = solution;
                if (solution.FO < bestSolution.FO) {
                    bestSolution = solution;
                }
            }
            else {
                double x = rand() % 1001;
                x = x / 1000.0;

                if (x < exp(-delta / T)) {
                    lastSolution = solution;
                }
            }
        }

        T = alpha * T;
    }

    solution = bestSolution;

    return 0;
}