import glfw
from OpenGL.GL import *
import OpenGL.GL.shaders
import numpy as np
from PIL import Image


WIDTH, HEIGHT = 800, 600
Window = None
Shader_programm = None
Vao_cubo = None


exibir_profundidade = False
modo_wireframe = False
efeito_distancia = False
mouse_livre = False
FOV = 67.0


Cam_pos = np.array([0.0, 1.0, 5.0])
Cam_yaw, Cam_pitch = -90.0, 0.0
lastX, lastY = WIDTH / 2, HEIGHT / 2
primeiro_mouse = True
Tempo_entre_frames = 0.0


cubos = [
    {"pos": [0, 0, 0], "escala": [1, 1, 1], "cor": [
        1.0, 0.6, 0.2, 1.0], "tipo": "fixo"},
    {"pos": [3, 0, 0], "escala": [0.7, 0.7, 0.7], "cor": [
        0.2, 0.8, 0.4, 1.0], "tipo": "cor_varia"},
    {"pos": [0, 0, 0], "escala": [0.4, 0.4, 0.4],
        "cor": [0.2, 0.5, 1.0, 1.0], "tipo": "orbita"}
]


def redimensionaCallback(window, w, h):
    global WIDTH, HEIGHT
    WIDTH, HEIGHT = w, h
    glViewport(0, 0, w, h)


def mouse_callback(window, xpos, ypos):
    global lastX, lastY, primeiro_mouse, Cam_yaw, Cam_pitch
    if mouse_livre:
        return

    if primeiro_mouse:
        lastX, lastY = xpos, ypos
        primeiro_mouse = False

    xoffset = (xpos - lastX) * 0.1
    yoffset = (lastY - ypos) * 0.1
    lastX, lastY = xpos, ypos

    Cam_yaw += xoffset
    Cam_pitch = np.clip(Cam_pitch + yoffset, -89.0, 89.0)


def key_callback(window, key, scancode, action, mode):
    global exibir_profundidade, modo_wireframe, efeito_distancia, mouse_livre, FOV

    if action == glfw.PRESS:
        if key == glfw.KEY_P:
            exibir_profundidade = not exibir_profundidade
        if key == glfw.KEY_Z:
            modo_wireframe = not modo_wireframe
            glPolygonMode(GL_FRONT_AND_BACK,
                          GL_LINE if modo_wireframe else GL_FILL)
        if key == glfw.KEY_E:
            efeito_distancia = not efeito_distancia
        if key == glfw.KEY_M:
            mouse_livre = not mouse_livre
            glfw.set_input_mode(
                window, glfw.CURSOR, glfw.CURSOR_NORMAL if mouse_livre else glfw.CURSOR_DISABLED)
        if key == glfw.KEY_UP:
            FOV = min(120.0, FOV + 5.0)
        if key == glfw.KEY_DOWN:
            FOV = max(10.0, FOV - 5.0)
        if key == glfw.KEY_F12:
            salvar_screenshot()


def salvar_screenshot():

    glPixelStorei(GL_PACK_ALIGNMENT, 1)
    data = glReadPixels(0, 0, WIDTH, HEIGHT, GL_RGB, GL_UNSIGNED_BYTE)
    image = Image.frombytes("RGB", (WIDTH, HEIGHT), data)
    image = image.transpose(Image.FLIP_TOP_BOTTOM)
    image.save("resultado_exercicio.png")
    print("Screenshot salva como resultado_exercicio.png!")


def inicializaShaders():
    global Shader_programm
    vertex_shader = """
        #version 400
        layout(location = 0) in vec3 vertex_posicao;
        uniform mat4 transform, view, proj;
        out float dist_camera;
        void main() {
            vec4 pos_mundo = transform * vec4(vertex_posicao, 1.0);
            gl_Position = proj * view * pos_mundo;
            dist_camera = length(view * pos_mundo); // Distância para efeito (Item 6)
        }
    """
    fragment_shader = """
        #version 400
        in float dist_camera;
        out vec4 frag_colour;
        uniform vec4 corobjeto;
        uniform float tempo;
        uniform int tipoCubo; // 0: normal, 1: variavel
        uniform bool visualizaProfundidade, usarFog;

        void main() {
            vec4 corBase = corobjeto;
            if(tipoCubo == 1) corBase = vec4(abs(sin(tempo)), abs(cos(tempo)), 0.6, 1.0); // Item 2

            if(usarFog) { // Item 6: Escurecer por distância
                float fog = exp(-0.07 * dist_camera);
                corBase.rgb *= clamp(fog, 0.1, 1.0);
            }

            if(visualizaProfundidade) { // Item 3: Profundidade
                frag_colour = vec4(vec3(gl_FragCoord.z), 1.0);
            } else {
                frag_colour = corBase;
            }
        }
    """
    vs = OpenGL.GL.shaders.compileShader(vertex_shader, GL_VERTEX_SHADER)
    fs = OpenGL.GL.shaders.compileShader(fragment_shader, GL_FRAGMENT_SHADER)
    Shader_programm = OpenGL.GL.shaders.compileProgram(vs, fs)


def especificaMatrizProjecao():

    znear, zfar, fov_rad = 0.1, 100.0, np.radians(FOV)
    aspecto = WIDTH / HEIGHT
    b = 1 / np.tan(fov_rad / 2)
    a = b / aspecto
    c = (zfar + znear) / (znear - zfar)
    d = (2 * znear * zfar) / (znear - zfar)
    proj = np.array([[a, 0, 0, 0], [0, b, 0, 0], [0, 0, c, d],
                    [0, 0, -1, 0]], dtype=np.float32)
    glUniformMatrix4fv(glGetUniformLocation(
        Shader_programm, "proj"), 1, GL_TRUE, proj)


