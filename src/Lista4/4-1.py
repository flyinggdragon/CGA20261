# Câmera FPS - Exemplo 5
#
# Este exemplo é idêntico ao Exemplo 4 - mesma cena, mesmo ray tracing,
# mesma reflexão. A única adição é a câmera interativa no estilo FPS:
#   W/A/S/D - mover câmera
#   Mouse   - girar câmera
#
# Isso permite orbitar a esfera espelho e observar o reflexo de diferentes
# ângulos - algo impossível de fazer nos exemplos anteriores com câmera fixa.
#
# Como funciona a câmera FPS aqui:
#   Nos exemplos anteriores, cam_origem era um valor fixo calculado uma vez.
#   Agora, cam_origem muda a cada frame conforme o movimento do jogador.
#   Os vetores horizontal, vertical e canto_inf_esq são recalculados todo
#   frame a partir da nova posição e orientação da câmera.
#
#   O shader não sabe que a câmera se moveu - ele recebe os mesmos uniforms
#   de sempre (cam_origem, cam_horizontal, etc.), só que com valores novos.
#   Isso mostra que o ray tracer no shader é completamente independente de
#   como a câmera é controlada no Python.
#
# Controles:
#   W/A/S/D - mover câmera
#   Mouse   - girar câmera
#   ESC     - fechar

import glfw
from OpenGL.GL import *
import OpenGL.GL.shaders
import numpy as np

# -----------------------------
# Variáveis globais
# -----------------------------

Window = None
Shader_programm = None
Vao_quad = None

WIDTH = 800
HEIGHT = 600

Tempo_entre_frames = 0.0

# -----------------------------
# Estado da câmera FPS
# -----------------------------
# A câmera é definida por posição, yaw e pitch - igual aos exemplos de
# rasterização das aulas anteriores. A diferença é que aqui os vetores
# resultantes são enviados ao ray tracer como uniforms, não como matriz.

Cam_pos = np.array([0.0, 0.5, 4.0], dtype=np.float32)
Cam_yaw = 270.0  # graus - 270 = olhando para -Z (em direção às esferas)
Cam_pitch = -5.0  # graus - levemente inclinado para baixo
Cam_speed = 5.0

lastX, lastY = WIDTH / 2.0, HEIGHT / 2.0
primeiro_mouse = True

# =============================================================================
# INICIALIZAÇÃO DO OPENGL
# =============================================================================


def inicializaOpenGL():
    global Window

    glfw.init()
    Window = glfw.create_window(WIDTH, HEIGHT, "Câmera FPS - Exemplo 5", None, None)
    glfw.make_context_current(Window)

    # Captura o cursor dentro da janela (modo FPS)
    glfw.set_input_mode(Window, glfw.CURSOR, glfw.CURSOR_DISABLED)

    glfw.set_key_callback(Window, key_callback)
    glfw.set_cursor_pos_callback(Window, mouse_callback)

    print("Placa de vídeo:", glGetString(GL_RENDERER))
    print("Versão OpenGL: ", glGetString(GL_VERSION))


def key_callback(window, key, scancode, action, mode):
    if key == glfw.KEY_ESCAPE and action == glfw.PRESS:
        glfw.set_window_should_close(window, True)


def mouse_callback(window, xpos, ypos):
    """
    Chamada automaticamente pelo GLFW toda vez que o mouse se move.
    Atualiza yaw (rotação horizontal) e pitch (rotação vertical) da câmera.
    """
    global lastX, lastY, primeiro_mouse, Cam_yaw, Cam_pitch

    # Na primeira vez que o mouse é capturado, ignoramos o movimento
    # para evitar um salto brusco da câmera
    if primeiro_mouse:
        lastX, lastY = xpos, ypos
        primeiro_mouse = False
        return

    # Quanto o mouse se moveu desde o último frame
    dx = xpos - lastX
    dy = lastY - ypos  # invertido: mouse para cima = pitch positivo (olhar para cima)
    lastX, lastY = xpos, ypos

    sensibilidade = 0.1  # quanto cada pixel de movimento rotaciona a câmera
    Cam_yaw += dx * sensibilidade
    Cam_pitch += dy * sensibilidade

    # Limita o pitch para não deixar a câmera "virar de cabeça pra baixo"
    Cam_pitch = max(-89.0, min(89.0, Cam_pitch))


