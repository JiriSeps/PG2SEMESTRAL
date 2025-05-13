#include "app.hpp"

void App::error_callback(int error, const char* description) {
    std::cerr << "Error: " << description << std::endl;
}

void App::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (yoffset > 0.0) {
        std::cout << "Wheel up...\n";
    }
}
void App::cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    if (!app) return;

    if (app->firstMouse) {
        app->lastX = xpos;
        app->lastY = ypos;
        app->firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos - app->lastX);
    float yoffset = static_cast<float>(app->lastY - ypos); // reversed y

    app->lastX = xpos;
    app->lastY = ypos;

    app->camera.ProcessMouseMovement(xoffset, yoffset);
}


void App::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    if (!app) return;

    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        case GLFW_KEY_V:
            app->toggleVSync();
            break;
        case GLFW_KEY_F3:
            app->noclip_enabled = !app->noclip_enabled;
            std::cout << "[Noclip] " << (app->noclip_enabled ? "ENABLED\n" : "DISABLED\n");
            break;
        case GLFW_KEY_SPACE:
            app->spawnParticles(app->camera.Position, 50); // Emit 50 particles from player
            break;
        case GLFW_KEY_F11:
            app->toggleFullscreen();
            break;



        }
    }
}
