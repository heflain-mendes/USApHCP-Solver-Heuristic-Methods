#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// Para compilar no visual studio
#define _CRT_SECURE_NO_WARNINGS

//Constantes
#define MAX_NODE 200
#define ALPHA 0.75

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

// Variáveis
char instanceFile[] = "instances/inst10.txt";
int qtdHubs = 2;
InstanceEntries instanceEntries;
double costMatrix[MAX_NODE][MAX_NODE];
int solution[MAX_NODE];
double rk[MAX_NODE];
double FO;
int maioresCustos[MAX_NODE][2]; // 0 - nó; 1 - custo

// Funções
int readInstance();
int calcCostMatriz();
int calcRk();
int calcFO();
int heuristic();

int main()
{
    /*solution[0] = 2;
    solution[1] = 2;
    solution[2] = 2;
    solution[3] = 4;
    solution[4] = 4;
    solution[5] = 2;
    solution[6] = 4;
    solution[7] = 4;
    solution[8] = 4;
    solution[9] = 4;*/

    readInstance();
    calcCostMatriz();
    heuristic();
    calcFO();
}

int readInstance()
{
    // Inicialização padrão
    instanceEntries.nodeQuantity = 0;

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
            free(instanceEntries.coordinates);
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
        if (costMatrix[k][solution[k]] > rk[solution[k]]) {
            rk[solution[k]] = costMatrix[k][solution[k]];
        }
    }

    return 0;
}

int calcFO() {
    calcRk();
    FO = 0;
    double calc;

    for (int m = 0; m < instanceEntries.nodeQuantity; m++) {
        for (int k = 0; k <= m; k++) {
            calc = rk[k] + rk[m] + ALPHA * costMatrix[k][m];

            if (calc > FO) {
                FO = calc;
            }
        }
    }

    printf("\n%lf\n", FO);

    return 0;
}

int comparador(const void* a, const void* b) {
    // Acessa os custos dos dois elementos
    int custoA = ((int*)a)[1];
    int custoB = ((int*)b)[1];

    // Ordena de forma crescente pelo custo
    if (custoA < custoB) return -1;
    if (custoA > custoB) return 1;
    return 0;
}

int getHubMenorCusto(int no) {
    int hub = maioresCustos[0][0];

    for (int k = 1; k < qtdHubs; k++) {
        if (costMatrix[no][maioresCustos[k][0]] < costMatrix[no][hub]) {
            hub = maioresCustos[k][0];
        }
    }

    return hub;
}

int heuristic() {
    // Encontrando os maiores custo para cada nó
    for (int k = 0; k < instanceEntries.nodeQuantity; k++) {
        maioresCustos[k][0] = k;
        maioresCustos[k][1] = costMatrix[0][k];
        
        for (int i = 1; i < instanceEntries.nodeQuantity; i++) {
            if (maioresCustos[k][1] < costMatrix[i][k]) {
                maioresCustos[k][1] = costMatrix[i][k];
            }
        }
    }

    // Ordenando os maiores custos para cada destino de forma crescente
    qsort(maioresCustos, instanceEntries.nodeQuantity, sizeof(maioresCustos[0]), comparador);

    // Zerando a solução
    memset(solution, -1, sizeof(solution));

    // Definindo os nós que serão hubs, com base no menor custo
    for (int h = 0; h < qtdHubs; h++) {
        solution[maioresCustos[h][0]] = maioresCustos[h][0]; 
    }

    // Conectando os nós aos hubs
    for (int i = 0; i < instanceEntries.nodeQuantity; i++) {
        if (solution[i] == -1) {
            solution[i] = getHubMenorCusto(i);
        }
    }

    return 0;
}