#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <float.h>
#include <math.h>
#include "grafos.h"
#include "dijkstra.h"
#include "colors.h"

#ifdef _WIN32
#include <windows.h>
#define LIMPIAR system("cls")
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#define LIMPIAR system("clear")
#endif

/* Declaraciones de funciones auxiliares */
double costo_por_latencia(const ARISTA *);
void imprimir_ayuda();
void imprimir_camino_por_indices(GRAFO *, const int *, int);
void resolver_ping(GRAFO *, const char *, const char *, int);
void comando_traceroute(GRAFO *, const char *, const char *, int);
int contar_alcanzables(GRAFO *, int);
void comando_analizar_resiliencia(GRAFO *);
void comando_optimizar_ruta(GRAFO *, const char *, const char *);

int main(void)
{
    srand((unsigned)time(NULL));
    GRAFO *grafo;
    bool ejecutar_cli;
    char linea[512], *token, *archivo_nombre, *nombre, *ip_cadena, *tipo_str, *cap_str, *origen_str, *destino_str, *lat_str, *bw_str, *fi_str, *ct_str, *ks_str;
    int indice, indice_origen, indice_destino, contador, k;
    Tipo_Dispositivo tipo_disp;
    pid_t pidPython = -1, pid;

    const char *archivo_default = "txt/topologia.txt";

    LIMPIAR;

    grafo = crear_grafo(20);

    if (!grafo)
    {
        printf("[ERROR] Error creando grafo.\n");
        return 1;
    }

    if (cargar_grafo(grafo, archivo_default) == 0)
    {
        printf("[OK] Grafo cargado desde %s\n", archivo_default);
    }
    else
    {
        printf("[INFO] No se pudo cargar %s, iniciando grafo vacío. (0/20 vertices)\n", archivo_default);
    }

    ejecutar_cli = true;
    signal(SIGCHLD, SIG_IGN);

    while (ejecutar_cli)
    {

        printf(COLOR_CIANO "net> " COLOR_NORMAL);

        if (!fgets(linea, sizeof(linea), stdin))
        {
            break;
        }

        token = strtok(linea, " \n");

        if (!token)
        {
            continue;
        }

        if (strcmp(token, "salir") == 0)
        {
            ejecutar_cli = false;
            break;
        }

        if (strcmp(token, "ayuda") == 0)
        {
            imprimir_ayuda();
            continue;
        }

        if (strcmp(token, "ver-grafo") == 0)
        {
            imprimir_grafo(grafo);
            continue;
        }

        if (strcmp(token, "guardar") == 0)
        {
            archivo_nombre = strtok(NULL, " \n");
            if (!archivo_nombre)
            {
                printf("[ERROR] Uso: guardar <nombre_archivo>\n");
                continue;
            }
            if (guardar_grafo(grafo, archivo_nombre) == 0)
            {
                printf("[OK] Guardado en %s\n", archivo_nombre);
            }
            else
            {
                printf("[ERROR] Fallo guardar.\n");
            }
            continue;
        }

        if (strcmp(token, "cargar-grafo") == 0)
        {
            archivo_nombre = strtok(NULL, " \n");
            if (!archivo_nombre)
            {
                printf("[ERROR] Uso: cargar-grafo <nombre_archivo>\n");
                continue;
            }

            if (cargar_grafo(grafo, archivo_nombre) == 0)
            {
                printf("[OK] Cargado %s\n", archivo_nombre);
            }
            else
            {
                printf("[ERROR] Fallo cargar %s\n", archivo_nombre);
            }

            continue;
        }

        if (strcmp(token, "nuevo-disp") == 0)
        {
            nombre = strtok(NULL, " \n");
            ip_cadena = strtok(NULL, " \n");
            tipo_str = strtok(NULL, " \n");
            cap_str = strtok(NULL, " \n");

            if (!nombre || !ip_cadena || !tipo_str || !cap_str)
            {
                printf("[ERROR] Uso: nuevo-disp <nombre> <ip> <tipo> <cap>\n");
                continue;
            }

            tipo_disp = D_DEFAULT;

            if (strcmp(tipo_str, "router") == 0)
            {
                tipo_disp = D_ROUTER;
            }
            else if (strcmp(tipo_str, "switch") == 0)
            {
                tipo_disp = D_SWITCH;
            }
            else if (strcmp(tipo_str, "host") == 0)
            {
                tipo_disp = D_HOST;
            }
            else if (strcmp(tipo_str, "servidor") == 0)
            {
                tipo_disp = D_SERVIDOR;
            }

            indice = agregar_vertice(grafo, nombre, ip_cadena, tipo_disp, atoi(cap_str));

            if (indice != -1)
            {
                printf("[OK] Dispositivo agregado: %s\n", nombre);
            }
            else
            {
                printf("[ERROR] No se pudo agregar dispositivo.\n");
            }

            guardar_grafo(grafo, archivo_default);

            continue;
        }

        if (strcmp(token, "conectar-dispositivo") == 0)
        {
            origen_str = strtok(NULL, " \n");
            destino_str = strtok(NULL, " \n");
            lat_str = strtok(NULL, " \n");
            bw_str = strtok(NULL, " \n");
            fi_str = strtok(NULL, " \n");

            if (!origen_str || !destino_str || !lat_str || !bw_str || !fi_str)
            {
                printf("[ERROR] Uso: conectar-dispositivo <orig> <dest> <lat> <bw> <fiab>\n");
                continue;
            }

            indice_origen = indice_por_nombre(grafo, origen_str);
            indice_destino = indice_por_nombre(grafo, destino_str);

            if (indice_origen == -1 || indice_destino == -1)
            {
                printf("[ERROR] Dispositivo origen/destino no existe.\n");
                continue;
            }

            if (agregar_arista(grafo, indice_origen, indice_destino, atoi(lat_str), atoi(bw_str), atof(fi_str), 1) == 0)
            {
                printf("[OK] Conexion agregada: %s -> %s\n", origen_str, destino_str);
            }
            else
            {
                printf("[ERROR] No se pudo agregar conexion.\n");
            }

            guardar_grafo(grafo, archivo_default);

            continue;
        }

        if (strcmp(token, "ping") == 0)
        {
            origen_str = strtok(NULL, " \n");
            destino_str = strtok(NULL, " \n");
            ct_str = strtok(NULL, " \n");
            contador = 4;

            if (ct_str)
            {
                contador = atoi(ct_str);
            }
            if (!origen_str || !destino_str)
            {
                printf("[ERROR] Uso: ping <origen> <destino> [count]\n");
                continue;
            }

            resolver_ping(grafo, origen_str, destino_str, contador);
            continue;
        }

        if (strcmp(token, "traceroute") == 0)
        {
            origen_str = strtok(NULL, " \n");
            destino_str = strtok(NULL, " \n");
            ks_str = strtok(NULL, " \n");

            k = 3;

            if (ks_str)
            {
                k = atoi(ks_str);
            }

            if (!origen_str || !destino_str)
            {
                printf("[ERROR] Uso: traceroute <origen> <destino> [K]\n");
                continue;
            }

            comando_traceroute(grafo, origen_str, destino_str, k);
            continue;
        }
        /*
        if (strcmp(token, "fallar-disp") == 0)
        {
            nombre = strtok(NULL, " \n");
            if (!nombre)
            {
                printf("[ERROR] Uso: fallar-disp <nombre>\n");
                continue;
            }
            indice = indice_por_nombre(grafo, nombre);
            if (indice == -1)
            {
                printf("[ERROR] Dispositivo no existe.\n");
                continue;
            }
            establecer_estado_vertice(grafo, indice, 0);
            printf("[OK] Dispositivo %s marcado como FALLIDO.\n", nombre);

            guardar_grafo(grafo, archivo_default);
            continue;
        }
        */

        if (strcmp(token, "fallar-enlace") == 0)
        {
            origen_str = strtok(NULL, " \n");
            destino_str = strtok(NULL, " \n");

            if (!origen_str || !destino_str)
            {
                printf("[ERROR] Uso: fallar-enlace <origen> <destino>\n");
                continue;
            }

            indice_origen = indice_por_nombre(grafo, origen_str);
            indice_destino = indice_por_nombre(grafo, destino_str);
            if (indice_origen == -1 || indice_destino == -1)
            {
                printf("[ERROR] Dispositivo origen/destino no existe.\n");
                continue;
            }
            establecer_estado_arista(grafo, indice_origen, indice_destino, 0);
            printf("[OK] Enlace %s -> %s desactivado.\n", origen_str, destino_str);
            guardar_grafo(grafo, archivo_default);
            continue;
        }

        if (strcmp(token, "analizar-resiliencia") == 0)
        {
            comando_analizar_resiliencia(grafo);
            continue;
        }

        if (strcmp(token, "optimizar-ruta") == 0)
        {
            origen_str = strtok(NULL, " \n");
            destino_str = strtok(NULL, " \n");

            if (!origen_str || !destino_str)
            {
                printf("[ERROR] Uso: optimizar-ruta <origen> <destino>\n");
                continue;
            }
            comando_optimizar_ruta(grafo, origen_str, destino_str);
            continue;
        }

        if (strcmp(token, "visualizar-grafo") == 0)
        {
            // Llamar a la función de visualización aquí fork()
            pid = fork();

            if (pid < 0)
            {
                printf("[ERROR] No se pudo abrir el visualizador grafico.\n");
            }
            else if (pid == 0)
            {
                execlp("python3", "python3", "src/verGrafoL.py", (char *)NULL);
            }
            else
            {
                pidPython = pid;
                printf("[OK] Visualizador grafico iniciado.\n");
            }

            continue;
        }

        if (strcmp(token, "limpiar") == 0)
        {
            LIMPIAR;
            continue;
        }

        printf("[ERROR] Comando no reconocido. Escribe 'ayuda'.\n");
    }

    if (pidPython > 0)
    {
        if (kill(pidPython, SIGTERM) == 0)
        {
            waitpid(pidPython, NULL, 0);
            printf("[OK] Visualizador grafico terminado.\n");
        }
        else
        {
            if (errno == ESRCH)
            {
                printf("[INFO] Visualizador grafico ya habia terminado.\n");
            }
            else
            {
                printf("[ERROR] No se pudo terminar el visualizador grafico.\n");
            }
        }
    }

    liberar_grafo(grafo);
    return 0;
}

