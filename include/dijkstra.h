#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "grafos.h"
#include <float.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Metricas utilizadas en Dijkstra*/
typedef enum
{
    METRICA_LATENCIA,
    METRICA_NEG_ANCHO_BANDA,
    METRICA_NEG_FIABILIDAD
} Metrica;

/* Funcion que evalua costo de arista */
typedef double (*FuncionCostoArista)(const ARISTA *arista);
/* Dijkstra básico*/
int dijkstra_camino_minimo(GRAFO *, int, int, FuncionCostoArista, int *, double *);
/* Reconstruye un camino desde anterior[]: devuelve longitud y rellena camino[] */
int reconstruir_camino(const int *, int, int *, int);
/* Calcula metricas agregadas sobre una ruta dada */
int calcular_metricas_ruta(GRAFO *, const int *, int, double *, int *, double *);
/* Encuentra hasta K rutas distintas (aprox) */
int encontrar_k_rutas_aproximadas(GRAFO *, int, int, FuncionCostoArista, int, int[][256], int[], int);

int caminos_iguales(const int *a, int alen, const int *b, int blen);

/* Dijkstra*/
int dijkstra_camino_minimo(GRAFO *grafo, int indice_origen, int indice_destino, FuncionCostoArista funcion_coste, int *anterior, double *distancia)
{
    int n, i, u;
    double mejor, c;
    ARISTA *ar;

    if (!grafo || !funcion_coste || !anterior || !distancia)
        return -1;

    n = grafo->num_vertices;
    bool visitado[n];
    if (indice_origen < 0 || indice_origen >= n)
        return -1;

    if (indice_destino < 0 || indice_destino >= n)
        return -1;

    i = 0;
    for (i = 0; i < n; ++i)
    {
        distancia[i] = DBL_MAX;
        anterior[i] = -1;
    }

    i = 0;
    for (i = 0; i < n; ++i)
        visitado[i] = false;

    distancia[indice_origen] = 0.0;

    for (;;)
    {
        u = -1;
        mejor = DBL_MAX;

        for (i = 0; i < n; ++i)
        {
            if (!visitado[i] && distancia[i] < mejor)
            {
                mejor = distancia[i];
                u = i;
            }
        }
        if (u == -1)
            break;
        if (u == indice_destino)
            break;

        visitado[u] = true;
        if (grafo->vertices[u].activo == 0)
            continue;

        ar = grafo->vertices[u].lista_adyacencia;
        while (ar)
        {
            if (ar->activo)
            {
                c = funcion_coste(ar);
                if (c < 0)
                {
                    ar = ar->siguiente;
                    continue;
                }
                if (distancia[u] + c < distancia[ar->destino])
                {
                    distancia[ar->destino] = distancia[u] + c;
                    anterior[ar->destino] = u;
                }
            }
            ar = ar->siguiente;
        }
    }
    return 0;
}

/* Reconstruir camino */
int reconstruir_camino(const int *anterior, int indice_destino, int *camino, int longitud_maxima)
{
    int tmp[256], contador, actual, i;

    if (!anterior || !camino || longitud_maxima <= 0)
        return -1;
    contador = 0;
    actual = indice_destino;

    while (actual != -1 && contador < 256)
    {
        tmp[contador++] = actual;
        actual = anterior[actual];
    }
    if (contador == 0)
        return -1;
    if (contador > longitud_maxima)
        return -1;

    i = 0;
    for (i = 0; i < contador; ++i)
        camino[i] = tmp[contador - 1 - i];
    return contador;
}

/* Calcular métricas de ruta */
int calcular_metricas_ruta(GRAFO *grafo, const int *camino, int longitud_camino, double *latencia_out, int *bw_min_out, double *fiab_out)
{

    double lat;
    int bwmin, i, u, v, encontrado;
    double prod;
    ARISTA *ar;

    if (!grafo || !camino || longitud_camino <= 0)
        return -1;
    lat = 0.0;
    bwmin = INT_MAX;
    prod = 1.0;

    i = 0;
    for (i = 0; i < longitud_camino - 1; ++i)
    {
        u = camino[i];
        v = camino[i + 1];
        ar = grafo->vertices[u].lista_adyacencia;
        encontrado = 0;

        while (ar)
        {
            if (ar->destino == v && ar->activo)
            {
                lat += (double)ar->latencia_ms;
                if (ar->ancho_banda_mbps < bwmin)
                    bwmin = ar->ancho_banda_mbps;
                prod *= ar->fiabilidad;
                encontrado = 1;
                break;
            }
            ar = ar->siguiente;
        }
        if (!encontrado)
            return -1;
    }
    if (bwmin == INT_MAX)
        bwmin = 0;
    if (latencia_out)
        *latencia_out = lat;
    if (bw_min_out)
        *bw_min_out = bwmin;
    if (fiab_out)
        *fiab_out = prod;
    return 0;
}

