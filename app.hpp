#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <opencv2/opencv.hpp>
#include "assets.hpp"
#include "ShaderProgram.hpp"
#include "Model.hpp"
#include "camera.hpp"

struct SpotLight {
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float constant;
    float linear;
    float quadratic;
    float cutoff;
    float outerCutoff;
};

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    float life;
    bool alive;

    Particle(glm::vec3 pos, glm::vec3 vel, float lifetime)
        : position(pos), velocity(vel), life(lifetime), alive(true) {
    }
};

struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float constant;
    float linear;
    float quadratic;
};

struct DirectionalLight {
    glm::vec3 direction;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

class App {
public:
    bool init();
    int run();
    void init_assets();
    App();
    ~App();

    Camera camera{ glm::vec3(0.0f, 0.0f, 5.0f) };
    std::vector<PointLight> pointLights;
    SpotLight spotLight;
    DirectionalLight sun;
    float sunAngle = 0.0f; // úhel v radiánech pro rotaci slunce

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    double lastX = 400, lastY = 300;
    bool firstMouse = true;

    // Particle logic
    std::vector<Particle> particles;

    void updateParticles(float deltaTime);
    void updateCameraHeight();
    void drawParticles();
    void spawnParticles(glm::vec3 origin, int count);

    // Maze logic from maze_gen.cpp
    uchar getmap(cv::Mat& map, int x, int y);
    cv::Point genLabyrinth(cv::Mat& map);
    bool noclip_enabled = false;  // default off
    void toggleFullscreen();

private:
    GLFWwindow* window;
    bool vsync_on = true;

    int frame_count = 0;
    double last_time = 0.0;

    static void error_callback(int error, const char* description);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);

    void updateFPS();
    void toggleVSync();

    ShaderProgram particleShader;
    GLuint particleVAO = 0;
    GLuint particleVBO = 0;
    ShaderProgram shader_program;
    Model* model = nullptr;
    GLuint VAO_ID{ 0 };
    GLuint VBO_ID{ 0 };
    GLuint texture_ID = 0;
    GLuint object1 = 0;
    GLuint object2 = 0;
    GLuint object3 = 0;
    GLuint heightmap_texture_ID = 0;            // heightmap texture
    Model* glassCube = nullptr;
    GLuint textureInit(const std::string& filename);

    bool fullscreen_enabled = false;
    int windowed_x = 100, windowed_y = 100;
    int windowed_width = 800, windowed_height = 600;

    // Maze-related
    cv::Mat mapa;
    cv::Mat heightmap_img; // grayscale heightmap image
    std::vector<Model*> moving_models;

    std::vector<Model*> maze_models;
    Model* wall_cube = nullptr;
    void generateMazeModels(const cv::Mat& mapa);

    Model* heightmap_model = nullptr;
    void initHeightmap(); // initialization
};

struct AppSettings {
    std::string appname = "default";
    int width = 800;
    int height = 600;
    bool antialiasing_enabled = false;
    int antialiasing_level = 1;
};
