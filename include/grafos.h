#ifndef GRAFO_L
#define GRAFO_L

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <string.h>

typedef struct ARST{
    char dest;
    int peso;
    struct ARST* sig_A;
} ARST;

typedef struct VRTC {
    char nombre;
    ARST* listaAdj;
    struct VRTC* sig_V;
} VRTC;

typedef struct GRAFOL {
    int numVertices;
    VRTC* sigVRTC;
} GRAFOL;

GRAFOL* initGrafoL();
ARST* crearARSTL(char, int);
VRTC* crearVRTCL(char);
void imprimirGrafoL(GRAFOL*);
void eliminarVRTCL(GRAFOL*, char);
void eliminarARSTL(GRAFOL*, char, char);
void agregarARSTL(GRAFOL*, char, char, int);
void agregarVRTCL(GRAFOL*, char);
void liberarGrafoL(GRAFOL* );
void guardarGrafoListaAdj(const char*, GRAFOL*);

ARST* crearARSTL(char dest, int peso) {
    ARST* nuevaARST = NULL;
    nuevaARST = (ARST*)malloc(sizeof(ARST));
    if(nuevaARST != NULL) {
        nuevaARST->dest = dest;
        nuevaARST->peso = peso;
        nuevaARST->sig_A = NULL;
    }
    return nuevaARST;
}

VRTC* crearVRTCL(char letra) {
    VRTC* nuevoVrtc = NULL;
    nuevoVrtc = (VRTC*)malloc(sizeof(VRTC));

    if(nuevoVrtc != NULL) {
        nuevoVrtc->nombre = letra;
        nuevoVrtc->sig_V = NULL;
        nuevoVrtc->listaAdj = NULL;
    }
    return nuevoVrtc;
}

GRAFOL* initGrafoL() {
    GRAFOL* grafo = NULL;

    grafo = (GRAFOL*)malloc(sizeof(GRAFOL));
    if(grafo != NULL) {
        grafo->numVertices = 0;
        grafo->sigVRTC = NULL;
    }
    return grafo;
}

void liberarGrafoL(GRAFOL* grafo) {
    VRTC* currVRTC = NULL;
    ARST* currARST = NULL;

    if (grafo != NULL) {
        currVRTC = grafo->sigVRTC;
        while (currVRTC != NULL) {
            // Liberar todas las aristas de este vértice
            currARST = currVRTC->listaAdj;
            while (currARST != NULL) {
                ARST* tempARST = currARST;
                currARST = currARST->sig_A;
                free(tempARST);
            }
            // Liberar el vértice
            VRTC* tempVRTC = currVRTC;
            currVRTC = currVRTC->sig_V;
            free(tempVRTC);
        }
        free(grafo);
    }
}

void imprimirGrafoL(GRAFOL* grafo) {
    VRTC* currVRTC = NULL;
    ARST* currARST = NULL;

    if (grafo != NULL){
        currVRTC = grafo->sigVRTC;
        printf("V -> [V | P]\n");
        while (currVRTC != NULL) {
            printf("%c -> ", currVRTC->nombre);
            currARST = currVRTC->listaAdj;
            while (currARST != NULL) {
                printf("[%c | %d]", currARST->dest, currARST->peso);
                currARST = currARST->sig_A;
            }
            printf("\n");
            currVRTC = currVRTC->sig_V;
        }
    }
}

void agregarVRTCL(GRAFOL* grafo, char letra) {
    VRTC *nuevoVRTC = NULL, *currVRTC;
    
    if(grafo != NULL) {
        nuevoVRTC = crearVRTCL(letra);
        grafo->numVertices++;
        
        if(grafo->sigVRTC == NULL) {
            grafo->sigVRTC = nuevoVRTC; 
        }
        else {
            currVRTC = grafo->sigVRTC;
            while(currVRTC->sig_V != NULL) {
                currVRTC = currVRTC->sig_V;
            }
            currVRTC->sig_V = nuevoVRTC;
        }
        printf("Vertice %c agregado!\n", letra);
    }
}

