#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Sphere/Sphere.h"

using namespace std;
void key_callback(
    GLFWwindow *window,
    int key,
    int scancode,
    int action,
    int mode
);

GLuint setupShader(
    const GLchar* fragmentShaderSource
);

const GLuint WIDTH = 800;
const GLuint HEIGHT = 800;

const GLchar *vertexShaderSource = R"(
    #version 400

    layout (location = 0) in vec3 position;
    layout (location = 1) in vec3 normal;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    out vec3 fragPos;
    out vec3 fragNormal;

    void main()
    {
        vec4 worldPos =
            model * vec4(position, 1.0);

        fragPos = worldPos.xyz;

        fragNormal =
            mat3(transpose(inverse(model))) * normal;

        gl_Position =
            projection * view * worldPos;
    }
)";

const GLchar *phongShader = R"(
    #version 400

    in vec3 fragPos;
    in vec3 fragNormal;

    uniform vec3 materialColor;

    uniform float Ka;
    uniform float Kd;
    uniform float Ks;

    uniform float shininess;

    uniform vec3 lightPosition;
    uniform vec3 viewPos;

    uniform float a;
    uniform float b;
    uniform float c;

    out vec4 color;

    void main()
    {
        vec3 normal = normalize(fragNormal);
        vec3 lightDirection = normalize(lightPosition - fragPos);

        // componente difuso do Phong/Blinn-Phong;
        float diff = max(dot(normal, lightDirection), 0.0);
        vec3 diffuse = Kd * diff * materialColor;

        float d = length(lightPosition - fragPos);
        float attenuation = 1.0 / (a + b * d + c * d * d);

        // componente especular do Phong/Blinn-Phong;
        vec3 viewDir = normalize(viewPos - fragPos);
        vec3 reflectDir = reflect(-lightDirection, normal);

        float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        vec3 specular = Ks * spec * vec3(1.0);

        diffuse *= attenuation;
        specular *= attenuation;

        vec3 ambient = Ka * materialColor;

        vec3 finalColor = ambient + diffuse + specular;

        color = vec4(finalColor, 1.0);
    }
)";

const GLchar *pbrShader = R"(
    #version 400

    in vec3 fragPos;
    in vec3 fragNormal;

    uniform vec3 materialColor;

    uniform float metallic;
    uniform float roughness;
    uniform float ao;

    uniform vec3 lightPosition;
    uniform vec3 viewPos;

    out vec4 color;

    const float PI = 3.14159265359;

    vec3 fresnelSchlick(float cosTheta, vec3 F0)
    {
        // cálculo de F0;
        return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
    }

    float DistributionGGX(vec3 N, vec3 H, float roughness)
    {
        roughness = max(roughness, 0.05);

        float a = roughness * roughness;
        float a2 = a * a;

        float NdotH = max(dot(N, H), 0.0);
        float NdotH2 = NdotH * NdotH;

        float numerator = a2;
        float denominator = (NdotH2 * (a2 - 1.0) + 1.0);

        denominator = PI * denominator * denominator;

        return numerator / denominator;
    }

    float GeometrySchlickGGX(float NdotV, float roughness)
    {
        float r = roughness + 1.0;
        float k = (r * r) / 8.0;

        float numerator = NdotV;
        float denominator = NdotV * (1.0 - k) + k;

        return numerator / denominator;
    }

    float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
    {
        float NdotV = max(dot(N, V), 0.0);
        float NdotL = max(dot(N, L), 0.0);

        float ggx2 = GeometrySchlickGGX(NdotV, roughness);
        float ggx1 = GeometrySchlickGGX(NdotL, roughness);

        return ggx1 * ggx2;
    }

    void main()
    {
        vec3 albedo = materialColor;

        vec3 N = normalize(fragNormal);
        vec3 V = normalize(viewPos - fragPos);
        vec3 L = normalize(lightPosition - fragPos);

        vec3 H = V + L;
        if(length(H) > 0.0001) H = normalize(H); else H = N;

        vec3 radiance = vec3(2.0);

        vec3 F0 = vec3(0.04);
        F0 = mix(F0, albedo, metallic);

        // cálculo dos termos D, F e G;
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator = NDF * G * F;

        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;

        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;

        // conservação de energia com kD e kS;
        kD *= (1.0 - metallic);

        float NdotL = max(dot(N, L), 0.0);

        vec3 Lo = (kD * albedo / PI + specular) * radiance * NdotL;

        vec3 ambient = vec3(0.03) * albedo * ao;

        // composição final da cor do fragmento.
        vec3 finalColor = ambient + Lo;

        finalColor = finalColor / (finalColor + vec3(1.0));
        finalColor = pow(finalColor, vec3(1.0 / 2.2));

        color = vec4(finalColor, 1.0);
    }
)";

struct Material {
    glm::vec3 color;
    float Ka;
    float Kd;
    float Ks;
    float shininess;
    float roughness;
    float metallic;

    Material(glm::vec3 matColor, float matKa, float matKd, float matKs, float matShininess, float matRoughness, float matMetallic) : color(matColor), Ka(matKa), Kd(matKd), Ks(matKs), shininess(matShininess), roughness(matRoughness), metallic(matMetallic) {}
};

// vermelho fosco/plástico
Material red(
    glm::vec3(1.0f, 0.1f, 0.1f), // cor
    0.2f,   // ka
    0.8f,   // kd 
    0.5f,   // ks
    32.0f,  // shininess
    0.7f,   // roughness
    0.0f    // metallic
);

