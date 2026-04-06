import glfw
from OpenGL.GL import *
import numpy as np
import math
import glm


vertex_shader_code = """
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
"""

fragment_shader_code = """
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 viewPos;
uniform int renderMode; // 0:Full, 1:Normais, 2:Amb, 3:Diff, 4:Spec


#define NR_LIGHTS 2
uniform vec3 lightPositions[NR_LIGHTS];
uniform vec3 lightColors[NR_LIGHTS];


uniform vec3 ka_color;
uniform vec3 kd_color;
uniform vec3 ks_color;
uniform float shininess;

void main() {
    vec3 norm = normalize(Normal);
    
   
    if (renderMode == 1) {
        FragColor = vec4(norm * 0.5 + 0.5, 1.0);
        return;
    }

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 ambientTotal = vec3(0.0);
    vec3 diffuseTotal = vec3(0.0);
    vec3 specularTotal = vec3(0.0);

    
    for(int i = 0; i < NR_LIGHTS; i++) {
       
        ambientTotal += ka_color * lightColors[i];

        
        vec3 lightDir = normalize(lightPositions[i] - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        diffuseTotal += (diff * kd_color) * lightColors[i];

        
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);
        specularTotal += (spec * ks_color) * lightColors[i];
    }

    
    if (renderMode == 2) FragColor = vec4(ambientTotal, 1.0);
    else if (renderMode == 3) FragColor = vec4(diffuseTotal, 1.0);
    else if (renderMode == 4) FragColor = vec4(specularTotal, 1.0);
    else FragColor = vec4(ambientTotal + diffuseTotal + specularTotal, 1.0);
}
"""


vertices = np.array([
    -0.5, -0.5, -0.5,  0.0,  0.0, -1.0,  0.5, -0.5, -0.5,  0.0,  0.0, -1.0,
     0.5,  0.5, -0.5,  0.0,  0.0, -1.0,  0.5,  0.5, -0.5,  0.0,  0.0, -1.0,
    -0.5,  0.5, -0.5,  0.0,  0.0, -1.0, -0.5, -0.5, -0.5,  0.0,  0.0, -1.0,
    -0.5, -0.5,  0.5,  0.0,  0.0,  1.0,  0.5, -0.5,  0.5,  0.0,  0.0,  1.0,
     0.5,  0.5,  0.5,  0.0,  0.0,  1.0,  0.5,  0.5,  0.5,  0.0,  0.0,  1.0,
    -0.5,  0.5,  0.5,  0.0,  0.0,  1.0, -0.5, -0.5,  0.5,  0.0,  0.0,  1.0,
    -0.5,  0.5,  0.5, -1.0,  0.0,  0.0, -0.5,  0.5, -0.5, -1.0,  0.0,  0.0,
    -0.5, -0.5, -0.5, -1.0,  0.0,  0.0, -0.5, -0.5, -0.5, -1.0,  0.0,  0.0,
    -0.5, -0.5,  0.5, -1.0,  0.0,  0.0, -0.5,  0.5,  0.5, -1.0,  0.0,  0.0,
     0.5,  0.5,  0.5,  1.0,  0.0,  0.0,  0.5,  0.5, -0.5,  1.0,  0.0,  0.0,
     0.5, -0.5, -0.5,  1.0,  0.0,  0.0,  0.5, -0.5, -0.5,  1.0,  0.0,  0.0,
     0.5, -0.5,  0.5,  1.0,  0.0,  0.0,  0.5,  0.5,  0.5,  1.0,  0.0,  0.0,
    -0.5, -0.5, -0.5,  0.0, -1.0,  0.0,  0.5, -0.5, -0.5,  0.0, -1.0,  0.0,
     0.5, -0.5,  0.5,  0.0, -1.0,  0.0,  0.5, -0.5,  0.5,  0.0, -1.0,  0.0,
    -0.5, -0.5,  0.5,  0.0, -1.0,  0.0, -0.5, -0.5, -0.5,  0.0, -1.0,  0.0,
    -0.5,  0.5, -0.5,  0.0,  1.0,  0.0,  0.5,  0.5, -0.5,  0.0,  1.0,  0.0,
     0.5,  0.5,  0.5,  0.0,  1.0,  0.0,  0.5,  0.5,  0.5,  0.0,  1.0,  0.0,
    -0.5,  0.5,  0.5,  0.0,  1.0,  0.0, -0.5,  0.5, -0.5,  0.0,  1.0,  0.0
], dtype=np.float32)


