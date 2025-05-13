#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "ShaderProgram.hpp"

extern float fov;
extern float aspect;
extern ShaderProgram shader;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (yoffset != 0.0) {
        fov -= static_cast<float>(yoffset);
        fov = glm::clamp(fov, 30.0f, 90.0f);

        glm::mat4 projection = glm::perspective(glm::radians(fov), aspect, 0.1f, 100.0f);
        shader.setUniform("uP_m", projection);
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);

    aspect = static_cast<float>(width) / static_cast<float>(height);
    glm::mat4 projection = glm::perspective(glm::radians(fov), aspect, 0.1f, 100.0f);
    shader.setUniform("uP_m", projection);
}
