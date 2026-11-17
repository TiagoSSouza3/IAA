# IAA
EP3


code:
```c
#include <stdio.h>
#include <stdlib.h>

// Estrutura para representar uma aresta (equivalente à tupla (cost, u, v))
typedef struct {
    int cost;
    int u;
    int v;
} Edge;

// Função de comparação para o qsort (ordena da menor distância para a maior)
int compare_edges(const void *a, const void *b) {
    Edge *edgeA = (Edge *)a;
    Edge *edgeB = (Edge *)b;
    return (edgeA->cost - edgeB->cost);
}

// Função Find do Union-Find com compressão de caminho
int find(int parent[], int i) {
    while (parent[i] != i) {
        parent[i] = parent[parent[i]];
        i = parent[i];
    }
    return i;
}

// Função principal da heurística Cheapest Greedy para o TSP
int tsp_cheapest_greedy(int n, int dist_matrix[n][n], Edge tour[]) {
    int max_edges = (n * (n - 1)) / 2;
    Edge *edges = (Edge *)malloc(max_edges * sizeof(Edge));
    int edge_count = 0;
    
    // 1. Lista todas as arestas possíveis (u, v) com seus custos
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            edges[edge_count].cost = dist_matrix[i][j];
            edges[edge_count].u = i;
            edges[edge_count].v = j;
            edge_count++;
        }
    }

    // 2. Ordena as arestas da mais barata para a mais cara
    qsort(edges, edge_count, sizeof(Edge), compare_edges);

    // Aloca e inicializa os vetores de grau e Union-Find
    int *degree = (int *)calloc(n, sizeof(int)); // Inicializa com 0
    int *parent = (int *)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        parent[i] = i;
    }

    int tour_count = 0;
    int total_cost = 0;

    // 3. Seleciona as arestas válidas
    for (int i = 0; i < edge_count; i++) {
        int cost = edges[i].cost;
        int u = edges[i].u;
        int v = edges[i].v;

        // Cada cidade pode ter no máximo 2 conexões
        if (degree[u] < 2 && degree[v] < 2) {
            int root_u = find(parent, u);
            int root_v = find(parent, v);

            // Evita fechar um ciclo antes de incluir todas as cidades (N - 1 arestas)
            if (root_u != root_v || tour_count == n - 1) {
                tour[tour_count] = edges[i];
                tour_count++;
                total_cost += cost;
                degree[u]++;
                degree[v]++;
                parent[root_u] = root_v;

                if (tour_count == n) {
                    break;
                }
            }
        }
    }

    // Libera a memória alocada dinamicamente
    free(edges);
    free(degree);
    free(parent);

    return total_cost;
}

int main() {
    int n = 4;
    // Exemplo com 4 cidades (A=0, B=1, C=2, D=3)
    int matriz[4][4] = {
        {0, 10, 15, 20},
        {10, 0, 35, 25},
        {15, 35, 0, 30},
        {20, 25, 30, 0}
    };

    // Array para armazenar as N arestas escolhidas na rota
    Edge rota[4];

    int custo = tsp_cheapest_greedy(n, matriz, rota);

    printf("Arestas escolhidas: [");
    for (int i = 0; i < n; i++) {
        printf("(%d, %d)%s", rota[i].u, rota[i].v, (i < n - 1) ? ", " : "");
    }
    printf("]\n");
    printf("Custo total: %d\n", custo);

    return 0;
}
```
