
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// Limite para executar a força bruta.
// O Greedy continua podendo executar com mais cidades.
#define MAX_BRUTE_FORCE_CITIES 12

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

// Calcula a distância euclidiana
double euclidean_distance(City a, City b) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;

    return sqrt(dx * dx + dy * dy);
}

// Find com compressão de caminho
int find(int parent[], int i) {
    if (parent[i] != i)
        parent[i] = find(parent, parent[i]);

    return parent[i];
}

// Union com rank
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
    size_t max_edges = (size_t)n * (n - 1) / 2;

    Edge *edges = malloc(max_edges * sizeof(Edge));

    if (edges == NULL)
        return NULL;

    *edge_count = 0;

    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            double distance = euclidean_distance(cities[i], cities[j]);

            if (!isfinite(distance)) {
                free(edges);
                return NULL;
            }

            edges[*edge_count].u = i;
            edges[*edge_count].v = j;
            edges[*edge_count].cost = distance;

            (*edge_count)++;
        }
    }

    return edges;
}

// Algoritmo Greedy por Arestas
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

    qsort(edges, (size_t)edge_count, sizeof(Edge), compare_edges);

    for (int i = 0; i < edge_count; i++) {
        int u = edges[i].u;
        int v = edges[i].v;

        if (degree[u] >= 2 || degree[v] >= 2)
            continue;

        int root_u = find(parent, u);
        int root_v = find(parent, v);

        // Conecta componentes diferentes ou fecha o ciclo final
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

    return tour_count == n;
}

// Reconstrói a rota a partir das arestas escolhidas
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

// --------------------------------------------------
// FORÇA BRUTA
// --------------------------------------------------

// Variáveis da busca recursiva
void brute_force_search(
    City cities[],
    int n,
    int route[],
    int best_route[],
    int visited[],
    int position,
    double current_cost,
    double *best_cost
) {
    // Poda: não precisamos continuar se já ultrapassamos
    // o melhor custo encontrado
    if (current_cost >= *best_cost)
        return;

    // Todas as cidades foram visitadas
    if (position == n) {
        double total = current_cost +
            euclidean_distance(cities[route[n - 1]], cities[0]);

        if (total < *best_cost) {
            *best_cost = total;

            for (int i = 0; i < n; i++)
                best_route[i] = route[i];
        }

        return;
    }

    // Testa todas as cidades ainda não visitadas
    for (int city = 1; city < n; city++) {

        if (visited[city])
            continue;

        // Elimina a orientação inversa da mesma rota.
        // A última cidade deve ter índice maior que a segunda.
        if (position == n - 1 && city <= route[1])
            continue;

        double new_cost = current_cost +
            euclidean_distance(cities[route[position - 1]], cities[city]);

        visited[city] = 1;
        route[position] = city;

        brute_force_search(
            cities,
            n,
            route,
            best_route,
            visited,
            position + 1,
            new_cost,
            best_cost
        );

        // Desfaz a escolha para testar outra possibilidade
        visited[city] = 0;
    }
}

// Executa a força bruta
int tsp_brute_force(
    City cities[],
    int n,
    int best_route[],
    double *best_cost
) {
    int *route = malloc((size_t)n * sizeof(int));
    int *visited = calloc((size_t)n, sizeof(int));

    if (route == NULL || visited == NULL) {
        free(route);
        free(visited);
        return 0;
    }

    // A cidade 0 será sempre o ponto inicial
    route[0] = 0;
    visited[0] = 1;

    *best_cost = HUGE_VAL;

    brute_force_search(
        cities,
        n,
        route,
        best_route,
        visited,
        1,
        0.0,
        best_cost
    );

    free(route);
    free(visited);

    return isfinite(*best_cost);
}

// Exibe uma rota
void print_route(int route[], int n) {
    for (int i = 0; i < n; i++)
        printf("%d -> ", route[i]);

    printf("%d\n", route[0]);
}

// --------------------------------------------------
// MAIN
// --------------------------------------------------

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

    Edge *tour = malloc((size_t)n * sizeof(Edge));
    int *greedy_route = malloc((size_t)n * sizeof(int));

    if (tour == NULL || greedy_route == NULL) {
        printf("Erro ao alocar memoria.\n");

        free(cities);
        free(edges);
        free(tour);
        free(greedy_route);

        return 1;
    }

    double greedy_cost;

    // Executa Greedy e mede o tempo
    clock_t start_greedy = clock();

    int greedy_success = tsp_cheapest_greedy(
        n, edges, edge_count, tour, &greedy_cost
    );

    clock_t end_greedy = clock();

    double greedy_time = (double)(end_greedy - start_greedy)
                         / CLOCKS_PER_SEC;

    if (!greedy_success ||
        !reconstruct_tour(n, tour, greedy_route)) {

        printf("Erro ao construir a rota Greedy.\n");

        free(cities);
        free(edges);
        free(tour);
        free(greedy_route);

        return 1;
    }

    printf("\n========== GREEDY ==========\n");

    printf("Rota: ");
    print_route(greedy_route, n);

    printf("Custo: %.6f\n", greedy_cost);
    printf("Tempo: %.6f segundos\n", greedy_time);

    // --------------------------------------------------
    // Executa a força bruta, se a instância for pequena
    // --------------------------------------------------

    if (n <= MAX_BRUTE_FORCE_CITIES) {

        int *best_route = malloc((size_t)n * sizeof(int));

        if (best_route == NULL) {
            printf("Erro ao alocar memoria para a forca bruta.\n");

            free(cities);
            free(edges);
            free(tour);
            free(greedy_route);

            return 1;
        }

        double best_cost;

        clock_t start_brute = clock();

        int brute_success = tsp_brute_force(
            cities, n, best_route, &best_cost
        );

        clock_t end_brute = clock();

        double brute_time = (double)(end_brute - start_brute)
                            / CLOCKS_PER_SEC;

        if (!brute_success) {
            printf("Erro ao executar a forca bruta.\n");

            free(cities);
            free(edges);
            free(tour);
            free(greedy_route);
            free(best_route);

            return 1;
        }

        printf("\n======= FORCA BRUTA =======\n");

        printf("Rota otima: ");
        print_route(best_route, n);

        printf("Custo otimo: %.6f\n", best_cost);
        printf("Tempo: %.6f segundos\n", brute_time);

        // Comparação dos resultados
        printf("\n======= COMPARACAO =======\n");

        if (best_cost > 0.0) {
            double error = ((greedy_cost - best_cost) / best_cost) * 100.0;

            printf("Erro percentual: %.2f%%\n", error);
        }
        else {
            printf("Erro percentual indefinido: custo otimo igual a zero.\n");
        }

        free(best_route);
    }
    else {
        printf("\nForca bruta nao executada.\n");
        printf("Limite configurado: %d cidades.\n",
               MAX_BRUTE_FORCE_CITIES);
    }

    // Liberação da memória
    free(cities);
    free(edges);
    free(tour);
    free(greedy_route);

    return 0;
}
