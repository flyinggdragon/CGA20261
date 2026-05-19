#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Ray {
private:
    GLfloat vertices[6];

    GLuint VBO;

    glm::mat4 _translateMatrix;
    glm::mat4 _scaleMatrix;
    glm::mat4 _rotateMatrix;
    float _rayLength;

    void GenerateVertices();
    GLuint GenerateVAO();
    glm::mat4 GenerateModelMatrix();

public:
    glm::vec3 origin;
    glm::vec3 direction;

    GLuint VAO;

    glm::vec3 RayColor;
    glm::mat4 ModelMatrix;

    Ray(glm::vec3 origin, glm::vec3 direction, float rayLength, glm::vec3 color = glm::vec3(1.0f, 0.0f, 0.0f));

    ~Ray();

    void Resize(float newLength);
    void Move(glm::vec3 movementVector);
    
    void CreateRay();
    void Draw(GLuint shaderID);
};