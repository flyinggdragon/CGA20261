#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Camera.h"

Camera::Camera(float WIDTH, float HEIGHT, GLuint shaderProgram, float cameraSpeed, float camSensitivity) {
	// Inicializações básicas
	sensitivity = camSensitivity;
	speed = cameraSpeed;

	lastX = WIDTH / 2.0f;
	lastY = HEIGHT / 2.0f;

	yaw = -90.0f;
	pitch = 0.0f;

	position = glm::vec3(0.0f, 0.0f, 3.0f);
	target = glm::vec3(0.0f, 0.0f, 0.0f);

	view = glm::mat4(1.0f);
	proj = glm::mat4(1.0f);

	viewLocation = glGetUniformLocation(shaderProgram, "view");
	projLocation = glGetUniformLocation(shaderProgram, "projection");

	// Cria a matriz de perspectiva e envia para o shader via uniform.
	proj = glm::perspective(glm::radians(90.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
	glUniformMatrix4fv(projLocation, 1, GL_FALSE, glm::value_ptr(proj));

	UpdateDirection();
}

Camera::~Camera() {

}

void Camera::UpdateDirection() {
	// Atualiza os vetores relacionados à câmera (front, right, up)
	glm::vec3 newDirection;
	newDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	newDirection.y = sin(glm::radians(pitch));
	newDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	front = glm::normalize(newDirection);
	right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
	up = glm::normalize(glm::cross(right, front));
}

// Cuida dos inputs de movimentação da câmera.
void Camera::ProcessInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		position += speed * front;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		position -= speed * front;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		position -= right * speed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		position += right * speed;
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// O alvo da câmera sempre é o vetor resultante da soma entre a posição e o vetor front/direção.
	target = position + front;

	// Envia a matriz lookAt (view) para o vertex shader.
	view = glm::lookAt(position, target, up);
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
}

// Cuida da movimentação a partir do mouse (yaw, pitch).
void Camera::ProcessMouseMovement(double xpos, double  ypos) {
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	UpdateDirection();
}