/* Funcion de coste por latencia */
double costo_por_latencia(const ARISTA *ar)
{
    return (double)ar->latencia_ms;
}

/* impresion de ayuda */
void imprimir_ayuda()
{
    printf("Comandos y Usos:\n\n");
    printf("nuevo-disp <nombre> <ip> <tipo> <cap>\n");
    printf("conectar-dispositivo <origen> <destino> <lat> <bw> <fiab>\n");
    printf("guardar <nombre_archivo>\n");
    printf("ping <origen> <destino> [count]\n");
    printf("traceroute <origen> <destino> [K]\n");
    printf("fallar-enlace <origen> <destino>\n");
    printf("analizar-resiliencia\n");
    printf("optimizar-ruta <origen> <destino>\n");
    printf("ver-grafo\n");
    printf("visualizar-grafo\n");
    printf("limpiar\n");
    printf("ayuda\n");
    printf("salir\n\n");
}

/* Imprimir camino a partir de índices */
void imprimir_camino_por_indices(GRAFO *grafo, const int *camino, int len)
{
    int i;
    if (!grafo || !camino || len <= 0)
        return;

    i = 0;
    for (i = 0; i < len; ++i)
    {
        printf("%s", grafo->vertices[camino[i]].nombre);
        if (i + 1 < len)
            printf(" -> ");
    }
    printf("\n");
}