current_mode = 0
is_metallic = False

def key_callback(window, key, scancode, action, mods):
    global current_mode, is_metallic
    if action == glfw.PRESS:
        if key >= glfw.KEY_0 and key <= glfw.KEY_4: current_mode = key - glfw.KEY_0
        if key == glfw.KEY_M: is_metallic = not is_metallic


glfw.init()
window = glfw.create_window(800, 600, "Exercicios 5-10", None, None)
glfw.make_context_current(window)
glfw.set_key_callback(window, key_callback)
glEnable(GL_DEPTH_TEST)

def compile_sh(src, t):
    s = glCreateShader(t); glShaderSource(s, src); glCompileShader(s); return s

prog = glCreateProgram()
glAttachShader(prog, compile_sh(vertex_shader_code, GL_VERTEX_SHADER))
glAttachShader(prog, compile_sh(fragment_shader_code, GL_FRAGMENT_SHADER))
glLinkProgram(prog)
glUseProgram(prog)

vao = glGenVertexArrays(1); vbo = glGenBuffers(1)
glBindVertexArray(vao); glBindBuffer(GL_ARRAY_BUFFER, vbo)
glBufferData(GL_ARRAY_BUFFER, vertices.nbytes, vertices, GL_STATIC_DRAW)
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*4, ctypes.c_void_p(0))
glEnableVertexAttribArray(0)
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*4, ctypes.c_void_p(3*4))
glEnableVertexAttribArray(1)


while not glfw.window_should_close(window):
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    t = glfw.get_time()

    proj = glm.perspective(glm.radians(45.0), 800/600, 0.1, 100.0)
    view = glm.translate(glm.mat4(1.0), glm.vec3(0, 0, -3.0))
    model = glm.rotate(glm.mat4(1.0), t * 0.5, glm.vec3(0, 1, 0))

    glUniformMatrix4fv(glGetUniformLocation(prog, "projection"), 1, GL_FALSE, glm.value_ptr(proj))
    glUniformMatrix4fv(glGetUniformLocation(prog, "view"), 1, GL_FALSE, glm.value_ptr(view))
    glUniformMatrix4fv(glGetUniformLocation(prog, "model"), 1, GL_FALSE, glm.value_ptr(model))
    glUniform3f(glGetUniformLocation(prog, "viewPos"), 0, 0, 3)
    glUniform1i(glGetUniformLocation(prog, "renderMode"), current_mode)

    
    lp1 = [math.sin(t)*2, 1, math.cos(t)*2]
    lp2 = [-2, math.sin(t*0.5)*2, 1]
    glUniform3f(glGetUniformLocation(prog, "lightPositions[0]"), *lp1)
    glUniform3f(glGetUniformLocation(prog, "lightColors[0]"), 1.0, 0.5, 0.5) 
    glUniform3f(glGetUniformLocation(prog, "lightPositions[1]"), *lp2)
    glUniform3f(glGetUniformLocation(prog, "lightColors[1]"), 0.5, 0.5, 1.0) 

   
    if is_metallic:
        
        glUniform3f(glGetUniformLocation(prog, "ka_color"), 0.1, 0.1, 0.1)
        glUniform3f(glGetUniformLocation(prog, "kd_color"), 0.1, 0.1, 0.1)
        glUniform3f(glGetUniformLocation(prog, "ks_color"), 1.0, 1.0, 1.0)
        glUniform1f(glGetUniformLocation(prog, "shininess"), 128.0)
    else:
        
        glUniform3f(glGetUniformLocation(prog, "ka_color"), 0.1, 0.0, 0.0)
        glUniform3f(glGetUniformLocation(prog, "kd_color"), 0.7, 0.1, 0.1)
        glUniform3f(glGetUniformLocation(prog, "ks_color"), 0.4, 0.4, 0.4)
        glUniform1f(glGetUniformLocation(prog, "shininess"), 32.0)

    glBindVertexArray(vao)
    glDrawArrays(GL_TRIANGLES, 0, 36)
    glfw.swap_buffers(window); glfw.poll_events()

glfw.terminate()