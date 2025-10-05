#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>

#define MAX_NODES 201
#define MAX_ENERGY 1001
#define INF LLONG_MAX

char node_names[MAX_NODES][10];
int node_count = 0;

typedef struct Edge {
    int to;
    int w;
    int o;
    struct Edge* next;
} Edge;

Edge* adj[MAX_NODES];
int is_rest_point[MAX_NODES] = {0};
int is_charging_station[MAX_NODES] = {0};

typedef struct {
    long long cost;
    int u;
    int energy;
    int parity; 
} State;

typedef struct {
    int u;
    int energy;
    int parity;
} Parent;

long long dist[MAX_NODES][MAX_ENERGY][2];
Parent parent_tracker[MAX_NODES][MAX_ENERGY][2];

State pq[MAX_NODES * MAX_ENERGY * 2];
int pq_size = 0;

void swap(State* a, State* b) {
    State temp = *a; *a = *b; *b = temp;
}

void heapify_up(int i) {
    while (i > 0 && pq[i].cost < pq[(i - 1) / 2].cost) {
        swap(&pq[i], &pq[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

void heapify_down(int i) {
    int min_idx = i;
    int l = 2 * i + 1, r = 2 * i + 2;
    if (l < pq_size && pq[l].cost < pq[min_idx].cost) min_idx = l;
    if (r < pq_size && pq[r].cost < pq[min_idx].cost) min_idx = r;
    if (i != min_idx) {
        swap(&pq[i], &pq[min_idx]);
        heapify_down(min_idx);
    }
}

void pq_push(State s) {
    pq[pq_size++] = s;
    heapify_up(pq_size - 1);
}

State pq_pop() {
    State top = pq[0];
    pq[0] = pq[pq_size - 1];
    pq_size--;
    heapify_down(0);
    return top;
}

int get_node_idx(char* name) {
    for (int i = 0; i < node_count; ++i) {
        if (strcmp(node_names[i], name) == 0) return i;
    }
    strcpy(node_names[node_count], name);
    return node_count++;
}

void add_edge(int u, int v, int w, int o) {
    Edge* edge_uv = (Edge*)malloc(sizeof(Edge));
    edge_uv->to = v; edge_uv->w = w; edge_uv->o = o;
    edge_uv->next = adj[u]; adj[u] = edge_uv;
    
    Edge* edge_vu = (Edge*)malloc(sizeof(Edge));
    edge_vu->to = u; edge_vu->w = w; edge_vu->o = o;
    edge_vu->next = adj[v]; adj[v] = edge_vu;
}

int main() {
    int N, M;
    scanf("%d %d", &N, &M);
    
    for (int i = 0; i < M; ++i) {
        char u_str[10], v_str[10]; 
        int w, o;
        scanf("%s %s %d %d", u_str, v_str, &w, &o);
        add_edge(get_node_idx(u_str), get_node_idx(v_str), w, o);
    }
    

    char s_str[10], t_str[10];
    scanf("%s %s", s_str, t_str);
    int start_node = get_node_idx(s_str);
    int target_node = get_node_idx(t_str);
    

    char rest_str[256];
    scanf(" %[^\n]", rest_str);
    if (strcmp(rest_str, "-") != 0) {
        char* token = strtok(rest_str, " ");
        while (token) { 
            is_rest_point[get_node_idx(token)] = 1; 
            token = strtok(NULL, " "); 
        }
    }
    

    char charge_str[256];
    scanf(" %[^\n]", charge_str);
    if (strcmp(charge_str, "-") != 0) {
        char* token = strtok(charge_str, " ");
        while (token) { 
            is_charging_station[get_node_idx(token)] = 1; 
            token = strtok(NULL, " "); 
        }
    }
    
    char dummy[10]; 
    scanf("%s", dummy);
    scanf("%s", dummy);
    
    int start_hour; 
    scanf("%d", &start_hour);

    for (int i = 0; i < MAX_NODES; ++i) {
        for (int j = 0; j < MAX_ENERGY; ++j) {
            for (int k = 0; k < 2; ++k) {
                dist[i][j][k] = INF;
            }
        }
    }
    

    int initial_parity = start_hour % 2; 
    dist[start_node][1000][initial_parity] = 0;
    parent_tracker[start_node][1000][initial_parity] = (Parent){-1, -1, -1};
    pq_push((State){0, start_node, 1000, initial_parity});


    while (pq_size > 0) {
        State current = pq_pop();
        long long cost = current.cost; 
        int u = current.u; 
        int energy = current.energy; 
        int parity = current.parity;
        
        if (cost > dist[u][energy][parity]) continue;


        Edge* edge = adj[u];
        while(edge != NULL) {
            int v = edge->to;
            double multiplier = (parity == 1) ? 1.3 : 0.8;
            long long actual_energy_cost = (long long)round((edge->w + edge->o) * multiplier);
            
            if (energy >= actual_energy_cost) {
                int new_energy = energy - actual_energy_cost;
                long long new_cost = cost + actual_energy_cost;
                int new_parity = 1 - parity; 
                
                if (new_cost < dist[v][new_energy][new_parity]) {
                    dist[v][new_energy][new_parity] = new_cost;
                    parent_tracker[v][new_energy][new_parity] = (Parent){u, energy, parity};
                    pq_push((State){new_cost, v, new_energy, new_parity});
                }
            }
            edge = edge->next;
        }

        if (is_rest_point[u]) {
            int new_parity = 1 - parity;
            if (cost < dist[u][energy][new_parity]) {
                dist[u][energy][new_parity] = cost;
                parent_tracker[u][energy][new_parity] = (Parent){u, energy, parity}; 
                pq_push((State){cost, u, energy, new_parity});
            }
        }
        
        if (is_charging_station[u] && energy < 1000) {
            if (cost < dist[u][1000][parity]) {
                dist[u][1000][parity] = cost;
                parent_tracker[u][1000][parity] = (Parent){u, energy, parity};
                pq_push((State){cost, u, 1000, parity});
            }
        }
    }

    long long min_total_energy = INF;
    int final_energy = -1, final_parity = -1;
    for (int e = 0; e < MAX_ENERGY; ++e) {
        for (int p = 0; p < 2; ++p) {
            if (dist[target_node][e][p] < min_total_energy) {
                min_total_energy = dist[target_node][e][p];
                final_energy = e; 
                final_parity = p;
            }
        }
    }

    if (min_total_energy == INF) {
        printf("Robot gagal dalam mencapai tujuan :(\n");
    } else {
        printf("Total energi minimum: %lld\n", min_total_energy);
        
        Parent path[MAX_NODES * 2]; 
        int path_len = 0;
        int curr_u = target_node, curr_e = final_energy, curr_p = final_parity;
        
        while(curr_u != -1) {
            path[path_len++] = (Parent){curr_u, curr_e, curr_p}; 
            Parent p = parent_tracker[curr_u][curr_e][curr_p];
            curr_u = p.u; 
            curr_e = p.energy; 
            curr_p = p.parity;
        }
        
        printf("Jalur: ");
        int printed = 0;
        for (int i = path_len - 1; i >= 0; --i) {
            if (i == path_len - 1 || path[i].u != path[i + 1].u) {
                if (printed > 0) printf(" -> ");
                printf("%s", node_names[path[i].u]);
                printed++;
            }
        }
        printf("\n");
        
        printf("Waktu tiba:\n");
        int step_count = 0;
        for (int i = path_len - 1; i >= 0; --i) {
            if (i == path_len - 1 || path[i].u != path[i + 1].u) { 
                printf("%s (menit %d)\n", node_names[path[i].u], step_count * 2);
                if (i > 0) step_count++;
            }
        }
    }
    
    return 0;
}