/* PING con simulacion de pérdida */
void resolver_ping(GRAFO *grafo, const char *origen_nombre, const char *dest_nombre, int cuenta)
{
    int indice_origen, indice_destino, anterior[256], i, u, v, perdido, camino[256], enviados, recibidos, prueba;
    double distancia[256], acumulada_lat, r, rtt_min, rtt_max, rtt_sum;
    int longitud_camino;
    ARISTA *ar, *ar_seleccionada;

    if (cuenta <= 0)
        cuenta = 4;

    indice_origen = indice_por_nombre(grafo, origen_nombre);
    indice_destino = indice_por_nombre(grafo, dest_nombre);

    if (indice_origen == -1 || indice_destino == -1)
    {
        printf("[ERROR] Nodo no encontrado.\n");
        return;
    }

    dijkstra_camino_minimo(grafo, indice_origen, indice_destino, costo_por_latencia, anterior, distancia);
    if (distancia[indice_destino] >= DBL_MAX / 2)
    {
        printf("[PING] No hay camino entre %s y %s.\n", origen_nombre, dest_nombre);
        return;
    }
    longitud_camino = reconstruir_camino(anterior, indice_destino, camino, 256);
    if (longitud_camino <= 0)
    {
        printf("[PING] Error reconstruyendo ruta.\n");
        return;
    }

    printf("[PING] Ruta seleccionada: ");
    imprimir_camino_por_indices(grafo, camino, longitud_camino);

    enviados = 0;
    recibidos = 0;
    rtt_min = 1e9;
    rtt_max = 0;
    rtt_sum = 0;

    prueba = 0;
    for (prueba = 0; prueba < cuenta; ++prueba)
    {
        enviados++;
        acumulada_lat = 0.0;
        perdido = 0;

        i = 0;
        for (i = 0; i < longitud_camino - 1; ++i)
        {
            u = camino[i];
            v = camino[i + 1];
            ar = grafo->vertices[u].lista_adyacencia;
            ar_seleccionada = NULL;
            while (ar)
            {
                if (ar->destino == v && ar->activo)
                {
                    ar_seleccionada = ar;
                    break;
                }
                ar = ar->siguiente;
            }
            if (!ar_seleccionada)
            {
                perdido = 1;
                break;
            }

            r = ((double)rand()) / ((double)RAND_MAX);
            if (r > ar_seleccionada->fiabilidad)
            {
                perdido = 1;
                break;
            }
            acumulada_lat += (double)ar_seleccionada->latencia_ms;
        }
        if (!perdido)
        {
            recibidos++;
            rtt_sum += acumulada_lat;

            if (acumulada_lat < rtt_min)
                rtt_min = acumulada_lat;
            if (acumulada_lat > rtt_max)
                rtt_max = acumulada_lat;
            printf("[prueba %d] Respuesta de %s: tiempo=%.2f ms\n", prueba + 1, dest_nombre, acumulada_lat);
        }
        else
        {
            printf("[prueba %d] Tiempo de espera agotado.\n", prueba + 1);
        }
    }
    printf("ESTADISTICAS DE PING\n");
    printf("%d paquetes transmitidos, %d recibidos, %.1f%% de pérdida\n", enviados, recibidos, 100.0 * (double)(enviados - recibidos) / (double)enviados);
    if (recibidos > 0)
    {
        printf("rtt min/prom/max = %.2f/%.2f/%.2f ms\n", rtt_min, rtt_sum / recibidos, rtt_max);
    }
}

