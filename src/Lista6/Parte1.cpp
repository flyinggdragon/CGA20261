/*
Implemente uma aplicação em OpenGL para simular e visualizar a intersecção entre um raio e um triângulo no espaço 3D. A aplicação deve cumprir os seguintes requisitos:

Renderização Base: Renderize um triângulo e um segmento de reta que representará o raio.
Manipulação Paramétrica: Implemente controles interativos (via teclado ou mouse) que permitam ao usuário alterar em tempo real:
As coordenadas (X, Y, Z) dos três vértices que compõem o triângulo.
A posição de origem, o vetor de direção e o comprimento do raio.
Detecção Contínua: Calcule matematicamente a intersecção entre o raio e o triângulo a cada frame.
Feedback Visual: Caso a intersecção seja confirmada, calcule as coordenadas espaciais do ponto de impacto e renderize uma pequena esfera exatamente nesse local. Se o raio não atingir o triângulo, a esfera não deve ser desenhada.
*/

#include <iostream>
#include <string>
#include <assert.h>
#include <cmath>
#include <algorithm>
#include <iterator>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Triangle/Triangle.h"
#include "Camera/Camera.h"
#include "Ray/Ray.h"
#include "Sphere/Sphere.h"

using namespace std;

Camera* camera = nullptr;
Triangle* triangle = nullptr;
Ray* ray = nullptr;
Sphere* sphere = nullptr;

// Protótipos de função.
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
int setupShader();

void MouseCallback(GLFWwindow* window, double xpos, double ypos) {
	camera->ProcessMouseMovement(xpos, ypos);
}

bool RayTriangleIntersect(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, float& t);

const GLuint WIDTH = 800, HEIGHT = 800;

const GLchar *vertexShaderSource = R"(
    #version 400

    layout (location = 0) in vec3 position;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main()
    {
        gl_Position =
            projection *
            view *
            model *
            vec4(position, 1.0);

        gl_PointSize = 15.0;
    }
)";

const GLchar *fragmentShaderSource = R"(
    #version 400

    uniform vec4 inputColor;

    out vec4 color;

    void main()
    {
        color = inputColor;
    }
)";


int main() {
	glfwInit();

	// Ativa a suavização de serrilhado (MSAA) com 8 amostras por pixel
	glfwWindowHint(GLFW_SAMPLES, 8);

	// Criação da janela GLFW
	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Parte 1", nullptr, nullptr);
	if (!window) {
		std::cerr << "Falha ao criar a janela GLFW" << std::endl;
		glfwTerminate();
		return -1;
	}
    
	glfwMakeContextCurrent(window);

	// Fazendo o registro da função de callback para a janela GLFW
	glfwSetKeyCallback(window, key_callback);

	// GLAD: carrega todos os ponteiros d funções da OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Falha ao inicializar GLAD" << std::endl;
		return -1;
	}

	// Obtendo as informações de versão
	const GLubyte *renderer = glGetString(GL_RENDERER);
	const GLubyte *version = glGetString(GL_VERSION);
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	// Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	// Compilando e buildando o programa de shader
	GLuint shaderID = setupShader();

	ray = new Ray(
        glm::vec3(0,0,-5),
        glm::vec3(0,0,1),
        50.0f
    );

    triangle = new Triangle(
        glm::vec3(1.0f, 0.5f, 0.0f),
        glm::vec3(0.0f),
        glm::vec3(1.0f),
        glm::vec3(0.0f,1.0f,0.0f)
    );

    sphere = new Sphere(0.1f);

    camera = new Camera(900, 700, shaderID, 0.05f, 0.2f);
	glfwSetCursorPosCallback(window, MouseCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	GLint colorLoc = glGetUniformLocation(shaderID, "inputColor");

	double prev_s = glfwGetTime();	// Define o "tempo anterior" inicial.
	double title_countdown_s = 0.1; // Intervalo para atualizar o título da janela com o FPS.
    
    glEnable(GL_DEPTH_TEST);

    bool hit = false;
    glm::vec3 hitPoint;

	// Loop da aplicação - "game loop"
	while (!glfwWindowShouldClose(window)) {
		// Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
		glfwPollEvents();
        glEnable(GL_DEPTH_TEST);

		// Limpa o buffer de cor
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // cor de fundo
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glLineWidth(10);

        // Processa input da câmera (atualiza view matrix e posição)
		camera->ProcessInput(window);

        glm::mat4 view = camera->view;
		glm::vec3 camPos = camera->position;

        glUseProgram(shaderID);

        glUniformMatrix4fv(glGetUniformLocation(shaderID, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shaderID, "projection"), 1, GL_FALSE, glm::value_ptr(camera->proj));

        triangle->Draw(shaderID);
        ray->Draw(shaderID);

        glm::vec3 v0 = triangle->GetWorldV0();
        glm::vec3 v1 = triangle->GetWorldV1();
        glm::vec3 v2 = triangle->GetWorldV2();

        glm::vec3 origin = ray->origin;
        glm::vec3 dir = ray->direction;

        float t;

        hit = RayTriangleIntersect(origin, dir, v0, v1, v2, t);

        if (hit) {
            if (hit) {
                hitPoint = origin + dir * t;

                glUniform4f(colorLoc, 1.0f, 0.0f, 0.0f, 1.0f);

                sphere->Draw(shaderID, hitPoint);
            }
        }
        
        // Troca os buffers da tela
        glfwSwapBuffers(window);
	}

    glDeleteVertexArrays(1, &triangle->VAO);
    glDeleteVertexArrays(1, &ray->VAO);
    
	// Finaliza a execução da GLFW, limpando os recursos alocados por ela
	glfwTerminate();
	return 0;
}

