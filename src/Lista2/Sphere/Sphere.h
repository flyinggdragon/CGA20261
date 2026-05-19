#include <vector>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Sphere {
public:

    GLuint VAO;
    GLuint VBO;
    GLuint EBO;

    unsigned int indexCount;

    Sphere(
        float radius = 0.1f,
        int sectors = 20,
        int stacks = 20
    );

    ~Sphere();

    void Draw(
        GLuint shaderID,
        glm::vec3 position
    );

private:

    void BuildSphere(
        float radius,
        int sectors,
        int stacks
    );
};