/* TRACEROUTE (K rutas aproximadas) */
void comando_traceroute(GRAFO *grafo, const char *origen_nombre, const char *dest_nombre, int K)
{
    int indice_origen, indice_destino, caminos[10][256], longitudes[10], bwmin, i;
    double lat, fiab;

    if (K <= 0)
        K = 3;
    indice_origen = indice_por_nombre(grafo, origen_nombre);
    indice_destino = indice_por_nombre(grafo, dest_nombre);
    if (indice_origen == -1 || indice_destino == -1)
    {
        printf("[ERROR] Nodo no encontrado.\n");
        return;
    }

    int encontrados = encontrar_k_rutas_aproximadas(grafo, indice_origen, indice_destino, costo_por_latencia, K, caminos, longitudes, 256);
    if (encontrados == 0)
    {
        printf("[TRACEROUTE] No se encontraron rutas.\n");
        return;
    }
    printf("[TRACEROUTE] Se encontraron %d rutas:\n", encontrados);
    i = 0;
    for (i = 0; i < encontrados; ++i)
    {
        if (calcular_metricas_ruta(grafo, caminos[i], longitudes[i], &lat, &bwmin, &fiab) == 0)
        {
            printf(" Ruta %d: saltos=%d lat=%.2fms bw_min=%dMbps fiab=%.4f\n", i + 1, longitudes[i] - 1, lat, bwmin, fiab);
            imprimir_camino_por_indices(grafo, caminos[i], longitudes[i]);
        }
        else
        {
            printf(" Ruta %d: (error al calcular métricas)\n", i + 1);
        }
    }
}

