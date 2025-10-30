#include "grafos.h"
#include "dijkstra.h"
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

#ifdef __WIN32
	#define LIMPIAR_PANTALLA system("cls")
#else
	#define LIMPIAR_PANTALLA system("clear")
#endif

void imprimirMenu();


void obtener_o_d_lista(char *);
void menu_grafo_lista(GRAFOL *);
void obtener_peso_lista(int *);

int letraIndice(char);

int main() {
	GRAFOL *G_L = NULL;
	pid_t pidPython;

	LIMPIAR_PANTALLA;
	G_L = initGrafoL();
	guardarGrafoListaAdj("txt/matrixGrafo.txt", G_L);
	pidPython = fork();
	if (pidPython == 0) {
		freopen("/dev/null", "w", stdout);
		freopen("/dev/null", "w", stderr);
		execlp("python3", "python3", "src/verGrafoL.py", (char*)NULL);
	}
	else if (pidPython > 0) {
		LIMPIAR_PANTALLA;
		menu_grafo_lista(G_L);
		kill(pidPython, SIGTERM);
	}
	else {
		printf("No se pudo abrir el simulador\n");
	}
	return 0;
}

void imprimirMenu() {
	printf("1 - Agregar Vertice\n");
	printf("2 - Eliminar Vertice\n");
	printf("3 - Conectar Vertices\n");
	printf("4 - Desconectar Vertices\n");
	printf("5 - Imprimir Grafo\n");
	printf("6 - Salir\n");
	printf("Opcion: ");
}


void obtener_o_d_lista(char *verticesEscogidos) {
	char letra;

    printf("Ingrese el vertice de origen: ");
    scanf(" %c", &letra);
    verticesEscogidos[0] = letra;
    
    printf("Ingrese el vertice de Destino: ");
    scanf(" %c", &letra);
    verticesEscogidos[1] = letra;
}

int letraIndice(char c) {
	int i;

	i = toupper(c);

	if (i >= 'A' && i <= 'Z') {
		return c - 'A';
	}
	else {
		return -1;
	}
}


void menu_grafo_lista(GRAFOL *G) {
	char op;
    char o_d[3];
    int peso;
	bool cont = true;

	do {
		guardarGrafoListaAdj("txt/matrixGrafoL.txt", G);
		printf("D - Algoritmo de Dijkstra\n");
		imprimirMenu();
		scanf(" %c", &op);
		switch(op)
		{
			case 'D':
				if (G->numVertices != 0) {
					LIMPIAR_PANTALLA;
					printf("Ingrese el vertice de origen: ");
					scanf(" %c", &op);
					dijkstra(G, op);
				}
				else {
					printf("No hay vertices\n");
				}
				break;
			case '1':
				LIMPIAR_PANTALLA;
                printf("Ingrese la etiqueta del nuevo vertice: ");
                scanf(" %c", &op);
				LIMPIAR_PANTALLA;
				agregarVRTCL(G, op);
				break;
			case '2':
				if (G->numVertices != 0) {
					LIMPIAR_PANTALLA;
					printf("Ingrese el vertice a eliminar\n");
					scanf(" %c", &op);
					LIMPIAR_PANTALLA;
					eliminarVRTCL(G, op);
				}
				else {
					printf("No hay vertices\n");
				}
				break;
			case '3':
				if (G->numVertices != 0) {
					obtener_o_d_lista(o_d);
                    obtener_peso_lista(&peso);
					LIMPIAR_PANTALLA;
					agregarARSTL(G, *(o_d + 0), *(o_d + 1), peso);
				}
				else {
					printf("No hay vertices\n");
				}
				break;
			case '4':
				if (G->numVertices != 0) {
					obtener_o_d_lista(o_d);
					LIMPIAR_PANTALLA;
					eliminarARSTL(G, *(o_d + 0), *(o_d + 1));
				}
				else {
					printf("No hay vertices\n");
				}
				break;
			case '5':
				if (G->numVertices != 0) {
					LIMPIAR_PANTALLA;
					imprimirGrafoL(G);
					printf("\n");
				}
				else {
					printf("No hay vertices\n");
				}
				break;
			case '6':
				cont = false;
				break;
			
			default:
				LIMPIAR_PANTALLA;
				printf("Opcion no valida\n");
				break;
		}

	} while (cont && G != NULL);


	printf("Saliendo del programa\n");
	liberarGrafoL(G);
}

void obtener_peso_lista(int *pesoV) {
    int peso;
    printf("Ingrese peso de la arista: ");
    scanf("%d", &peso);
    *pesoV = peso;
}
