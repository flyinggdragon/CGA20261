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
    glm::vec3 Position;
    glm::vec3 CubeColor;
    glm::mat4 ModelMatrix;

    Cube(glm::vec3 color, glm::vec3 translate, glm::vec3 scale, glm::vec3 rotate, float rotationRadians = glm::radians(0.0f));
    ~Cube();
    
    void CreateCube();
};