// azul brilhante/plástico
Material blue(
    glm::vec3(0.1f, 0.3f, 1.0f), // cor
    0.2f,   // ka
    0.8f,   // kd
    0.6f,   // Ks
    64.0f,  // shininess
    0.4f,   // roughness
    0.0f    // metallic
);

// dourado metálico
Material gold(
    glm::vec3(1.0f, 0.84f, 0.0f), // cor
    0.25f,  // ka
    0.7f,   // kd
    1.0f,   // ks
    128.0f, // shininess
    0.2f,   // roughness
    1.0f    // metallic
);

Material materials[] = {red, blue, gold};

int matIndex = 0;

int main() {
    glfwInit();

    glfwWindowHint(GLFW_SAMPLES, 8);

    GLFWwindow *window =
        glfwCreateWindow(
            WIDTH,
            HEIGHT,
            "Phong vs PBR",
            nullptr,
            nullptr
        );

    if (!window) {
        cout << "Failed to create GLFW window" << endl;

        glfwTerminate();

        return -1;
    }

    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD" << endl;

        return -1;
    }

    glViewport(0, 0, WIDTH, HEIGHT);

    glEnable(GL_DEPTH_TEST);

    GLuint phongProgram =  setupShader(phongShader);

    GLuint pbrProgram = setupShader(pbrShader);

    Sphere* spheres[] = {
        new Sphere(0.3f),
        new Sphere(0.3f)
    };

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        Material currentMaterial = materials[matIndex];

        glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);

        glUseProgram(phongProgram);

        glUniformMatrix4fv(glGetUniformLocation(phongProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

        glUniformMatrix4fv(glGetUniformLocation(phongProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3f(glGetUniformLocation(phongProgram, "lightPosition"), 2.0f, 2.0f, 2.0f);

        glUniform3f(glGetUniformLocation(phongProgram, "viewPos"), 0.0f, 0.0f, 3.0f);
        glUniform1f(glGetUniformLocation(phongProgram, "a"), 1.0f);
        glUniform1f(glGetUniformLocation(phongProgram, "b"), 0.09f);
        glUniform1f(glGetUniformLocation(phongProgram, "c"), 0.032f);

        glUniform3fv(glGetUniformLocation(phongProgram, "materialColor"), 1, glm::value_ptr(currentMaterial.color));
        glUniform1f(glGetUniformLocation(phongProgram, "Ka"), currentMaterial.Ka);
        glUniform1f(glGetUniformLocation(phongProgram, "Kd"), currentMaterial.Kd);
        glUniform1f(glGetUniformLocation(phongProgram, "Ks"), currentMaterial.Ks);
        glUniform1f(glGetUniformLocation(phongProgram, "shininess"), currentMaterial.shininess);

        glBindVertexArray(spheres[0]->VAO);

        spheres[0]->Draw(phongProgram, glm::vec3(-0.7f, 0.0f, 0.0f));


        glUseProgram(pbrProgram);

        GLint viewLocPBR = glGetUniformLocation(pbrProgram, "view");
        GLint projLocPBR = glGetUniformLocation(pbrProgram, "projection");

        glUniformMatrix4fv(viewLocPBR, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLocPBR, 1, GL_FALSE, glm::value_ptr(projection));

        glUniform3fv(
            glGetUniformLocation(pbrProgram, "materialColor"),
            1,
            glm::value_ptr(currentMaterial.color)
        );

        glUniform1f(
            glGetUniformLocation(pbrProgram, "metallic"),
            currentMaterial.metallic
        );

        glUniform1f(
            glGetUniformLocation(pbrProgram, "roughness"),
            currentMaterial.roughness
        );

        glUniform1f(
            glGetUniformLocation(pbrProgram, "ao"),
            1.0f
        );

        glBindVertexArray(spheres[1]->VAO);

        spheres[1]->Draw(pbrProgram, glm::vec3(0.7f, 0.0f, 0.0f));

        glfwSwapBuffers(window);
    }

    glfwTerminate();

    return 0;
    }

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode) {
    if (action != GLFW_PRESS) return;

    Material &mat = materials[matIndex];

    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    // troca material
    if (key == GLFW_KEY_LEFT)  matIndex = (matIndex + 2) % 3;
    if (key == GLFW_KEY_RIGHT) matIndex = (matIndex + 1) % 3;

    // ROUGHNESS
    if (key == GLFW_KEY_R) mat.roughness += 0.5f;
    if (key == GLFW_KEY_F) mat.roughness -= 0.5f;

    // METALLIC
    if (key == GLFW_KEY_M) mat.metallic += 0.5f;
    if (key == GLFW_KEY_N) mat.metallic -= 0.5f;

    // SHININESS
    if (key == GLFW_KEY_K) mat.shininess += 2.0f;
    if (key == GLFW_KEY_J) mat.shininess -= 2.0f;

    // KS
    if (key == GLFW_KEY_L) mat.Ks += 0.05f;
    if (key == GLFW_KEY_H) mat.Ks -= 0.05f;

    // KD
    if (key == GLFW_KEY_D) mat.Kd += 0.05f;
    if (key == GLFW_KEY_A) mat.Kd -= 0.05f;
}

GLuint setupShader(const GLchar* fragmentShaderSource) {
    GLint success;
    GLchar infoLog[512];

    // Vertex Shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);

    glCompileShader(vertexShader);

    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);

        cout << "VERTEX SHADER ERROR\n" << infoLog << endl;
    }

    // Fragment Shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);

    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);

        cout << "FRAGMENT SHADER ERROR\n" << infoLog << endl;
    }

    // Shader Program
    GLuint shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);

        cout << "PROGRAM LINK ERROR\n" << infoLog << endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}