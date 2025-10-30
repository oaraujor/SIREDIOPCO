import pygame
import math
import sys
import time
import os

WIDTH, HEIGHT = 800, 600
RADIUS = 40
FONT_SIZE = 34
ARROW_SIZE = 22
VERTEX_COLOR = (0, 255, 0)
EDGE_COLOR = (255, 255, 255)
TEXT_COLOR = (255, 255, 255)
LOOP_COLOR = (255, 255, 255)
BACKGROUND = (0, 0, 0)
EDGE_WIDTH = 4
REFRESH_INTERVAL = 1.0  # segundos

def leer_grafo(filename):
    vertices = []
    aristas = []
    with open(filename, 'r') as f:
        lines = f.readlines()
        modo = None
        for line in lines:
            line = line.strip()
            if line == "VERTICES":
                modo = "V"
            elif line == "ARISTAS":
                modo = "A"
            elif line and modo == "V":
                vertices.append(line)
            elif line and modo == "A":
                partes = line.split()
                if len(partes) == 3:
                    aristas.append((partes[0], partes[1], int(partes[2])))
    return vertices, aristas

def get_vertex_positions(n):
    cx, cy = WIDTH // 2, HEIGHT // 2
    radio = min(WIDTH, HEIGHT) // 2 - 80
    pos = []
    for i in range(n):
        ang = 2 * math.pi * i / n if n > 0 else 0
        x = cx + int(radio * math.cos(ang))
        y = cy + int(radio * math.sin(ang))
        pos.append((x, y))
    return pos

def draw_arrow(surface, start, end, color=EDGE_COLOR, width=EDGE_WIDTH):
    pygame.draw.line(surface, color, start, end, width)
    dx, dy = end[0]-start[0], end[1]-start[1]
    angle = math.atan2(dy, dx)
    arrow_p1 = (end[0] - ARROW_SIZE * math.cos(angle - math.pi/6),
                end[1] - ARROW_SIZE * math.sin(angle - math.pi/6))
    arrow_p2 = (end[0] - ARROW_SIZE * math.cos(angle + math.pi/6),
                end[1] - ARROW_SIZE * math.sin(angle + math.pi/6))
    pygame.draw.polygon(surface, color, [end, arrow_p1, arrow_p2])

def draw_loop(surface, pos, color=LOOP_COLOR, peso=1):
    x, y = pos
    loop_rect = pygame.Rect(x + RADIUS//2, y - RADIUS//2, RADIUS, RADIUS)
    pygame.draw.arc(surface, color, loop_rect, math.pi*0.2, math.pi*1.7, EDGE_WIDTH)
    font = pygame.font.SysFont(None, FONT_SIZE//2)
    text = font.render(str(peso), True, LOOP_COLOR)
    surface.blit(text, (x + RADIUS + 5, y - RADIUS//2))

def main():
    pygame.init()
    screen = pygame.display.set_mode((WIDTH, HEIGHT))
    pygame.display.set_caption("Visualizador de Grafo Dirigido")
    font = pygame.font.SysFont(None, FONT_SIZE)
    clock = pygame.time.Clock()
    filename = "txt/matrixGrafoL.txt"


    vertices, aristas = leer_grafo(filename)
    positions = get_vertex_positions(len(vertices))
    vert_index = {v: i for i, v in enumerate(vertices)}
    last_mtime = os.path.getmtime(filename)
    last_refresh = time.time()

    running = True
    while running:
        now = time.time()
        current_mtime = os.path.getmtime(filename)
        if now - last_refresh > REFRESH_INTERVAL or current_mtime != last_mtime:
            last_refresh = now
            last_mtime = current_mtime
            vertices, aristas = leer_grafo(filename)
            positions = get_vertex_positions(len(vertices))
            vert_index = {v: i for i, v in enumerate(vertices)}

        screen.fill(BACKGROUND)
        # Dibuja las aristas
        for orig, dest, peso in aristas:
            i, j = vert_index[orig], vert_index[dest]
            orig_pos = positions[i]
            dest_pos = positions[j]
            if orig != dest:
                dx, dy = dest_pos[0]-orig_pos[0], dest_pos[1]-orig_pos[1]
                dist = math.hypot(dx, dy)
                if dist == 0: dist = 1
                start = (orig_pos[0] + RADIUS*dx/dist, orig_pos[1] + RADIUS*dy/dist)
                end = (dest_pos[0] - RADIUS*dx/dist, dest_pos[1] - RADIUS*dy/dist)
                draw_arrow(screen, start, end)
                mx = (start[0] + end[0]) // 2
                my = (start[1] + end[1]) // 2
                text = font.render(str(peso), True, EDGE_COLOR)
                screen.blit(text, (mx, my))
            else:
                draw_loop(screen, orig_pos, peso=peso)
        # Dibuja los vértices
        for i, v in enumerate(vertices):
            x, y = positions[i]
            pygame.draw.circle(screen, VERTEX_COLOR, (x, y), RADIUS)
            pygame.draw.circle(screen, EDGE_COLOR, (x, y), RADIUS, EDGE_WIDTH)
            text = font.render(v, True, TEXT_COLOR)
            text_rect = text.get_rect(center=(x, y))
            screen.blit(text, text_rect)

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False

        pygame.display.flip()
        clock.tick(30)

    pygame.quit()
    sys.exit()

if __name__ == "__main__":
        main()