def inicializaCubo():
    global Vao_cubo
    p = [0.5, 0.5, 0.5, 0.5, -0.5, 0.5, -0.5, -0.5, 0.5, 0.5, 0.5, 0.5, -0.5, -0.5, 0.5, -0.5, 0.5, 0.5,
         0.5, 0.5, -0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -
         0.5, 0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5,
         -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, -0.5, -
         0.5, -0.5, -0.5, -0.5, 0.5, -0.5, -0.5, 0.5, 0.5,
         0.5, -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, -
         0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5, 0.5, 0.5,
         -0.5, -0.5, 0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, -
         0.5, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5,
         -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, -0.5, -0.5, 0.5, 0.5]
    Vao_cubo = glGenVertexArrays(1)
    glBindVertexArray(Vao_cubo)
    vbo = glGenBuffers(1)
    glBindBuffer(GL_ARRAY_BUFFER, vbo)
    glBufferData(GL_ARRAY_BUFFER, np.array(
        p, dtype=np.float32), GL_STATIC_DRAW)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, None)
    glEnableVertexAttribArray(0)


def transformacaoGenerica(Tx, Ty, Tz, Sx, Sy, Sz, Ry):

    ry = np.radians(Ry)
    cosy, siny = np.cos(ry), np.sin(ry)
    m = np.array([
        [Sx*cosy, 0, Sx*siny, Tx],
        [0, Sy, 0, Ty],
        [-Sz*siny, 0, Sz*cosy, Tz],
        [0, 0, 0, 1]
    ], dtype=np.float32)
    glUniformMatrix4fv(glGetUniformLocation(
        Shader_programm, "transform"), 1, GL_TRUE, m)


def especificaMatrizVisualizacao():
    f = np.array([np.cos(np.radians(Cam_yaw))*np.cos(np.radians(Cam_pitch)), np.sin(
        np.radians(Cam_pitch)), np.sin(np.radians(Cam_yaw))*np.cos(np.radians(Cam_pitch))])
    f /= np.linalg.norm(f)
    r = np.cross(f, [0, 1, 0])
    r /= np.linalg.norm(r)
    u = np.cross(r, f)
    v = np.identity(4)
    v[0, :3], v[1, :3], v[2, :3] = r, u, -f
    v[0, 3], v[1, 3], v[2, 3] = - \
        np.dot(r, Cam_pos), -np.dot(u, Cam_pos), np.dot(f, Cam_pos)
    glUniformMatrix4fv(glGetUniformLocation(
        Shader_programm, "view"), 1, GL_TRUE, v.astype(np.float32))


def trataTeclado():
    global Cam_pos
    v = 5.0 * Tempo_entre_frames
    f = np.array([np.cos(np.radians(Cam_yaw)), 0, np.sin(np.radians(Cam_yaw))])
    r = np.cross(f, [0, 1, 0])
    if glfw.get_key(Window, glfw.KEY_W) == glfw.PRESS:
        Cam_pos += f * v
    if glfw.get_key(Window, glfw.KEY_S) == glfw.PRESS:
        Cam_pos -= f * v
    if glfw.get_key(Window, glfw.KEY_A) == glfw.PRESS:
        Cam_pos -= r * v
    if glfw.get_key(Window, glfw.KEY_D) == glfw.PRESS:
        Cam_pos += r * v


def main():
    global Window, Tempo_entre_frames
    glfw.init()
    Window = glfw.create_window(
        WIDTH, HEIGHT, "Exercício Cubos - Unisinos", None, None)
    glfw.make_context_current(Window)
    glfw.set_window_size_callback(Window, redimensionaCallback)
    glfw.set_cursor_pos_callback(Window, mouse_callback)
    glfw.set_key_callback(Window, key_callback)
    glfw.set_input_mode(Window, glfw.CURSOR, glfw.CURSOR_DISABLED)

    inicializaCubo()
    inicializaShaders()
    glEnable(GL_DEPTH_TEST)
    tempo_anterior = glfw.get_time()

    while not glfw.window_should_close(Window):
        t_atual = glfw.get_time()
        Tempo_entre_frames = t_atual - tempo_anterior
        tempo_anterior = t_atual

        glClearColor(0.1, 0.1, 0.15, 1.0)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        glUseProgram(Shader_programm)

        especificaMatrizVisualizacao()
        especificaMatrizProjecao()

        glUniform1f(glGetUniformLocation(Shader_programm, "tempo"), t_atual)
        glUniform1i(glGetUniformLocation(Shader_programm,
                    "visualizaProfundidade"), exibir_profundidade)
        glUniform1i(glGetUniformLocation(
            Shader_programm, "usarFog"), efeito_distancia)

        glBindVertexArray(Vao_cubo)

        for cubo in cubos:
            tx, ty, tz = cubo["pos"]

            if cubo["tipo"] == "orbita":
                tx = 2.5 * np.cos(t_atual * 1.5)
                tz = 2.5 * np.sin(t_atual * 1.5)

            tipo = 1 if cubo["tipo"] == "cor_varia" else 0
            glUniform1i(glGetUniformLocation(
                Shader_programm, "tipoCubo"), tipo)

            loc_cor = glGetUniformLocation(Shader_programm, "corobjeto")
            glUniform4fv(loc_cor, 1, np.array(cubo["cor"], dtype=np.float32))

            transformacaoGenerica(tx, ty, tz, *cubo["escala"], t_atual * 30)
            glDrawArrays(GL_TRIANGLES, 0, 36)

        glfw.swap_buffers(Window)
        glfw.poll_events()
        trataTeclado()
    glfw.terminate()


if __name__ == "__main__":
    main()
