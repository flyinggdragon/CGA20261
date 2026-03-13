#include <glad/glad.h>
#include <glm/glm.hpp>

class Cube {
private:
    glm::mat4 _translateMatrix;
    glm::mat4 _scaleMatrix;
    glm::mat4 _rotateMatrix;

    void GenerateVertices();
    GLuint GenerateVAO();
    glm::mat4 GenerateModelMatrix();

public:
    GLfloat _vertices[108];
    int VerticesCount = 36;

    GLuint VAO;
    glm::vec3 CubeColor;
    glm::mat4 ModelMatrix;

    Cube(glm::vec3 color, glm::mat4 translate, glm::mat4 scale, glm::mat4 rotate);

    void CreateCube();
};