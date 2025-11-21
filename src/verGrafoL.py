#!/usr/bin/env python3
# Visualizador de grafo dirigido adaptado al formato de guardar_grafo / cargar_grafo (C).
# Lee archivos con líneas:
#  NS <num>
#  N <nombre> <ip> <tipo_int> <cap>
#  AS <num>
#  A <origen> <destino> <lat> <bw> <fiab> <activo>
# Opcionalmente acepta líneas de estado de vértice (no producido por guardar_grafo,
# pero útiles para marcar manualmente nodos caídos):
#  V <nombre> <activo>    donde activo es 0 o 1
#
# En la visualización se muestran solo:
#  - los nodos (con el texto de su IP)
#  - las aristas (flechas)
# Colores:
#  - Routers (tipo 0): azul
#  - Switches (tipo 1): verde
#  - Hosts (tipo 2): amarillo
#  - Servidores (tipo 3): cian
#  - Default (tipo 4 o no reconocido): blanco
#  - Si un dispositivo está marcado como caído (activo==0) -> rojo
#  - Enlaces activos -> blanco, enlaces caídos -> rojo
#
# Uso: ajustar la variable FILENAME o pasar ruta como primer argumento.

import pygame
import math
import sys
import time
import os

# --- Configuración de la ventana / estilo ---
WIDTH, HEIGHT = 1000, 700
RADIUS = 36
FONT_SIZE = 16
ARROW_SIZE = 16
EDGE_WIDTH = 3
BACKGROUND = (10, 10, 10)
REFRESH_INTERVAL = 1.0  # segundos

# Colores por tipo
COLOR_ROUTER = (0, 102, 204)   # azul
COLOR_SWITCH = (0, 180, 0)     # verde
COLOR_HOST = (230, 200, 0)     # amarillo
COLOR_SERVIDOR = (0, 180, 180) # cian
COLOR_DEFAULT = (220, 220, 220)
COLOR_DOWN = (220, 30, 30)     # rojo para nodos caídos
COLOR_EDGE_ACTIVE = (240, 240, 240)
COLOR_EDGE_DOWN = (220, 30, 30)
TEXT_COLOR = (240, 240, 240)

# Nombre por defecto del archivo (se puede sobreescribir con argumento)
DEFAULT_FILENAME = "txt/topologia.txt"

def parse_guardado(filename):
    """
    Parsea el fichero en el formato usado por guardar_grafo/cargar_grafo.
    Devuelve:
        vertices: lista de dict {name, ip, tipo (int), cap (int), activo (1/0)}
        aristas: lista de dict {origen, destino, lat, bw, fiab, activo (1/0)}
    Acepta opcionalmente líneas 'V <nombre> <activo>' para marcar vértices caídos.
    """
    vertices = []
    name_to_index = {}
    aristas = []

    if not os.path.exists(filename):
        return vertices, aristas

    with open(filename, 'r', encoding='utf-8') as f:
        for raw in f:
            line = raw.strip()
            if not line or line.startswith('#'):
                continue
            parts = line.split()
            if not parts:
                continue
            tag = parts[0]
            if tag == "N" and len(parts) >= 4:
                # N <nombre> <ip> <tipo_int> <cap>
                # guardado por el C: exactamente 4 campos después de la N
                nombre = parts[1]
                ip = parts[2]
                try:
                    tipo = int(parts[3])
                except:
                    tipo = 4
                cap = 0
                if len(parts) >= 5:
                    try:
                        cap = int(parts[4])
                    except:
                        cap = 0
                v = {"name": nombre, "ip": ip, "tipo": tipo, "cap": cap, "activo": 1}
                name_to_index[nombre] = len(vertices)
                vertices.append(v)
            elif tag == "A" and len(parts) >= 7:
                # A <origen> <destino> <lat> <bw> <fiab> <activo>
                origen = parts[1]
                destino = parts[2]
                try:
                    lat = int(parts[3])
                except:
                    lat = 0
                try:
                    bw = int(parts[4])
                except:
                    bw = 0
                try:
                    fiab = float(parts[5])
                except:
                    fiab = 0.0
                try:
                    activo = int(parts[6])
                except:
                    activo = 1
                ar = {"origen": origen, "destino": destino, "lat": lat, "bw": bw, "fiab": fiab, "activo": 1 if activo != 0 else 0}
                aristas.append(ar)
            elif tag == "V" and len(parts) >= 3:
                # Línea opcional: V <nombre> <activo>  (activo 1/0)
                nombre = parts[1]
                try:
                    activo = int(parts[2])
                except:
                    activo = 1
                # si ya existe el vértice, marcar su estado
                if nombre in name_to_index:
                    idx = name_to_index[nombre]
                    vertices[idx]["activo"] = 1 if activo != 0 else 0
            else:
                # Ignorar otras etiquetas (NS, AS, etc.)
                continue

    # Nota: el formato estándar no guarda el estado de vértices; si no hay
    # líneas V, todos los vértices se consideran activos. Si quieres marcar
    # manualmente un nodo como caído, añade una línea "V <nombre> 0" en el archivo.
    return vertices, aristas

def get_vertex_positions(n, width, height):
    cx, cy = width // 2, height // 2
    radio = min(width, height) // 2 - 120
    if radio < 100:
        radio = min(width, height) // 2 - 40
    pos = []
    for i in range(n):
        ang = 2 * math.pi * i / n if n > 0 else 0
        x = cx + int(radio * math.cos(ang))
        y = cy + int(radio * math.sin(ang))
        pos.append((x, y))
    return pos