// Função de callback de teclado - só pode ter uma instância (deve ser estática se
// estiver dentro de uma classe) - É chamada sempre que uma tecla for pressionada
// ou solta via GLFW
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_T && (action == GLFW_PRESS || action == GLFW_REPEAT))
		triangle->Move(glm::vec3(1.0f, 0.0f, 0.0f));
	if (key == GLFW_KEY_G && (action == GLFW_PRESS || action == GLFW_REPEAT))
		triangle->Move(glm::vec3(-1.0f, 0.0f, 0.0f));
	if (key == GLFW_KEY_H && (action == GLFW_PRESS || action == GLFW_REPEAT))
		triangle->Move(glm::vec3(0.0f, 0.0f, 1.0f));
	if (key == GLFW_KEY_F && (action == GLFW_PRESS || action == GLFW_REPEAT))
		triangle->Move(glm::vec3(0.0f, 0.0f, -1.0f));
	if (key == GLFW_KEY_V && (action == GLFW_PRESS || action == GLFW_REPEAT))
		triangle->Move(glm::vec3(0.0f, -1.0f, 0.0f));
	if (key == GLFW_KEY_B && (action == GLFW_PRESS || action == GLFW_REPEAT))
		triangle->Move(glm::vec3(0.0f, 1.0f, -0.0f));

	if (key == GLFW_KEY_J && (action == GLFW_PRESS || action == GLFW_REPEAT))
		ray->Move(glm::vec3(1.0f, 0.0f, 0.0f));
	if (key == GLFW_KEY_L && (action == GLFW_PRESS || action == GLFW_REPEAT))
		ray->Move(glm::vec3(-1.0f, 0.0f, 0.0f));
	if (key == GLFW_KEY_K && (action == GLFW_PRESS || action == GLFW_REPEAT))
		ray->Move(glm::vec3(0.0f, -1.0f, 0.0f));
	if (key == GLFW_KEY_I && (action == GLFW_PRESS || action == GLFW_REPEAT))
		ray->Move(glm::vec3(0.0f, 1.0f, -0.0f));

    if (key == GLFW_KEY_8 && (action == GLFW_PRESS || action == GLFW_REPEAT))
        ray->Resize(-1.0f);
	if (key == GLFW_KEY_9 && (action == GLFW_PRESS || action == GLFW_REPEAT))
		ray->Resize(1.0f);
}

// Esta função está bastante hardcoded - objetivo é compilar e "buildar" um programa de
//  shader simples e único neste exemplo de código
//  O código fonte do vertex e fragment shader está nos arrays vertexShaderSource e
//  fragmentShader source no iniçio deste arquivo
//  A função retorna o identificador do programa de shader

int setupShader() {
	// Vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	// Checando erros de compilação (exibição via log no terminal)
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

	if (!success) {
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
				  << infoLog << std::endl;
	}

	// Fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	// Checando erros de compilação (exibição via log no terminal)
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
				  << infoLog << std::endl;
	}

	// Linkando os shaders e criando o identificador do programa de shader
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	// Checando por erros de linkagem
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
				  << infoLog << std::endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

bool RayTriangleIntersect(
    const glm::vec3& rayOrigin,
    const glm::vec3& rayDir,
    const glm::vec3& v0,
    const glm::vec3& v1,
    const glm::vec3& v2,
    float& outT
) {
    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;

    glm::vec3 h = glm::cross(rayDir, edge2);
    float a = glm::dot(edge1, h);

    if (fabs(a) < 1e-6f) return false;

    float f = 1.0f / a;
    glm::vec3 s = rayOrigin - v0;

    float u = f * glm::dot(s, h);
    if (u < 0.0f || u > 1.0f) return false;

    glm::vec3 q = glm::cross(s, edge1);
    float v = f * glm::dot(rayDir, q);
    if (v < 0.0f || (u + v) > 1.0f) return false;

    float t = f * glm::dot(edge2, q);

    if (t > 1e-6f) {
        outT = t;
        return true;
    }

    return false;
}