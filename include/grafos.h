#ifndef GRAFOS_H
#define GRAFOS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#define MAX_NOMBRE 64
#define MAX_IP 16

typedef enum
{
    D_ROUTER,
    D_SWITCH,
    D_HOST,
    D_SERVIDOR,
    D_DEFAULT
} Tipo_Dispositivo;

/* Arista (lista de adyacencia) */
typedef struct ARISTA
{
    int destino;              /* índice del vértice destino */
    int latencia_ms;          /* latencia en ms */
    int ancho_banda_mbps;     /* ancho de banda en Mbps */
    double fiabilidad;        /* 0.0 .. 1.0 */
    int activo;               /* 1 = activo, 0 = caído */
    struct ARISTA *siguiente; /* siguiente arista */
} ARISTA;

/* Vértice */
typedef struct VERTICE
{
    char nombre[MAX_NOMBRE]; /* "Router 1", "Switch 2", etc. */
    char ip[MAX_IP];         /* "192.168.1.154" */
    Tipo_Dispositivo tipo;
    int capacidad_procesamiento; /* valor arbitrario que simula capacidad */
    int activo;                  /* 1 = activo, 0 = fallido */
    ARISTA *lista_adyacencia;    /* lista de adyacencia */
} VERTICE;

/* Grafo por lista de adyacencia */
typedef struct GRAFO
{
    int num_vertices;
    VERTICE *vertices; /* array dinámico de vértices */
    int capacidad;     /* tamaño actual del array */
} GRAFO;

/* Creación / liberación */
GRAFO *crear_grafo(int capacidad_inicial);
void liberar_grafo(GRAFO *grafo);

/* Vértices */
int agregar_vertice(GRAFO *grafo, const char *nombre, const char *ip, Tipo_Dispositivo tipo, int capacidad_proc);
int indice_por_nombre(GRAFO *grafo, const char *nombre);
int establecer_estado_vertice(GRAFO *grafo, int indice, int activo);

/* Aristas */
int agregar_arista(GRAFO *grafo, int indice_origen, int indice_destino, int latencia_ms, int ancho_banda_mbps, double fiabilidad, int activo);
int establecer_estado_arista(GRAFO *grafo, int indice_origen, int indice_destino, int activo);

/* I/O */
void imprimir_grafo(GRAFO *grafo);
int guardar_grafo(GRAFO *grafo, const char *filename);
int cargar_grafo(GRAFO *grafo, const char *filename);

/* Funciones ayudantes */
int asegurar_capacidad(GRAFO *grafo);
int ip_valida(const char *ip);

const char *tipo_dispositivo_a_cadena(Tipo_Dispositivo);

// Implementaciones de funciones

int asegurar_capacidad(GRAFO *grafo)
{
    VERTICE *tmp;
    int nueva;

    if (!grafo)
    {
        return -1;
    }
    if (grafo->num_vertices < grafo->capacidad)
    {
        return 0;
    }
    nueva = (grafo->capacidad == 0) ? 8 : grafo->capacidad * 2;
    tmp = realloc(grafo->vertices, sizeof(VERTICE) * nueva);

    if (!tmp)
        return -1;

    grafo->vertices = tmp;
    grafo->capacidad = nueva;
    return 0;
}

GRAFO *crear_grafo(int capacidad_inicial)
{
    GRAFO *grafo;

    grafo = malloc(sizeof(GRAFO));

    if (!grafo)
        return NULL;

    grafo->num_vertices = 0;
    grafo->capacidad = (capacidad_inicial > 0) ? capacidad_inicial : 8;
    grafo->vertices = calloc(grafo->capacidad, sizeof(VERTICE));

    if (!grafo->vertices)
    {
        free(grafo);
        return NULL;
    }
    return grafo;
}

void liberar_grafo(GRAFO *grafo)
{
    int i;
    ARISTA *ar, *sig;

    if (!grafo)
        return;

    i = 0;
    for (i = 0; i < grafo->num_vertices; ++i)
    {
        ar = grafo->vertices[i].lista_adyacencia;
        while (ar)
        {
            sig = ar->siguiente;
            free(ar);
            ar = sig;
        }
    }
    free(grafo->vertices);
    free(grafo);
}

/* Comprueba formato IPv4 simple: a.b.c.d  (0-255) */
int ip_valida(const char *ip)
{
    int a, b, c, d;
    char extra[8];

    if (!ip)
        return 0;
    if (sscanf(ip, "%d.%d.%d.%d%7s", &a, &b, &c, &d, extra) < 4)
        return 0;
    if (a < 0 || a > 255 || b < 0 || b > 255 || c < 0 || c > 255 || d < 0 || d > 255)
        return 0;
    return 1;
}