/* BFS simple para contar alcanzables */
int contar_alcanzables(GRAFO *grafo, int indice)
{
    int visitado[256] = {0}, cola[256], cabeza, cola_tail, u, v;
    ARISTA *ar;

    if (!grafo || indice < 0 || indice >= grafo->num_vertices)
        return 0;

    cabeza = 0;
    cola_tail = 0;

    if (grafo->vertices[indice].activo == 0)
        return 0;

    cola[cola_tail++] = indice;
    visitado[indice] = 1;

    while (cabeza < cola_tail)
    {
        u = cola[cabeza++];
        ar = grafo->vertices[u].lista_adyacencia;
        while (ar)
        {
            if (ar->activo)
            {
                v = ar->destino;
                if (!visitado[v] && grafo->vertices[v].activo)
                {
                    visitado[v] = 1;
                    cola[cola_tail++] = v;
                }
            }
            ar = ar->siguiente;
        }
    }
    return cola_tail;
}

/* ANALIZAR RESILIENCIA */
void comando_analizar_resiliencia(GRAFO *grafo)
{
    int n, i, inicio, alcanzables, peor_indice, peor_impacto, saved, r, impacto, nb, j, A, B, existe;
    int vecinos[256];
    ARISTA *ar, *aa;

    if (!grafo)
        return;
    n = grafo->num_vertices;
    if (n == 0)
    {
        printf("Grafo vacío.\n");
        return;
    }
    inicio = -1;

    i = 0;
    for (i = 0; i < n; ++i)
    {
        if (grafo->vertices[i].activo)
        {
            inicio = i;
            break;
        }
    }
    if (inicio == -1)
    {
        printf("Todos los nodos están fallidos.\n");
        return;
    }
    alcanzables = contar_alcanzables(grafo, inicio);

    printf("[RESILIENCE] Nodos totales: %d. Alcanzables desde %s: %d\n", n, grafo->vertices[inicio].nombre, alcanzables);
    peor_indice = -1;
    peor_impacto = -1;

    i = 0;
    for (i = 0; i < n; ++i)
    {
        saved = grafo->vertices[i].activo;
        grafo->vertices[i].activo = 0;
        r = contar_alcanzables(grafo, inicio == i ? ((inicio == 0 && n > 1) ? 1 : 0) : inicio);
        impacto = alcanzables - r;
        grafo->vertices[i].activo = saved;
        printf(" - Si falla %s -> impacto: %d nodos no alcanzables\n", grafo->vertices[i].nombre, impacto);
        if (impacto > peor_impacto)
        {
            peor_impacto = impacto;
            peor_indice = i;
        }
    }
    if (peor_indice != -1)
    {
        printf("[RESILIENCE] Nodo crítico identificado: %s (impacto=%d)\n", grafo->vertices[peor_indice].nombre, peor_impacto);
        nb = 0;
        ar = grafo->vertices[peor_indice].lista_adyacencia;
        while (ar)
        {
            vecinos[nb++] = ar->destino;
            ar = ar->siguiente;
        }
        printf("[RECOMENDACION] Considerar añadir enlaces redundantes entre los vecinos del nodo crítico.\n");
        if (nb >= 2)
        {
            i = 0;
            for (i = 0; i < nb; ++i)
            {
                j = 0;
                for (j = i + 1; j < nb; ++j)
                {
                    A = vecinos[i];
                    B = vecinos[j];
                    existe = 0;
                    aa = grafo->vertices[A].lista_adyacencia;
                    while (aa)
                    {
                        if (aa->destino == B)
                        {
                            existe = 1;
                            break;
                        }
                        aa = aa->siguiente;
                    }
                    if (!existe)
                        printf(" - Sugerencia: conectar %s <--> %s\n", grafo->vertices[A].nombre, grafo->vertices[B].nombre);
                }
            }
        }
        else
            printf(" - No se encontraron suficientes vecinos para sugerir enlaces.\n");
    }
}

