// Exercício 8 - Marco

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

#include "Cube/Cube.h"
#include "Camera/Camera.h"

using namespace std;

Camera* camera = nullptr;
bool cursorEnabled = false;

// Protótipos de função.
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
int setupShader();

void MouseCallback(GLFWwindow* window, double xpos, double ypos) {
	camera->ProcessMouseMovement(xpos, ypos);
}

const GLuint WIDTH = 800, HEIGHT = 800;

const GLchar *vertexShaderSource = R"(
    #version 400
    layout (location = 0) in vec3 position;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main() {
        gl_Position = projection * view * model * vec4(position, 1.0);
    }
)";

const GLchar *fragmentShaderSource = R"(
    #version 400

    uniform vec4 inputColor;
    out vec4 color;

    void main() {
        color = inputColor;
    }
)";


int main() {
	glfwInit();

	// Ativa a suavização de serrilhado (MSAA) com 8 amostras por pixel
	glfwWindowHint(GLFW_SAMPLES, 8);

	// Criação da janela GLFW
	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Exercício 6", nullptr, nullptr);
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

	Cube* cubes[] = {
        new Cube(
            glm::vec3(0.0f, 1.0f, 0.5f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(1.0f),
            glm::vec3(0.0f, 0.0f, 1.0f)
        ),

        new Cube(
            glm::vec3(0.5f, 0.0f, 1.0f),
            glm::vec3(5.0f, 0.0f, 5.0f),
            glm::vec3(0.7f),
            glm::vec3(0.0f, 0.0f, 1.0f)
        ),

        new Cube(
            glm::vec3(1.0f, 0.0f, 0.5f),
            glm::vec3(-3.0f, 0.0f, -3.0f),
            glm::vec3(1.2f),
            glm::vec3(0.0f, 0.0f, 1.0f)
        ),
    };

    camera = new Camera(900, 700, shaderID, 0.05f, 0.2f);
    glfwSetCursorPosCallback(window, MouseCallback);

    glfwSetInputMode(window, GLFW_CURSOR, cursorEnabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);

	// Enviando a cor desejada (vec4) para o fragment shader
	// Utilizamos a variáveis do tipo uniform em GLSL para armazenar esse tipo de info
	// que não está nos buffers
	GLint colorLoc = glGetUniformLocation(shaderID, "inputColor");

	glUseProgram(shaderID); // Reseta o estado do shader para evitar problemas futuros

	double prev_s = glfwGetTime();	// Define o "tempo anterior" inicial.
	double title_countdown_s = 0.1; // Intervalo para atualizar o título da janela com o FPS.

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

        glUniformMatrix4fv(glGetUniformLocation(shaderID, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shaderID, "projection"), 1, GL_FALSE, glm::value_ptr(camera->proj));

        for (Cube* c : cubes) {
            glBindVertexArray(c->VAO);

            // Envia a matriz de transformação pro shader.
            glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, glm::value_ptr(c->ModelMatrix));

            // Envia a cor pro shader.
            glUniform4f(colorLoc, c->CubeColor.x, c->CubeColor.y, c->CubeColor.z, 1.0f); // enviando cor para variável uniform inputColor

            glDrawArrays(GL_TRIANGLES, 0, c->VerticesCount);
        }

        // Troca os buffers da tela
        glfwSwapBuffers(window);
	}

    for (Cube* c : cubes) {
        glDeleteVertexArrays(1, &c->VAO);
    }
	
	// Finaliza a execução da GLFW, limpando os recursos alocados por ela
	glfwTerminate();
	return 0;
}

// Função de callback de teclado - só pode ter uma instância (deve ser estática se
// estiver dentro de uma classe) - É chamada sempre que uma tecla for pressionada
// ou solta via GLFW
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    
    if (key == GLFW_KEY_I && action == GLFW_PRESS) {
        cursorEnabled = !cursorEnabled;

        glfwSetInputMode(window, GLFW_CURSOR, cursorEnabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);

        camera->firstMouse = true;
    }
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