/* Add vertex */
int agregar_vertice(GRAFO *grafo, const char *nombre, const char *ip, Tipo_Dispositivo tipo, int capacidad_proc)
{
    int indice;
    VERTICE *v;

    if (!grafo || !nombre || !ip)
        return -1;
    if (!ip_valida(ip))
    {
        fprintf(stderr, "[!!!] IP inválida: %s\n", ip);
        return -1;
    }
    if (indice_por_nombre(grafo, nombre) != -1)
    {
        fprintf(stderr, "[!!!] Nodo duplicado: %s\n", nombre);
        return -1;
    }
    if (asegurar_capacidad(grafo) != 0)
        return -1;

    indice = grafo->num_vertices++;
    v = &grafo->vertices[indice];
    strncpy(v->nombre, nombre, MAX_NOMBRE - 1);
    v->nombre[MAX_NOMBRE - 1] = '\0';
    strncpy(v->ip, ip, MAX_IP - 1);
    v->ip[MAX_IP - 1] = '\0';
    v->tipo = tipo;
    v->capacidad_procesamiento = capacidad_proc;
    v->activo = 1;
    v->lista_adyacencia = NULL;
    return indice;
}

int indice_por_nombre(GRAFO *grafo, const char *nombre)
{
    int i;

    if (!grafo || !nombre)
        return -1;

    i = 0;
    for (i = 0; i < grafo->num_vertices; ++i)
    {
        if (strncmp(grafo->vertices[i].nombre, nombre, MAX_NOMBRE) == 0)
            return i;
    }
    return -1;
}

int establecer_estado_vertice(GRAFO *grafo, int indice, int activo)
{
    if (!grafo)
        return -1;
    if (indice < 0 || indice >= grafo->num_vertices)
        return -1;
    grafo->vertices[indice].activo = activo ? 1 : 0;
    return 0;
}

/* Add directed edge origen -> dest */
int agregar_arista(GRAFO *grafo, int indice_origen, int indice_destino, int latencia_ms, int ancho_banda_mbps, double fiabilidad, int activo)
{
    ARISTA *ar;
    if (!grafo)
        return -1;
    if (indice_origen < 0 || indice_origen >= grafo->num_vertices)
        return -1;
    if (indice_destino < 0 || indice_destino >= grafo->num_vertices)
        return -1;
    if (latencia_ms < 0 || ancho_banda_mbps < 0)
    {
        fprintf(stderr, "[!!!] Latencia/Ancho invalidos.\n");
        return -1;
    }
    if (fiabilidad < 0.0)
        fiabilidad = 0.0;
    if (fiabilidad > 1.0)
        fiabilidad = 1.0;

    ar = malloc(sizeof(ARISTA));
    if (!ar)
        return -1;
    ar->destino = indice_destino;
    ar->latencia_ms = latencia_ms;
    ar->ancho_banda_mbps = ancho_banda_mbps;
    ar->fiabilidad = fiabilidad;
    ar->activo = activo ? 1 : 0;
    ar->siguiente = grafo->vertices[indice_origen].lista_adyacencia;
    grafo->vertices[indice_origen].lista_adyacencia = ar;
    return 0;
}

int establecer_estado_arista(GRAFO *grafo, int indice_origen, int indice_destino, int activo)
{
    ARISTA *ar;
    if (!grafo)
        return -1;
    if (indice_origen < 0 || indice_origen >= grafo->num_vertices)
        return -1;

    ar = grafo->vertices[indice_origen].lista_adyacencia;

    while (ar)
    {
        if (ar->destino == indice_destino)
        {
            ar->activo = activo ? 1 : 0;
            return 0;
        }
        ar = ar->siguiente;
    }
    return -1;
}

const char *tipo_dispositivo_a_cadena(Tipo_Dispositivo t)
{
    switch (t)
    {
    case D_ROUTER:
        return "ROUTER";
    case D_SWITCH:
        return "SWITCH";
    case D_HOST:
        return "HOST";
    case D_SERVIDOR:
        return "SERVIDOR";
    default:
        return "NA";
    }
}

/* IMPRIME GRAFO */
void imprimir_grafo(GRAFO *grafo)
{
    int i;
    VERTICE *v;
    ARISTA *ar;

    if (!grafo)
    {
        printf("Grafo NULL\n");
        return;
    }
    printf("Grafo: %d vertices\n", grafo->num_vertices);

    i = 0;
    for (i = 0; i < grafo->num_vertices; ++i)
    {
        v = &grafo->vertices[i];
        printf(" [%d] %s (%s) - Tipo: %s, Cap: %d, Estado: %s\n", i, v->nombre, v->ip, tipo_dispositivo_a_cadena(v->tipo), v->capacidad_procesamiento, v->activo ? "ACTIVO" : "FALLIDO");
        ar = v->lista_adyacencia;
        while (ar)
        {
            printf("    -> %s (idx %d) | lat=%dms bw=%dMbps conf=%.2f estado=%s\n", ((ar->destino >= 0 && ar->destino < grafo->num_vertices) ? grafo->vertices[ar->destino].nombre : "??"), ar->destino, ar->latencia_ms, ar->ancho_banda_mbps, ar->fiabilidad, ar->activo ? "ACTIVO" : "FALLADO");
            ar = ar->siguiente;
        }
    }
}