void agregarARSTL(GRAFOL* grafo, char orig, char dest, int peso) {
    VRTC* currVRTC = NULL;
    ARST* nuevaARST = NULL;
    
    if (grafo != NULL) {
        currVRTC = grafo->sigVRTC;
        while (currVRTC != NULL && currVRTC->nombre != orig) {
            currVRTC = currVRTC->sig_V;
        }
        if (currVRTC != NULL) {
            nuevaARST = crearARSTL(dest, peso);
            nuevaARST->sig_A = currVRTC->listaAdj;
            currVRTC->listaAdj = nuevaARST;
            printf("Arista creada de %c ---> %c", orig + 'A', dest + 'A');
        }
        else {
            printf("No se encontro el vertice de origen\n");
        }
    }
}

void eliminarARSTL(GRAFOL* grafo, char src, char dest) {
    ARST* currARST = NULL;
    VRTC* currVRTC = NULL;
    ARST* prevARST = NULL;
    
    if (grafo != NULL) {
        currVRTC = grafo->sigVRTC;
        while (currVRTC != NULL && currVRTC->nombre != src) {
            currVRTC = currVRTC->sig_V;
        }
        if (currVRTC != NULL) {
            currARST = currVRTC->listaAdj;
            prevARST = NULL;
        
            while (currARST != NULL && currARST->dest != dest) {
                prevARST = currARST;
                currARST = currARST->sig_A;
            }
            if (currARST != NULL) {
                if (prevARST == NULL) {
                    currVRTC->listaAdj = currARST->sig_A;
                } else {
                    prevARST->sig_A = currARST->sig_A;
                }
                free(currARST);
                printf("Arista eliminada de %c --> %c\n", src + 'A', dest + 'A');
            }
            else {
                printf("No se encontro la arista\n");
            }
        }
        else {
            printf("No se encontro el vertice de origen\n");
        }
    }
}

void eliminarVRTCL(GRAFOL* grafo, char nombre) {
    VRTC* currVRTC = NULL;
    VRTC* prevVRTC = NULL;
    ARST* currARST = NULL;
    ARST* curr = NULL;
    ARST* temp = NULL;
    ARST* prev = NULL;
    VRTC* v = NULL;

    v = grafo->sigVRTC;
    while (v != NULL) {
        curr = v->listaAdj;
        prev = NULL;
        while (curr != NULL) {
            if (curr->dest == nombre) {
                if (prev == NULL) {
                    v->listaAdj = curr->sig_A;
                } else {
                    prev->sig_A = curr->sig_A;
                }
                temp = curr;
                curr = curr->sig_A;
                free(temp);
            } else {
                prev = curr;
                curr = curr->sig_A;
            }
        }
        v = v->sig_V;
    }

    currVRTC = grafo->sigVRTC;
    
    if(grafo != NULL) {
        while (currVRTC != NULL && currVRTC->nombre != nombre) {
            prevVRTC = currVRTC;
            currVRTC = currVRTC->sig_V;
        }
        if (currVRTC != NULL) {
            currARST = currVRTC->listaAdj;
            while (currARST != NULL) {
                temp = currARST;
                currARST = currARST->sig_A;
                free(temp);
            }
            if (prevVRTC == NULL) {
                grafo->sigVRTC = currVRTC->sig_V;
            } else {
                prevVRTC->sig_V = currVRTC->sig_V;
            }
            free(currVRTC);
            grafo->numVertices--;
            printf("Vertice %c eliminado!\n", nombre);
        }
        else {
            printf("No se encontro el vertice %c\n", nombre);
        }
    }
}

void guardarGrafoListaAdj(const char* dir, GRAFOL* grafo) {
    FILE* archivo = fopen(dir, "w");
    if (!archivo || !grafo) {
        printf("No se pudo abrir el archivo o el grafo es NULL\n");
        return;
    }

    fprintf(archivo, "VERTICES\n");
    VRTC* v = grafo->sigVRTC;
    while (v != NULL) {
        fprintf(archivo, "%c\n", v->nombre);
        v = v->sig_V;
    }

    fprintf(archivo, "ARISTAS\n");
    v = grafo->sigVRTC;
    while (v != NULL) {
        ARST* a = v->listaAdj;
        while (a != NULL) {
            fprintf(archivo, "%c %c %d\n", v->nombre, a->dest, a->peso);
            a = a->sig_A;
        }
        v = v->sig_V;
    }

    fclose(archivo);
}

#endif