/* Compara caminos */
int caminos_iguales(const int *a, int alen, const int *b, int blen)
{
    int i;
    if (alen != blen)
        return 0;

    i = 0;
    for (i = 0; i < alen; ++i)
        if (a[i] != b[i])
            return 0;
    return 1;
}

/* Aproximación simple a K-shortest basada en desactivar aristas del mejor camino */
int encontrar_k_rutas_aproximadas(GRAFO *grafo, int indice_origen, int indice_destino, FuncionCostoArista funcion_coste, int K, int caminos[][256], int longitudes[], int longitud_maxima)
{
    int len, n, anterior[256], encontrados, iter, mejor_camino_len, base_count, p, *camino_base, base_len, e, u, v, i, unico, q, l;
    double distancia[256], mejor_coste;
    int mejor_camino_buf[256];
    ARISTA *ar, *ar_encontrada;
    int tmp[256];

    if (!grafo || !funcion_coste || K <= 0 || longitud_maxima <= 2)
        return 0;
    n = grafo->num_vertices;

    encontrados = 0;
    dijkstra_camino_minimo(grafo, indice_origen, indice_destino, funcion_coste, anterior, distancia);

    if (distancia[indice_destino] >= DBL_MAX / 2)
        return 0;
    len = reconstruir_camino(anterior, indice_destino, caminos[encontrados], longitud_maxima);

    if (len <= 0)
        return 0;
    longitudes[encontrados] = len;
    encontrados++;

    iter = 0;
    for (iter = 0; iter < K * 4 && encontrados < K; ++iter)
    {
        mejor_coste = DBL_MAX;
        mejor_camino_len = 0;
        base_count = encontrados;

        p = 0;
        for (p = 0; p < base_count; ++p)
        {
            camino_base = caminos[p];
            base_len = longitudes[p];
            e = 0;
            for (e = 0; e < base_len - 1; ++e)
            {
                u = camino_base[e];
                v = camino_base[e + 1];
                ar_encontrada = NULL;
                ar = grafo->vertices[u].lista_adyacencia;
                while (ar)
                {
                    if (ar->destino == v && ar->activo)
                    {
                        ar_encontrada = ar;
                        break;
                    }
                    ar = ar->siguiente;
                }
                if (!ar_encontrada)
                    continue;
                /* desactivar */
                ar_encontrada->activo = 0;

                i = 0;
                for (i = 0; i < n; ++i)
                {
                    anterior[i] = -1;
                    distancia[i] = DBL_MAX;
                }
                dijkstra_camino_minimo(grafo, indice_origen, indice_destino, funcion_coste, anterior, distancia);
                if (distancia[indice_destino] < mejor_coste)
                {

                    l = reconstruir_camino(anterior, indice_destino, tmp, longitud_maxima);
                    if (l > 0)
                    {
                        unico = 1;
                        q = 0;
                        for (q = 0; q < encontrados; ++q)
                            if (caminos_iguales(tmp, l, caminos[q], longitudes[q]))
                            {
                                unico = 0;
                                break;
                            }
                        if (unico)
                        {
                            memcpy(mejor_camino_buf, tmp, sizeof(int) * l);
                            mejor_camino_len = l;
                            mejor_coste = distancia[indice_destino];
                        }
                    }
                }
                /* restaurar */
                ar_encontrada->activo = 1;
            }
        }
        if (mejor_coste < DBL_MAX)
        {
            memcpy(caminos[encontrados], mejor_camino_buf, sizeof(int) * mejor_camino_len);
            longitudes[encontrados] = mejor_camino_len;
            encontrados++;
        }
        else
            break;
    }
    return encontrados;
}

#endif /* DIJKSTRA_H */