/* OPTIMIZE-ROUTE heurística simple */
void comando_optimizar_ruta(GRAFO *grafo, const char *origen_nombre, const char *dest_nombre)
{
    int indice_origen, indice_destino, anterior[256], try_lat, try_bw;
    double distancia[256], actual, try_f, mejorado;
    ARISTA *ar, *p, *prevp;

    indice_origen = indice_por_nombre(grafo, origen_nombre);
    indice_destino = indice_por_nombre(grafo, dest_nombre);

    if (indice_origen == -1 || indice_destino == -1)
    {
        printf("[ERROR] Nodo no encontrado.\n");
        return;
    }

    ar = grafo->vertices[indice_origen].lista_adyacencia;
    while (ar)
    {
        if (ar->destino == indice_destino && ar->activo)
        {
            printf("[OPT] Ya existe enlace directo %s -> %s\n", origen_nombre, dest_nombre);
            return;
        }
        ar = ar->siguiente;
    }

    dijkstra_camino_minimo(grafo, indice_origen, indice_destino, costo_por_latencia, anterior, distancia);
    if (distancia[indice_destino] >= DBL_MAX / 2)
    {
        printf("[OPT] No hay camino actual entre ambos nodos.\n");
        return;
    }

    actual = distancia[indice_destino];
    try_lat = 10;
    try_bw = 100;
    try_f = 0.99;
    agregar_arista(grafo, indice_origen, indice_destino, try_lat, try_bw, try_f, 1);
    dijkstra_camino_minimo(grafo, indice_origen, indice_destino, costo_por_latencia, anterior, distancia);
    mejorado = distancia[indice_destino];
    p = grafo->vertices[indice_origen].lista_adyacencia;
    prevp = NULL;

    while (p)
    {
        if (p->destino == indice_destino && p->latencia_ms == try_lat && p->ancho_banda_mbps == try_bw && p->fiabilidad == try_f)
        {
            if (prevp)
                prevp->siguiente = p->siguiente;
            else
                grafo->vertices[indice_origen].lista_adyacencia = p->siguiente;
            free(p);
            break;
        }
        prevp = p;
        p = p->siguiente;
    }
    printf("[OPT] Latencia actual: %.2f ms. Latencia con enlace hipotético: %.2f ms\n", actual, mejorado);
    if (mejorado < 0.8 * actual)
    {
        printf("[OPT] Recomendacion: añadir enlace directo %s -> %s (lat=%d ms, bw=%d Mbps, fiab=%.2f)\n", origen_nombre, dest_nombre, try_lat, try_bw, try_f);
    }
    else
    {
        printf("[OPT] No se recomienda añadir enlace directo; mejora insuficiente.\n");
    }
}