# =============================================================================
# QUAD DE TELA INTEIRA - idêntico aos exemplos anteriores
# =============================================================================


def inicializaQuad():
    global Vao_quad

    vertices = np.array(
        [
            -1.0,
            -1.0,
            1.0,
            -1.0,
            1.0,
            1.0,
            -1.0,
            -1.0,
            1.0,
            1.0,
            -1.0,
            1.0,
        ],
        dtype=np.float32,
    )

    Vao_quad = glGenVertexArrays(1)
    glBindVertexArray(Vao_quad)

    vbo = glGenBuffers(1)
    glBindBuffer(GL_ARRAY_BUFFER, vbo)
    glBufferData(GL_ARRAY_BUFFER, vertices, GL_STATIC_DRAW)

    glEnableVertexAttribArray(0)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, None)


# =============================================================================
# SHADERS - idênticos ao Exemplo 4
# =============================================================================


def inicializaShaders():
    global Shader_programm

    vertex_shader = """
        #version 400
        layout(location = 0) in vec2 posicao;
        out vec2 uv;
        void main() {
            uv = posicao * 0.5 + 0.5;
            gl_Position = vec4(posicao, 0.0, 1.0);
        }
    """

    # Fragment shader idêntico ao Exemplo 4 - não sabe nada sobre FPS.
    # Recebe os mesmos uniforms de sempre; só os valores mudam a cada frame.
    fragment_shader = """
        #version 400

        in  vec2 uv;
        out vec4 cor_final;

        uniform vec3 cam_origem;
        uniform vec3 cam_horizontal;
        uniform vec3 cam_vertical;
        uniform vec3 cam_canto_inf_esq;

        uniform vec3  esfera_centro;
        uniform float esfera_raio;
        uniform vec3  esfera_cor;
        uniform float esfera_refletividade;

        uniform vec3  esfera2_centro;
        uniform float esfera2_raio;
        uniform vec3  esfera2_cor;
        uniform float esfera2_refletividade;

        uniform vec3  esfera3_centro;
        uniform float esfera3_raio;
        uniform vec3  esfera3_cor;
        uniform float esfera3_refletividade;

        uniform vec3 luzes[5];

        float intersecta_esfera(vec3 origem, vec3 direcao, vec3 centro, float raio) {
            vec3  oc = origem - centro;
            float a  = dot(direcao, direcao);
            float b  = 2.0 * dot(direcao, oc);
            float c  = dot(oc, oc) - raio*raio;
            float discriminante = b*b - 4.0*a*c;
            if (discriminante < 0.0) return -1.0;
            float t1 = (-b - sqrt(discriminante)) / (2.0 * a);
            float t2 = (-b + sqrt(discriminante)) / (2.0 * a);
            if (t1 > 0.0) return t1;
            if (t2 > 0.0) return t2;
            return -1.0;
        }
        
        float intersecta_plano(vec3 origem, vec3 direcao, vec3 ponto_plano, vec3 normal_plano) {
            float denom = dot(normal_plano, direcao);
            if (abs(denom) > 1e-6) { // Evita divisão por zero (raio paralelo ao plano)
                float t = dot(ponto_plano - origem, normal_plano) / denom;
                if (t >= 0.0) return t;
            }
            return -1.0;
        }

        bool intersecta_cena(vec3 origem, vec3 direcao,
                             out vec3 ponto, out vec3 normal,
                             out vec3 cor,   out float reflet) {
            float t_min   = 1e10;
            bool  acertou = false;
            float t;

            t = intersecta_esfera(origem, direcao, esfera_centro, esfera_raio);
            if (t > 0.0 && t < t_min) {
                t_min = t; ponto = origem + t*direcao;
                normal = normalize(ponto - esfera_centro);
                cor = esfera_cor; reflet = esfera_refletividade; acertou = true;
            }
            t = intersecta_esfera(origem, direcao, esfera2_centro, esfera2_raio);
            if (t > 0.0 && t < t_min) {
                t_min = t; ponto = origem + t*direcao;
                normal = normalize(ponto - esfera2_centro);
                cor = esfera2_cor; reflet = esfera2_refletividade; acertou = true;
            }
            t = intersecta_esfera(origem, direcao, esfera3_centro, esfera3_raio);
            if (t > 0.0 && t < t_min) {
                t_min = t; ponto = origem + t*direcao;
                normal = normalize(ponto - esfera3_centro);
                cor = esfera3_cor; reflet = esfera3_refletividade; acertou = true;
            }
            
            // --- NOVO: Teste do Chão (Plano) ---
            vec3 chao_normal = vec3(0.0, 1.0, 0.0); // Chão virado para cima
            vec3 chao_ponto  = vec3(0.0, -1.0, 0.0); // Posicionado abaixo das esferas
            float t_chao = intersecta_plano(origem, direcao, chao_ponto, chao_normal);
            
            if (t_chao > 0.0 && t_chao < t_min) {
                t_min = t_chao;
                ponto = origem + t_chao * direcao;
                normal = chao_normal;
                
                // Efeito de Xadrez (opcional, para dar profundidade)
                float check = mod(floor(ponto.x) + floor(ponto.z), 2.0);
                cor = mix(vec3(0.2), vec3(0.4), check); 
                
                reflet = 0.2; // O chão reflete um pouco as esferas
                acertou = true;
            }
            return acertou;
        }

        bool esta_na_sombra(vec3 luz_posicao, vec3 ponto_de_impacto, vec3 normal) {
            vec3  dir_luz       = normalize(luz_posicao - ponto_de_impacto);
            float dist_ate_luz  = length(luz_posicao - ponto_de_impacto);
            vec3  origem_shadow = ponto_de_impacto + normal * 0.001;
            float t1 = intersecta_esfera(origem_shadow, dir_luz, esfera_centro,  esfera_raio);
            float t2 = intersecta_esfera(origem_shadow, dir_luz, esfera2_centro, esfera2_raio);
            float t3 = intersecta_esfera(origem_shadow, dir_luz, esfera3_centro, esfera3_raio);
            
            // Teste contra o plano (importante para objetos acima do plano)
            float tp = intersecta_plano(origem_shadow, dir_luz, vec3(0.0, -1.0, 0.0), vec3(0.0, 1.0, 0.0));
            return (t1 > 0.0 && t1 < dist_ate_luz)
                || (t2 > 0.0 && t2 < dist_ate_luz)
                || (t3 > 0.0 && t3 < dist_ate_luz);
        }

        vec3 calcula_phong(vec3 luz_posicao, vec3 ponto, vec3 normal, vec3 cor_material) {
            vec3  L         = normalize(luz_posicao - ponto);
            vec3  V         = normalize(cam_origem  - ponto);
            vec3  ambiente  = cor_material * 0.15;
            float Kd        = max(dot(normal, L), 0.0);
            vec3  difusa    = cor_material * Kd * 0.75;
            vec3  R         = reflect(-L, normal);
            float Ks        = pow(max(dot(R, V), 0.0), 32.0);
            vec3  especular = vec3(1.0) * Ks * 0.6;
            return ambiente + difusa + especular;
        }

        vec3 cor_fundo(vec3 direcao) {
            float t = direcao.y * 0.5 + 0.5;
            return mix(vec3(0.1, 0.1, 0.3), vec3(0.5, 0.7, 1.0), t);
        }

        void main() {
            vec3 cor_acumulada = vec3(0.0);

            for (int i = 0; i < 5; i++) {
                vec3 pixel_no_mundo = cam_canto_inf_esq
                                    + uv.x * cam_horizontal
                                    + uv.y * cam_vertical;

                vec3 origem_raio  = cam_origem;
                vec3 direcao_raio = normalize(pixel_no_mundo - cam_origem);

                float peso = 1.0;

                for (int bounce = 0; bounce < 4; bounce++) {
                    vec3  ponto, normal, cor_mat;
                    float reflet;

                    bool acertou = intersecta_cena(origem_raio, direcao_raio,
                                                ponto, normal, cor_mat, reflet);

                    if (!acertou) {
                        cor_acumulada += peso * cor_fundo(direcao_raio);
                        break;
                    }

                    vec3 cor_local;
                    if (esta_na_sombra(luzes[i], ponto, normal)) {
                        cor_local = cor_mat * 0.15;
                    } else {
                        cor_local = calcula_phong(luzes[i], ponto, normal, cor_mat);
                    }

                    cor_acumulada += peso * cor_local * (1.0 - reflet);

                    if (reflet < 0.01) break;

                    peso         *= reflet;
                    origem_raio   = ponto + normal * 0.001;
                    direcao_raio  = reflect(direcao_raio, normal);
                }
            }

            cor_final = vec4(cor_acumulada, 1.0);
        }
    """

    vs = OpenGL.GL.shaders.compileShader(vertex_shader, GL_VERTEX_SHADER)
    fs = OpenGL.GL.shaders.compileShader(fragment_shader, GL_FRAGMENT_SHADER)
    Shader_programm = OpenGL.GL.shaders.compileProgram(vs, fs)