def draw_arrow(surface, start, end, color=(255,255,255), width=EDGE_WIDTH):
    # dibuja línea con cabeza de flecha en 'end'
    pygame.draw.line(surface, color, start, end, width)
    dx, dy = end[0] - start[0], end[1] - start[1]
    angle = math.atan2(dy, dx)
    # puntos de la cabeza
    p1 = (end[0] - ARROW_SIZE * math.cos(angle - math.pi/6),
          end[1] - ARROW_SIZE * math.sin(angle - math.pi/6))
    p2 = (end[0] - ARROW_SIZE * math.cos(angle + math.pi/6),
          end[1] - ARROW_SIZE * math.sin(angle + math.pi/6))
    pygame.draw.polygon(surface, color, [end, p1, p2])

def color_for_vertex(tipo, activo):
    if not activo:
        return COLOR_DOWN
    if tipo == 0:
        return COLOR_ROUTER
    if tipo == 1:
        return COLOR_SWITCH
    if tipo == 2:
        return COLOR_HOST
    if tipo == 3:
        return COLOR_SERVIDOR
    return COLOR_DEFAULT

def main():
    pygame.init()
    screen = pygame.display.set_mode((WIDTH, HEIGHT))
    pygame.display.set_caption("Visualizador de Grafo (formato guardar_grafo/cargar_grafo)")
    base_font = pygame.font.SysFont(None, FONT_SIZE)
    clock = pygame.time.Clock()

    # usar argumento de línea si se pasa
    filename = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_FILENAME

    # lectura inicial
    vertices, aristas = parse_guardado(filename)
    positions = get_vertex_positions(len(vertices), WIDTH, HEIGHT)
    name_to_index = {v["name"]: i for i, v in enumerate(vertices)}
    last_mtime = os.path.getmtime(filename) if os.path.exists(filename) else 0
    last_refresh = time.time()

    running = True
    while running:
        now = time.time()
        try:
            current_mtime = os.path.getmtime(filename) if os.path.exists(filename) else 0
        except Exception:
            current_mtime = 0

        if now - last_refresh > REFRESH_INTERVAL or current_mtime != last_mtime:
            last_refresh = now
            last_mtime = current_mtime
            try:
                vertices, aristas = parse_guardado(filename)
                positions = get_vertex_positions(len(vertices), WIDTH, HEIGHT)
                name_to_index = {v["name"]: i for i, v in enumerate(vertices)}
            except Exception as e:
                print("Error leyendo archivo:", e)

        # fondo
        screen.fill(BACKGROUND)

        # dibujar aristas (primero para que queden debajo de nodos)
        for ar in aristas:
            origen_name = ar["origen"]
            destino_name = ar["destino"]
            activo_ar = ar.get("activo", 1)
            # si alguno de los nodos no existe, ignorar la arista (mensaje en consola)
            if origen_name not in name_to_index or destino_name not in name_to_index:
                # Ignorar arista no válidas
                continue
            i = name_to_index[origen_name]
            j = name_to_index[destino_name]
            orig_pos = positions[i]
            dest_pos = positions[j]

            # calcular puntos de inicio/fin desplazados por el radio
            dx, dy = dest_pos[0] - orig_pos[0], dest_pos[1] - orig_pos[1]
            dist = math.hypot(dx, dy)
            if dist == 0:
                # self-loop: dibujar arco
                loop_rect = pygame.Rect(orig_pos[0] + RADIUS//2, orig_pos[1] - RADIUS//2, RADIUS, RADIUS)
                color = COLOR_EDGE_ACTIVE if activo_ar else COLOR_EDGE_DOWN
                pygame.draw.arc(screen, color, loop_rect, math.pi*0.2, math.pi*1.7, EDGE_WIDTH)
                continue
            start = (orig_pos[0] + (RADIUS * dx / dist), orig_pos[1] + (RADIUS * dy / dist))
            end = (dest_pos[0] - (RADIUS * dx / dist), dest_pos[1] - (RADIUS * dy / dist))

            color = COLOR_EDGE_ACTIVE if activo_ar else COLOR_EDGE_DOWN
            draw_arrow(screen, start, end, color=color, width=EDGE_WIDTH)

        # dibujar nodos encima de aristas
        for idx, v in enumerate(vertices):
            x, y = positions[idx]
            tipo = v.get("tipo", 4)
            activo_v = v.get("activo", 1)

            fill_color = color_for_vertex(tipo, activo_v)
            border_color = (60, 60, 60) if activo_v else COLOR_DOWN

            # círculo de fondo
            pygame.draw.circle(screen, fill_color, (x, y), RADIUS)
            # borde
            pygame.draw.circle(screen, border_color, (x, y), RADIUS, 3)

            # mostrar solo la IP centrada
            ip_text = v.get("ip", "")
            text_surf = base_font.render(ip_text, True, TEXT_COLOR)
            text_rect = text_surf.get_rect(center=(x, y))
            screen.blit(text_surf, text_rect)

        # eventos
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False

        pygame.display.flip()
        clock.tick(30)

    pygame.quit()
    sys.exit()

if __name__ == "__main__":
    main()