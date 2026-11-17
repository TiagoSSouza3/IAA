# IAA
EP3


code:
```c
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Estrutura para representar uma cidade
typedef struct {
    double x;
    double y;
} City;

// Estrutura para representar uma aresta
typedef struct {
    int u;
    int v;
    double cost;
} Edge;

// Compara duas arestas para o qsort
int compare_edges(const void *a, const void *b) {
    const Edge *edgeA = a;
    const Edge *edgeB = b;

    if (edgeA->cost < edgeB->cost) return -1;
    if (edgeA->cost > edgeB->cost) return 1;

    // Desempate determinístico
    if (edgeA->u != edgeB->u)
        return (edgeA->u > edgeB->u) - (edgeA->u < edgeB->u);

    return (edgeA->v > edgeB->v) - (edgeA->v < edgeB->v);
}

// Calcula a distância euclidiana entre duas cidades
double euclidean_distance(City a, City b) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;

    return sqrt(dx * dx + dy * dy);
}

// Encontra o representante de um conjunto
// Utiliza compressão de caminho
int find(int parent[], int i) {
    if (parent[i] != i)
        parent[i] = find(parent, parent[i]);

    return parent[i];
}

// Une dois conjuntos utilizando rank
void union_sets(int parent[], int rank[], int root_u, int root_v) {
    if (rank[root_u] < rank[root_v]) {
        parent[root_u] = root_v;
    }
    else if (rank[root_u] > rank[root_v]) {
        parent[root_v] = root_u;
    }
    else {
        parent[root_v] = root_u;
        rank[root_u]++;
    }
}

// Gera todas as arestas possíveis
Edge *generate_edges(City cities[], int n, int *edge_count) {
    int max_edges = n * (n - 1) / 2;

    Edge *edges = malloc((size_t)max_edges * sizeof(Edge));

    if (edges == NULL)
        return NULL;

    *edge_count = 0;

    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            edges[*edge_count].u = i;
            edges[*edge_count].v = j;
            edges[*edge_count].cost =
                euclidean_distance(cities[i], cities[j]);

            (*edge_count)++;
        }
    }

    return edges;
}

// Implementa a heurística Greedy por Arestas
int tsp_cheapest_greedy(int n, Edge edges[],
                        int edge_count, Edge tour[],
                        double *total_cost) {

    int *degree = calloc((size_t)n, sizeof(int));
    int *parent = malloc((size_t)n * sizeof(int));
    int *rank = calloc((size_t)n, sizeof(int));

    if (degree == NULL || parent == NULL || rank == NULL) {
        free(degree);
        free(parent);
        free(rank);
        return 0;
    }

    for (int i = 0; i < n; i++)
        parent[i] = i;

    int tour_count = 0;
    *total_cost = 0.0;

    // Ordena as arestas pelo menor custo
    qsort(edges, (size_t)edge_count, sizeof(Edge), compare_edges);

    // Percorre as arestas em ordem crescente de custo
    for (int i = 0; i < edge_count; i++) {
        int u = edges[i].u;
        int v = edges[i].v;

        // Cada cidade pode ter no máximo duas conexões
        if (degree[u] >= 2 || degree[v] >= 2)
            continue;

        int root_u = find(parent, u);
        int root_v = find(parent, v);

        // Aceita se conecta componentes diferentes
        // ou se é a última aresta necessária para fechar o ciclo
        if (root_u != root_v || tour_count == n - 1) {

            tour[tour_count] = edges[i];
            tour_count++;

            *total_cost += edges[i].cost;

            degree[u]++;
            degree[v]++;

            if (root_u != root_v)
                union_sets(parent, rank, root_u, root_v);

            if (tour_count == n)
                break;
        }
    }

    free(degree);
    free(parent);
    free(rank);

    // Verifica se todas as arestas do ciclo foram selecionadas
    return tour_count == n;
}

// Reconstrói a ordem das cidades a partir das arestas escolhidas
int reconstruct_tour(int n, Edge tour[], int route[]) {
    int *visited = calloc((size_t)n, sizeof(int));

    if (visited == NULL)
        return 0;

    int current = 0;
    route[0] = current;
    visited[current] = 1;

    for (int i = 1; i < n; i++) {
        int next = -1;

        for (int j = 0; j < n; j++) {
            if (visited[j])
                continue;

            for (int k = 0; k < n; k++) {
                if ((tour[k].u == current && tour[k].v == j) ||
                    (tour[k].v == current && tour[k].u == j)) {
                    next = j;
                    break;
                }
            }

            if (next != -1)
                break;
        }

        if (next == -1) {
            free(visited);
            return 0;
        }

        route[i] = next;
        visited[next] = 1;
        current = next;
    }

    free(visited);
    return 1;
}

int main(void) {
    int n;

    printf("Quantidade de cidades: ");

    if (scanf("%d", &n) != 1 || n < 3) {
        printf("Quantidade de cidades invalida.\n");
        return 1;
    }

    City *cities = malloc((size_t)n * sizeof(City));

    if (cities == NULL) {
        printf("Erro ao alocar memoria para as cidades.\n");
        return 1;
    }

    // Leitura das coordenadas
    for (int i = 0; i < n; i++) {
        printf("Coordenadas da cidade %d (x y): ", i);

        if (scanf("%lf %lf", &cities[i].x, &cities[i].y) != 2 ||
            !isfinite(cities[i].x) || !isfinite(cities[i].y)) {
            printf("Coordenadas invalidas.\n");
            free(cities);
            return 1;
        }
    }

    // Geração das arestas
    int edge_count;

    Edge *edges = generate_edges(cities, n, &edge_count);

    if (edges == NULL) {
        printf("Erro ao gerar arestas.\n");
        free(cities);
        return 1;
    }

    // Vetor para armazenar as arestas escolhidas
    Edge *tour = malloc((size_t)n * sizeof(Edge));
    int *route = malloc((size_t)n * sizeof(int));

    if (tour == NULL || route == NULL) {
        printf("Erro ao alocar memoria para a rota.\n");
        free(cities);
        free(edges);
        free(tour);
        free(route);
        return 1;
    }

    double total_cost;

    // Executa o algoritmo Greedy
    if (!tsp_cheapest_greedy(n, edges, edge_count,
                             tour, &total_cost)) {
        printf("Nao foi possivel construir um ciclo Hamiltoniano.\n");

        free(cities);
        free(edges);
        free(tour);
        free(route);
        return 1;
    }

    // Reconstrói a rota
    if (!reconstruct_tour(n, tour, route)) {
        printf("Erro ao reconstruir a rota.\n");

        free(cities);
        free(edges);
        free(tour);
        free(route);
        return 1;
    }

    // Exibe os resultados
    printf("\n--- Resultado Greedy ---\n");

    printf("Rota: ");

    for (int i = 0; i < n; i++)
        printf("%d -> ", route[i]);

    printf("%d\n", route[0]);

    printf("Custo total: %.6f\n", total_cost);

    // Libera a memória
    free(cities);
    free(edges);
    free(tour);
    free(route);

    return 0;
}
```