# =============================================================================
# CÂMERA FPS
# =============================================================================


def calculaFront():
    """
    Calcula o vetor 'frente' da câmera a partir do yaw e pitch atuais.

    yaw   = rotação horizontal (esquerda/direita), em graus
    pitch = rotação vertical   (cima/baixo),       em graus

    A fórmula converte ângulos esféricos para vetor cartesiano:
      x = cos(yaw) * cos(pitch)
      y = sin(pitch)
      z = sin(yaw) * cos(pitch)
    """
    yaw_rad = np.radians(Cam_yaw)
    pitch_rad = np.radians(Cam_pitch)

    frente = np.array(
        [
            np.cos(yaw_rad) * np.cos(pitch_rad),
            np.sin(pitch_rad),
            np.sin(yaw_rad) * np.cos(pitch_rad),
        ],
        dtype=np.float32,
    )

    return frente / np.linalg.norm(frente)


def trataTeclado():
    """
    Lê as teclas WASD a cada frame e move a câmera na direção correspondente.
    Multiplicamos pela velocidade e pelo tempo entre frames para que o
    movimento seja independente do FPS.
    """
    global Cam_pos

    frente = calculaFront()
    direita = np.cross(frente, np.array([0.0, 1.0, 0.0], dtype=np.float32))
    direita = direita / np.linalg.norm(direita)

    velocidade = Cam_speed * Tempo_entre_frames

    if glfw.get_key(Window, glfw.KEY_W) == glfw.PRESS:
        Cam_pos += frente * velocidade
    if glfw.get_key(Window, glfw.KEY_S) == glfw.PRESS:
        Cam_pos -= frente * velocidade
    if glfw.get_key(Window, glfw.KEY_A) == glfw.PRESS:
        Cam_pos -= direita * velocidade
    if glfw.get_key(Window, glfw.KEY_D) == glfw.PRESS:
        Cam_pos += direita * velocidade


