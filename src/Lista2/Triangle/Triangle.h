#include <glad/glad.h>
#include <glm/glm.hpp>

class Triangle {
private:
    glm::mat4 _translateMatrix;
    glm::mat4 _scaleMatrix;
    glm::mat4 _rotateMatrix;
    float moveSpeed;

    void GenerateVertices();
    void GenerateNormals();
    GLuint GenerateVAO();
    glm::mat4 GenerateModelMatrix();

public:
    GLfloat _vertices[36];
    GLfloat _normals[36];
    int VerticesCount = 12;

    glm::vec3 localV0;
    glm::vec3 localV1;
    glm::vec3 localV2;

    glm::vec3 GetWorldV0();
    glm::vec3 GetWorldV1();
    glm::vec3 GetWorldV2();

    GLuint VAO;
    glm::vec3 Position;
    glm::vec3 TriangleColor;
    glm::mat4 ModelMatrix;

    Triangle(glm::vec3 color, glm::vec3 translate, glm::vec3 scale, glm::vec3 rotate, float rotationRadians = glm::radians(0.0f));
    ~Triangle();
    
    void Move(glm::vec3 movementVector);

    void CreateTriangle();
    void Draw(GLuint shaderID);
};