/* Guardar grafo */
int guardar_grafo(GRAFO *grafo, const char *filename)
{
    FILE *f;
    int i, contador_aristas;
    VERTICE *v;
    ARISTA *ar;

    if (!grafo || !filename)
        return -1;

    f = fopen(filename, "w");

    if (!f)
        return -1;

    fprintf(f, "NS %d\n", grafo->num_vertices);

    i = 0;
    for (i = 0; i < grafo->num_vertices; ++i)
    {
        v = &grafo->vertices[i];
        /* NODE <nombre> <ip> <tipo> <cap> */
        fprintf(f, "N %s %s %d %d\n", v->nombre, v->ip, (int)v->tipo, v->capacidad_procesamiento);
    }
    /* contar aristas */
    contador_aristas = 0;

    i = 0;
    for (i = 0; i < grafo->num_vertices; ++i)
    {
        ar = grafo->vertices[i].lista_adyacencia;
        while (ar)
        {
            ++contador_aristas;
            ar = ar->siguiente;
        }
    }

    fprintf(f, "AS %d\n", contador_aristas);

    i = 0;
    for (i = 0; i < grafo->num_vertices; ++i)
    {
        ar = grafo->vertices[i].lista_adyacencia;
        while (ar)
        {
            /* EDGE <origenName> <destName> <lat> <bw> <fiab> <activo> */
            fprintf(f, "A %s %s %d %d %.6f %d\n",
                    grafo->vertices[i].nombre,
                    grafo->vertices[ar->destino].nombre,
                    ar->latencia_ms,
                    ar->ancho_banda_mbps,
                    ar->fiabilidad,
                    ar->activo);
            ar = ar->siguiente;
        }
    }
    fclose(f);
    return 0;
}

/* Carga grafo de un archivo de texto (formato producido por guardar_grafo) */
int cargar_grafo(GRAFO *grafo, const char *filename)
{
    FILE *f;
    char linea[512], nombre[MAX_NOMBRE], ip[MAX_IP], tag[32], orig[MAX_NOMBRE], dest[MAX_NOMBRE];
    int nodos_esperados, aristas_esperadas, tipo_int, cap, lat, bw, activo, oi, di;
    Tipo_Dispositivo dt;
    double fi;

    if (!grafo || !filename)
        return -1;

    f = fopen(filename, "r");

    if (!f)
        return -1;

    nodos_esperados = -1;
    aristas_esperadas = -1;

    /* We'll store node names to map them to indices as they're added */
    while (fgets(linea, sizeof(linea), f))
    {
        if (linea[0] == '#' || isspace((unsigned char)linea[0]))
            continue;

        if (sscanf(linea, "%31s", tag) != 1)
            continue;

        if (strcmp(tag, "NS") == 0)
        {
            sscanf(linea + strlen("NS"), "%d", &nodos_esperados);
        }
        else if (strcmp(tag, "N") == 0)
        {
            tipo_int = D_DEFAULT;
            cap = 0;
            /* NODE <nombre> <ip> <tipo> <cap> */
            if (sscanf(linea, "N %63s %15s %d %d", nombre, ip, &tipo_int, &cap) >= 3)
            {
                dt = (Tipo_Dispositivo)tipo_int;
                agregar_vertice(grafo, nombre, ip, dt, cap);
            }
        }
        else if (strcmp(tag, "AS") == 0)
        {
            sscanf(linea + strlen("AS"), "%d", &aristas_esperadas);
        }
        else if (strcmp(tag, "A") == 0)
        {
            if (sscanf(linea, "A %63s %63s %d %d %lf %d", orig, dest, &lat, &bw, &fi, &activo) >= 6)
            {
                oi = indice_por_nombre(grafo, orig);
                di = indice_por_nombre(grafo, dest);
                if (oi == -1 || di == -1)
                {
                    /* ignorar o reportar en cli */
                    fprintf(stderr, "[!!!] A hace referencia a nodo invalido: %s -> %s\n", orig, dest);
                }
                else
                {
                    agregar_arista(grafo, oi, di, lat, bw, fi, activo);
                }
            }
        }
        else
        {
            /* ignora todo lo demas */
        }
    }

    fclose(f);
    return 0;
}

#endif