def enviaUniformsCamera():
    """
    Reconstrói os vetores do plano de pixels a cada frame com base na
    posição e orientação atuais da câmera.

    O shader recebe exatamente os mesmos uniforms dos exemplos anteriores -
    a câmera FPS é transparente para o ray tracer.
    """
    frente = calculaFront()

    cima_mundo = np.array([0.0, 1.0, 0.0], dtype=np.float32)
    direita = np.cross(frente, cima_mundo)
    direita = direita / np.linalg.norm(direita)
    cima = np.cross(direita, frente)

    fov = 60.0
    aspecto = WIDTH / HEIGHT
    half_h = np.tan(np.radians(fov) / 2.0)
    half_w = half_h * aspecto

    horizontal = 2.0 * half_w * direita
    vertical = 2.0 * half_h * cima
    canto_inf_esq = Cam_pos + frente - half_w * direita - half_h * cima

    glUniform3fv(glGetUniformLocation(Shader_programm, "cam_origem"), 1, Cam_pos)
    glUniform3fv(glGetUniformLocation(Shader_programm, "cam_horizontal"), 1, horizontal)
    glUniform3fv(glGetUniformLocation(Shader_programm, "cam_vertical"), 1, vertical)
    glUniform3fv(
        glGetUniformLocation(Shader_programm, "cam_canto_inf_esq"), 1, canto_inf_esq
    )


