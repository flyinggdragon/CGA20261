#include "Sphere.h"

#include <vector>
#include <cmath>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Sphere::Sphere(
    float radius,
    int sectors,
    int stacks
) {
    BuildSphere(radius, sectors, stacks);
}

Sphere::~Sphere() {

    glDeleteVertexArrays(1, &VAO);

    glDeleteBuffers(1, &VBO);

    glDeleteBuffers(1, &EBO);
}

void Sphere::BuildSphere(
    float radius,
    int sectors,
    int stacks
) {

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    for (int i = 0; i <= stacks; ++i) {

        float stackAngle =
            glm::pi<float>() / 2.0f -
            i * glm::pi<float>() / stacks;

        float xy = radius * cos(stackAngle);
        float z = radius * sin(stackAngle);

        for (int j = 0; j <= sectors; ++j) {

            float sectorAngle =
                j * 2.0f * glm::pi<float>() / sectors;

            float x = xy * cos(sectorAngle);
            float y = xy * sin(sectorAngle);

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
        }
    }

    for (int i = 0; i < stacks; ++i) {

        int k1 = i * (sectors + 1);
        int k2 = k1 + sectors + 1;

        for (int j = 0; j < sectors; ++j, ++k1, ++k2) {

            if (i != 0) {

                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (stacks - 1)) {

                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    indexCount = static_cast<unsigned int>(indices.size());

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(float),
        vertices.data(),
        GL_STATIC_DRAW
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        indices.size() * sizeof(unsigned int),
        indices.data(),
        GL_STATIC_DRAW
    );

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        3 * sizeof(float),
        (void*)0
    );

    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void Sphere::Draw(GLuint shaderID, glm::vec3 position) {

    glm::mat4 model = glm::mat4(1.0f);

    model = glm::translate(model, position);

    glUniformMatrix4fv(
        glGetUniformLocation(shaderID, "model"),
        1,
        GL_FALSE,
        glm::value_ptr(model)
    );

    glBindVertexArray(VAO);

    glDrawElements(
        GL_TRIANGLES,
        indexCount,
        GL_UNSIGNED_INT,
        0
    );

    glBindVertexArray(0);
}