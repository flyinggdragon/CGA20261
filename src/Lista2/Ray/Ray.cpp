#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <iterator>

#include "Ray.h"

void Ray::GenerateVertices() {
    glm::vec3 end = origin + direction * _rayLength;

    GLfloat verts[] =
    {
        origin.x, origin.y, origin.z,
        end.x,    end.y,    end.z
    };

    std::copy(std::begin(verts), std::end(verts), vertices);
}

GLuint Ray::GenerateVAO() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);

    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}

glm::mat4 Ray::GenerateModelMatrix() {
    return _translateMatrix * _rotateMatrix * _scaleMatrix;
}

Ray::Ray(glm::vec3 origin, glm::vec3 direction, float rayLength, glm::vec3 color) {
    this->origin = origin;
    this->direction = glm::normalize(direction);

    this->_rayLength = rayLength;

    this->RayColor = color;

    _translateMatrix = glm::translate(glm::mat4(1.0f), origin );

    _scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));

    _rotateMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    CreateRay();
}

Ray::~Ray() {
    
}

void Ray::Move(glm::vec3 movementVector) {
    origin += movementVector * 0.1f;

    _translateMatrix = glm::translate(glm::mat4(1.0f), origin);

    ModelMatrix = GenerateModelMatrix();
}

void Ray::Resize(float increment) {
    _rayLength += increment * 5;

    if (_rayLength < 0.0f) _rayLength = 0.0f;

    GenerateVertices();

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Ray::CreateRay() {
    GenerateVertices();

    VAO = GenerateVAO();

    ModelMatrix = GenerateModelMatrix();
}

void Ray::Draw(GLuint shaderID) {
    glBindVertexArray(VAO);

    glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, glm::value_ptr(ModelMatrix));
    glUniform4f(glGetUniformLocation(shaderID, "inputColor"), RayColor.r, RayColor.g, RayColor.b, 1.0f);

    glLineWidth(2.0f);
    glDrawArrays(GL_LINES, 0, 2);

    glBindVertexArray(0);
}