# =============================================================================
# CENA - idêntica ao Exemplo 4
# =============================================================================


def enviaUniformsCena():
    glUniform3fv(
        glGetUniformLocation(Shader_programm, "esfera_centro"),
        1,
        np.array([0.0, 0.0, 0.0], dtype=np.float32),
    )
    glUniform1f(glGetUniformLocation(Shader_programm, "esfera_raio"), np.float32(1.0))
    glUniform3fv(
        glGetUniformLocation(Shader_programm, "esfera_cor"),
        1,
        np.array([0.9, 0.9, 0.9], dtype=np.float32),
    )
    glUniform1f(
        glGetUniformLocation(Shader_programm, "esfera_refletividade"), np.float32(0.9)
    )

    glUniform3fv(
        glGetUniformLocation(Shader_programm, "esfera2_centro"),
        1,
        np.array([-1.8, 0.0, 0.5], dtype=np.float32),
    )
    glUniform1f(glGetUniformLocation(Shader_programm, "esfera2_raio"), np.float32(0.5))
    glUniform3fv(
        glGetUniformLocation(Shader_programm, "esfera2_cor"),
        1,
        np.array([1.0, 0.4, 0.1], dtype=np.float32),
    )
    glUniform1f(
        glGetUniformLocation(Shader_programm, "esfera2_refletividade"), np.float32(0.0)
    )

    glUniform3fv(
        glGetUniformLocation(Shader_programm, "esfera3_centro"),
        1,
        np.array([1.8, 0.0, 0.5], dtype=np.float32),
    )
    glUniform1f(glGetUniformLocation(Shader_programm, "esfera3_raio"), np.float32(0.5))
    glUniform3fv(
        glGetUniformLocation(Shader_programm, "esfera3_cor"),
        1,
        np.array([0.2, 0.5, 1.0], dtype=np.float32),
    )
    glUniform1f(
        glGetUniformLocation(Shader_programm, "esfera3_refletividade"), np.float32(0.0)
    )
    
    luzes = np.array(
        [
            [3.0, 4.0, 2.0],
            [-4.0, 5.0, -1.0],
            [-40.0, 5.0, 12.0],
        ]
    );
    
    glUniform3fv(
        glGetUniformLocation(Shader_programm, "luzes"),
        len(luzes),
        np.array(luzes, dtype=np.float32)
    )


# =============================================================================
# LOOP PRINCIPAL
# =============================================================================


def inicializaRenderizacao():
    global Tempo_entre_frames

    print("\n--- Câmera FPS - Exemplo 5 ---")
    print("  Mesmo ray tracing do Exemplo 4, com câmera interativa.")
    print("  W/A/S/D - mover câmera")
    print("  Mouse   - girar câmera")
    print("  ESC     - fechar\n")
    print("  Dica: orbite a esfera espelho e observe como o reflexo")
    print("  muda de ângulo - cada pixel dispara um raio diferente.\n")

    tempo_anterior = glfw.get_time()

    while not glfw.window_should_close(Window):

        # Calcula quanto tempo passou desde o último frame.
        # Usado para que o movimento da câmera seja independente do FPS.
        tempo_atual = glfw.get_time()
        Tempo_entre_frames = tempo_atual - tempo_anterior
        tempo_anterior = tempo_atual

        trataTeclado()

        glClear(GL_COLOR_BUFFER_BIT)
        glViewport(0, 0, WIDTH, HEIGHT)

        glUseProgram(Shader_programm)
        enviaUniformsCamera()  # recalculado todo frame com a posição atual
        enviaUniformsCena()  # estático - a cena não muda

        glBindVertexArray(Vao_quad)
        glDrawArrays(GL_TRIANGLES, 0, 6)

        glfw.swap_buffers(Window)
        glfw.poll_events()

    glfw.terminate()


# =============================================================================
# MAIN
# =============================================================================


def main():
    inicializaOpenGL()
    inicializaShaders()
    inicializaQuad()
    inicializaRenderizacao()


if __name__ == "__main__":
    main()
