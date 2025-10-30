#ifndef DJKTRA
#define DJKTRA

#include "grafos.h"

int obtenerIndiceVertice(GRAFOL*, char);
char obtenerNombrePorIndice(GRAFOL*, int);
void dijkstra(GRAFOL*, char);

int obtenerIndiceVertice(GRAFOL* grafo, char nombre) {
    int idx = 0;
    VRTC* v = grafo->sigVRTC;
    while (v != NULL) {
        if (v->nombre == nombre)
            return idx;
        idx++;
        v = v->sig_V;
    }
    return -1;
}

char obtenerNombrePorIndice(GRAFOL* grafo, int idx) {
    int i = 0;
    VRTC* v = grafo->sigVRTC;
    while (v != NULL) {
        if (i == idx)
            return v->nombre;
        i++;
        v = v->sig_V;
    }
    return '?';
}

void dijkstra(GRAFOL* grafo, char origen) {
    int n, i, u, minDist;
    n = grafo->numVertices;
    int dist[n];           // Distancias mínimas
    bool visitado[n];      // Visitados
    char prev[n];          // Predecesores para reconstruir caminos
    VRTC *vertU;
    ARST *a;
    
    i = 0;
    for (i = 0; i < n; i++) {
        dist[i] = INT_MAX;
        visitado[i] = false;
        prev[i] = 0;
    }

    int idxOrigen = obtenerIndiceVertice(grafo, origen);
    if (idxOrigen == -1) {
        printf("Vertice de origen no existe\n");
        return;
    }
    dist[idxOrigen] = 0;

    for (int count = 0; count < n; count++) {
        //actualizacion de distancias
        minDist = INT_MAX;
        u = -1;
        for (int i = 0; i < n; i++) {
            if (!visitado[i] && dist[i] < minDist) {
                minDist = dist[i];
                u = i;
            }
        }
        if (u == -1)
            break; // Todos visitados o inaccesibles

        visitado[u] = true;
        // Revisar adyacentes de u
        char nombreU = obtenerNombrePorIndice(grafo, u);
        vertU = grafo->sigVRTC;
        while (vertU != NULL && vertU->nombre != nombreU)
            vertU = vertU->sig_V;
        if (vertU != NULL) {
            a = vertU->listaAdj;
            while (a != NULL) {
                int v = obtenerIndiceVertice(grafo, a->dest);
                if (v != -1 && !visitado[v] && dist[u] != INT_MAX && dist[u] + a->peso < dist[v]) { //relajacion
                    dist[v] = dist[u] + a->peso;
                    prev[v] = nombreU;
                }
                a = a->sig_A;
            }
        }
    }

    printf("Distancias mínimas desde %c:\n", origen);
    for (int i = 0; i < n; i++) {
        char destino = obtenerNombrePorIndice(grafo, i);
        printf("%c: ", destino);
        if (dist[i] == INT_MAX) {
            printf("INFINITO\n");
        } else {
            printf("%d", dist[i]);
            printf(" [camino: ");
            char path[128];
            int pidx = 0;
            char curr = destino;
            while (curr != origen && prev[obtenerIndiceVertice(grafo, curr)] != 0) {
                path[pidx++] = curr;
                curr = prev[obtenerIndiceVertice(grafo, curr)];
            }
            path[pidx++] = origen;
            for (int k = pidx - 1; k >= 0; k--)
                printf("%c", path[k]);
            printf("]\n");
        }
    }
    printf("\n");
}

#endif