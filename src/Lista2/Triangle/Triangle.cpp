#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <iterator>

#include "Triangle.h"

using namespace std;

void Triangle::GenerateVertices() {
    GLfloat _verts[] = {
        // Base
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.0f, -0.5f,  0.5f,

        // Face frontal
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.0f,  0.5f,  0.0f,

        // Face direita
         0.5f, -0.5f, -0.5f,
         0.0f, -0.5f,  0.5f,
         0.0f,  0.5f,  0.0f,

        // Face esquerda
         0.0f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,
         0.0f,  0.5f,  0.0f
    };

    copy(begin(_verts), end(_verts), _vertices);
}

void Triangle::GenerateNormals() {
    GLfloat normals[] = {
        // Base
        0.0f, -1.0f,  0.0f,
        0.0f, -1.0f,  0.0f,
        0.0f, -1.0f,  0.0f,

        // Face frontal
        0.0f, 0.7f, -0.7f,
        0.0f, 0.7f, -0.7f,
        0.0f, 0.7f, -0.7f,

        // Face direita
        0.7f, 0.7f, 0.0f,
        0.7f, 0.7f, 0.0f,
        0.7f, 0.7f, 0.0f,

        // Face esquerda
       -0.7f, 0.7f, 0.0f,
       -0.7f, 0.7f, 0.0f,
       -0.7f, 0.7f, 0.0f
    };

    std::copy(std::begin(normals), std::end(normals), _normals);
}

GLuint Triangle::GenerateVAO() {
    GLuint n_VBO, VBO, VAO;

    // Geração do identificador do VAO (Vertex Array Object)
    glGenVertexArrays(1, &VAO);

    // Vincula (bind) o VAO primeiro, e em seguida  conecta e seta o(s) buffer(s) de vértices
    // e os ponteiros para os atributos
    glBindVertexArray(VAO);

    // Geração do identificador do VBO
    glGenBuffers(1, &VBO);
    // Faz a conexão (vincula) do buffer como um buffer de array
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // Envia os dados do array de floats para o buffer da OpenGl
    glBufferData(GL_ARRAY_BUFFER, sizeof(_vertices), _vertices, GL_STATIC_DRAW);

    // Para cada atributo do vertice, criamos um "AttribPointer" (ponteiro para o atributo), indicando:
    //  Localização no shader * (a localização dos atributos devem ser correspondentes no layout especificado no vertex shader)
    //  Numero de valores que o atributo tem (por ex, 3 coordenadas xyz)
    //  Tipo do dado
    //  Se está normalizado (entre zero e um)
    //  Tamanho em bytes
    //  Deslocamento a partir do byte zero (stride)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &n_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, n_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(_normals), _normals, GL_STATIC_DRAW);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    // Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)
    glBindVertexArray(0);

    return VAO;
}

glm::mat4 Triangle::GenerateModelMatrix() {
    return this->_translateMatrix * this->_rotateMatrix * this->_scaleMatrix;
}

Triangle::Triangle(glm::vec3 color, glm::vec3 translate, glm::vec3 scale, glm::vec3 rotate, float rotationRadians) {
    this->TriangleColor = color;
    this->Position = translate;
    this->_translateMatrix = glm::translate(glm::mat4(1.0f), translate);
    this->_scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
    this->_rotateMatrix = glm::rotate(glm::mat4(1.0f), rotationRadians, rotate);

    this->moveSpeed = 0.1f;

    localV0 = glm::vec3(-0.5f, -0.5f, -0.5f);
    localV1 = glm::vec3( 0.5f, -0.5f, -0.5f);
    localV2 = glm::vec3( 0.0f,  0.5f,  0.0f);

    CreateTriangle();
}

Triangle::~Triangle() {
    
}

glm::vec3 Triangle::GetWorldV0() {
    return glm::vec3(ModelMatrix * glm::vec4(localV0, 1.0f));
}

glm::vec3 Triangle::GetWorldV1() {
    return glm::vec3(ModelMatrix * glm::vec4(localV1, 1.0f));
}

glm::vec3 Triangle::GetWorldV2() {
    return glm::vec3(ModelMatrix * glm::vec4(localV2, 1.0f));
}

void Triangle::Move(glm::vec3 movementVector) {
    Position += movementVector;

    _translateMatrix = glm::translate(glm::mat4(1.0f),Position);

    ModelMatrix = GenerateModelMatrix();
}

void Triangle::CreateTriangle() {
    GenerateVertices();
    GenerateNormals();

    VAO = GenerateVAO();

    ModelMatrix = GenerateModelMatrix();
}

void Triangle::Draw(GLuint shaderID) {
    glBindVertexArray(VAO);

    // Envia a matriz de transformação e vetor de cor pro shader.
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, glm::value_ptr(ModelMatrix));
    glUniform4f(glGetUniformLocation(shaderID, "inputColor"), TriangleColor.r, TriangleColor.g, TriangleColor.b, 1.0f);

    glDrawArrays(GL_TRIANGLES, 0, VerticesCount);
}