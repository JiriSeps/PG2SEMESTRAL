// === app.cpp ===
#include "app.hpp"
#include "ShaderProgram.hpp"
#include "gl_info.hpp"
#include "gl_err_callback.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <sstream>
#include <random>
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;


float fov = 90.0f;
float aspect = 800.0f / 600.0f;
ShaderProgram shader;
GLuint maze_texture_ID = 0;
GLuint heightmap_texture_ID = 0;


App::App() : window(nullptr) {
    std::cout << "Constructed...\n";
}

void App::updateFPS() {
    double current_time = glfwGetTime();
    frame_count++;
    if (current_time - last_time >= 1.0) {
        std::ostringstream title;
        title << "OpenGL Context | FPS: " << frame_count;
        glfwSetWindowTitle(window, title.str().c_str());
        frame_count = 0;
        last_time = current_time;
    }
}

void App::toggleVSync() {
    vsync_on = !vsync_on;
    glfwSwapInterval(vsync_on ? 1 : 0);
    std::cout << "VSync " << (vsync_on ? "ON" : "OFF") << std::endl;
}

void App::spawnParticles(glm::vec3 origin, int count) {
    for (int i = 0; i < count; ++i) {
        glm::vec3 vel = glm::sphericalRand(3.0f); // random unit sphere vector
        particles.emplace_back(origin, vel, 2.0f); // 2 seconds lifetime
    }
}

void App::updateParticles(float dt) {
    for (auto& p : particles) {
        if (!p.alive) continue;

        p.velocity += glm::vec3(0, -9.8f, 0) * dt;
        p.position += p.velocity * dt;
        p.life -= dt;

        if (p.life <= 0.0f) p.alive = false;
    }

    particles.erase(
        std::remove_if(particles.begin(), particles.end(), [](const Particle& p) { return !p.alive; }),
        particles.end()
    );

    //std::cout << "Alive particles: " << std::count_if(particles.begin(), particles.end(), [](const Particle& p) { return p.alive; }) << "\n";

}

void App::updateCameraHeight() {
    float maze_floor_y = -68.0f;
    float eye_height = 1.0f;
    float offsetX = mapa.cols / 2.0f;
    float offsetZ = mapa.rows / 2.0f;

    int x_tile = static_cast<int>(floor(camera.Position.x + offsetX));
    int z_tile = static_cast<int>(floor(camera.Position.z + offsetZ));

    bool in_maze_bounds = x_tile >= 0 && x_tile < mapa.cols && z_tile >= 0 && z_tile < mapa.rows;
    bool in_maze_height = camera.Position.y > -70.0f && camera.Position.y < -58.0f;

    if (in_maze_bounds && in_maze_height && getmap(mapa, x_tile, z_tile) != '#') {
        float targetY = maze_floor_y + eye_height;
        camera.Position.y = glm::mix(camera.Position.y, targetY, 0.1f);
    }
    else if (!heightmap_img.empty()) {
        float x_world = camera.Position.x;
        float z_world = camera.Position.z;

        float x_local = (x_world + (heightmap_img.cols / 2.0f) * 0.5f) / 0.5f;
        float z_local = (z_world + (heightmap_img.rows / 2.0f) * 0.5f) / 0.5f;

        int x_px = static_cast<int>(std::clamp(x_local, 0.0f, float(heightmap_img.cols - 1)));
        int z_px = static_cast<int>(std::clamp(z_local, 0.0f, float(heightmap_img.rows - 1)));

        uchar pixel = heightmap_img.at<uchar>(z_px, x_px);
        float h = -pixel * 0.25f;
        float targetY = h + (-10.0f) + 1.0f;

        camera.Position.y = glm::mix(camera.Position.y, targetY, 0.1f);
    }
}

void App::drawParticles() {
    std::vector<glm::vec3> positions;
    for (const auto& p : particles)
        if (p.alive)
            positions.push_back(p.position);

    if (!positions.empty()) {
        particleShader.activate();
        particleShader.setUniform("uV_m", camera.GetViewMatrix());
        particleShader.setUniform("uP_m", glm::perspective(glm::radians(60.0f), 1024.0f / 768.0f, 0.1f, 1000.0f));

        glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size() * sizeof(glm::vec3), positions.data());

        glBindVertexArray(particleVAO);
        glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(positions.size()));
        glBindVertexArray(0);
    }
}

void App::toggleFullscreen() {
    fullscreen_enabled = !fullscreen_enabled;

    if (fullscreen_enabled) {
        // Ulož pozici a velikost okna
        glfwGetWindowPos(window, &windowed_x, &windowed_y);
        glfwGetWindowSize(window, &windowed_width, &windowed_height);

        // Přepni do fullscreen na primární monitor
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);

        // Nastav nový viewport a projekci
        int fb_width, fb_height;
        glfwGetFramebufferSize(window, &fb_width, &fb_height);
        glViewport(0, 0, fb_width, fb_height);

        aspect = static_cast<float>(fb_width) / static_cast<float>(fb_height);
        glm::mat4 projection = glm::perspective(glm::radians(fov), aspect, 0.1f, 1000.0f);
        shader_program.setUniform("uP_m", projection);
    }
    else {
        // Zpět do windowed režimu
        glfwSetWindowMonitor(window, nullptr, windowed_x, windowed_y, windowed_width, windowed_height, 0);

        // Nastav nový viewport a projekci
        int fb_width, fb_height;
        glfwGetFramebufferSize(window, &fb_width, &fb_height);
        glViewport(0, 0, fb_width, fb_height);

        aspect = static_cast<float>(fb_width) / static_cast<float>(fb_height);
        glm::mat4 projection = glm::perspective(glm::radians(fov), aspect, 0.1f, 1000.0f);
        shader_program.setUniform("uP_m", projection);
    }
}

void App::generateMazeModels(const cv::Mat& mapa) {
    if (!wall_cube) return;

    float offsetX = mapa.cols / 2.0f;
    float offsetZ = mapa.rows / 2.0f;
    float maze_base_y = -67.0f; // or adjust slightly higher if you want them to sit on terrain
    float maze_floor_base_y = -68.0f;

    for (int j = 0; j < mapa.rows; ++j) {
        for (int i = 0; i < mapa.cols; ++i) {
            char cell = mapa.at<uchar>(j, i);

            // === Podlaha pro všechny typy buněk ===
            Model* floor = new Model(*wall_cube);
            floor->origin = glm::vec3(i - offsetX + 0.5f, maze_floor_base_y, j - offsetZ + 0.5f);
            floor->scale = glm::vec3(1.0f, 0.05f, 1.0f);
            floor->texture_ID = heightmap_texture_ID;
            maze_models.push_back(floor);

            // === Zdi ===
            if (cell == '#') {
                Model* cube = new Model(*wall_cube);
                cube->origin = glm::vec3(i - offsetX + 0.5f, maze_base_y, j - offsetZ + 0.5f);// výš posunuté
                cube->scale = glm::vec3(1.0f, 2.0f, 1.0f);                // dvojnásobná výška
                maze_models.push_back(cube);
                std::cout << "Wall at: " << cube->origin.x << ", " << cube->origin.y << ", " << cube->origin.z << "\n";

            }

        }
    }
}

Model* makeCubeModel(ShaderProgram& shader) {
    std::vector<vertex> vertices;
    std::vector<GLuint> indices;

    glm::vec3 normals[] = {
        {0, 0, 1},  // front
        {0, 0, -1}, // back
        {1, 0, 0},  // right
        {-1, 0, 0}, // left
        {0, 1, 0},  // top
        {0, -1, 0}  // bottom
    };

    glm::vec3 facePositions[6][4] = {
        // Front
        { {-0.5f,-0.5f, 0.5f}, {0.5f,-0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f} },
        // Back
        { {0.5f,-0.5f,-0.5f}, {-0.5f,-0.5f,-0.5f}, {-0.5f, 0.5f,-0.5f}, {0.5f, 0.5f,-0.5f} },
        // Right
        { {0.5f,-0.5f, 0.5f}, {0.5f,-0.5f,-0.5f}, {0.5f, 0.5f,-0.5f}, {0.5f, 0.5f, 0.5f} },
        // Left
        { {-0.5f,-0.5f,-0.5f}, {-0.5f,-0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f,-0.5f} },
        // Top
        { {-0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}, {0.5f, 0.5f,-0.5f}, {-0.5f, 0.5f,-0.5f} },
        // Bottom
        { {-0.5f,-0.5f,-0.5f}, {0.5f,-0.5f,-0.5f}, {0.5f,-0.5f, 0.5f}, {-0.5f,-0.5f, 0.5f} },
    };

    glm::vec2 tex[] = { {0,0}, {1,0}, {1,1}, {0,1} };

    for (int face = 0; face < 6; ++face) {
        for (int i = 0; i < 4; ++i) {
            vertex v;
            v.position = facePositions[face][i];
            v.normal = normals[face];
            v.texcoords = tex[i];
            v.color = glm::vec3(1.0f); // white
            vertices.push_back(v);
        }

        int start = face * 4;
        indices.push_back(start + 0);
        indices.push_back(start + 1);
        indices.push_back(start + 2);
        indices.push_back(start + 2);
        indices.push_back(start + 3);
        indices.push_back(start + 0);
    }

    Model* m = new Model("manual", shader);
    m->meshes.emplace_back(GL_TRIANGLES, shader, vertices, indices, glm::vec3(0), glm::vec3(0));
    return m;
}

bool App::init() {
    // === Load config ===
    AppSettings config;
    try {
        std::ifstream settings_file("app_settings.json");
        json settings;
        settings_file >> settings;

        config.appname = settings.value("appname", "default");
        config.width = settings["default_resolution"].value("x", 800);
        config.height = settings["default_resolution"].value("y", 600);
        config.antialiasing_enabled = settings.value("antialiasing_enabled", false);
        config.antialiasing_level = settings.value("antialiasing_level", 1);
    }
    catch (...) {
        std::cerr << "[Config] Failed to load app_settings.json, using defaults\n";
    }

    // === Init GLFW and apply AA settings ===
    if (!glfwInit()) throw std::runtime_error("Failed to initialize GLFW!");

    if (config.antialiasing_enabled && config.antialiasing_level > 1 && config.antialiasing_level <= 8) {
        glfwWindowHint(GLFW_SAMPLES, config.antialiasing_level);
        std::cout << "[AA] Enabled, level = " << config.antialiasing_level << "\n";
    }
    else if (config.antialiasing_enabled) {
        std::cerr << "[AA] Warning: Invalid AA level " << config.antialiasing_level << ", disabling AA\n";
    }
    else {
        std::cout << "[AA] Disabled\n";
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    window = glfwCreateWindow(config.width, config.height, config.appname.c_str(), nullptr, nullptr);
    if (!window) throw std::runtime_error("Failed to create GLFW window!");

    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);

    glfwSetErrorCallback(error_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    if (glewInit() != GLEW_OK) throw std::runtime_error("Failed to initialize GLEW!");

    if (GLEW_ARB_debug_output) {
        glDebugMessageCallback(MessageCallback, 0);
        glEnable(GL_DEBUG_OUTPUT);
        std::cout << "GL_DEBUG enabled." << std::endl;
    }

    if (config.antialiasing_enabled && config.antialiasing_level > 1 && config.antialiasing_level <= 8)
        glEnable(GL_MULTISAMPLE);

    glfwSwapInterval(1);
    vsync_on = true;

    printGLInfo();
    init_assets();
    return true;
}

void App::init_assets() {
    // === Load config ===
    json settings;
    try {
        std::ifstream settings_file("app_settings.json");
        if (!settings_file.is_open()) throw std::runtime_error("Cannot open app_settings.json");
        settings_file >> settings;
    } catch (const std::exception& e) {
        std::cerr << "[Settings Error] " << e.what() << "\nUsing defaults...\n";
    }

    std::string base       = settings.value("resource_path", "resources/");
    std::string shader_dir = base + settings.value("shader_dir", "shaders/");
    std::string texture_dir = base + settings.value("texture_dir", "textures/");
    std::string object_dir  = base + settings.value("object_dir", "objects/");

    try {
        shader_program = ShaderProgram(shader_dir + "tex.vert", shader_dir + "tex.frag");
        particleShader = ShaderProgram(shader_dir + "particle.vert", shader_dir + "particle.frag");
        shader_program.activate();
    } catch (const std::exception& e) {
        std::cerr << "[Shader Load Error] " << e.what() << std::endl;
        return;
    }

    // === Setup VAO/VBO for particles ===
    glGenVertexArrays(1, &particleVAO);
    glGenBuffers(1, &particleVBO);
    glBindVertexArray(particleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 1000, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glBindVertexArray(0);
    glEnable(GL_PROGRAM_POINT_SIZE);

    // === Light setup ===
    sun.ambient = glm::vec3(0.2f);
    sun.diffuse = glm::vec3(0.7f);
    sun.specular = glm::vec3(1.0f);

    pointLights = {
        { glm::vec3(4, -60, 2), glm::vec3(0.05f), glm::vec3(1.0f, 0.1f, 0.1f), glm::vec3(0.0f), 1.0f, 0.045f, 0.0075f },
        { glm::vec3(-4, -60, 5), glm::vec3(0.05f), glm::vec3(0.1f, 1.0f, 0.1f), glm::vec3(0.0f), 1.0f, 0.045f, 0.0075f },
        { glm::vec3(0, -60, -5), glm::vec3(0.05f), glm::vec3(0.1f, 0.1f, 1.0f), glm::vec3(0.0f), 1.0f, 0.045f, 0.0075f }
    };

    spotLight.ambient = glm::vec3(0.2f);
    spotLight.diffuse = glm::vec3(1.5f);
    spotLight.specular = glm::vec3(0.5f);
    spotLight.constant = 1.0f;
    spotLight.linear = 0.09f;
    spotLight.quadratic = 0.032f;
    spotLight.cutoff = glm::cos(glm::radians(15.0f));
    spotLight.outerCutoff = glm::cos(glm::radians(25.0f));

    // === Load textures ===
    maze_texture_ID     = textureInit(texture_dir + "box_rgb888.png");
    heightmap_texture_ID = textureInit(texture_dir + "TextureDouble_A.png");
    object1             = textureInit(texture_dir + "wire2.png");
    object2             = textureInit(texture_dir + "wg_lily_00.png");
    object3             = textureInit(texture_dir + "wg_lily_01.png");

    // === Generate maze ===
    wall_cube = makeCubeModel(shader_program);
    mapa = cv::Mat(10, 25, CV_8U);
    cv::Point start = genLabyrinth(mapa);
    generateMazeModels(mapa);
    camera.Position = glm::vec3(start.x + 0.5f, -59.0f, start.y + 0.5f);

    // === Transparent cubes ===
    float maze_floor_y = -60.0f;
    auto addGlassCube = [&](glm::vec3 pos, GLuint tex) {
        Model* cube = makeCubeModel(shader_program);
        cube->origin = pos;
        cube->scale = glm::vec3(1.0f);
        cube->transparent = true;
        cube->texture_ID = tex;
        maze_models.push_back(cube);
    };
    addGlassCube(glm::vec3(5.0f, maze_floor_y, 5.0f), object1);
    addGlassCube(glm::vec3(7.0f, maze_floor_y, 5.0f), object2);
    addGlassCube(glm::vec3(6.0f, maze_floor_y, 7.0f), object3);

    // === Load OBJ models ===
    auto loadModel = [&](const std::string& filename, glm::vec3 pos, glm::vec3 scale) {
        Model* model = new Model(object_dir + filename, shader_program);
        model->origin = pos;
        model->scale = scale;
        model->transparent = false;
        moving_models.push_back(model);
    };
    loadModel("teapot_tri_vnt.obj", glm::vec3(3.0f, maze_floor_y, 3.0f), glm::vec3(0.5f));
    loadModel("bunny10k.obj", glm::vec3(3.0f, maze_floor_y, 10.0f), glm::vec3(50.0f));

    // === Camera and projection ===
    glm::mat4 projection = glm::perspective(glm::radians(60.0f), 1024.0f / 768.0f, 0.1f, 1000.0f);
    shader_program.setUniform("uP_m", projection);
    camera = Camera(glm::vec3(start.x + 0.5f, -59.0f, start.y + 0.5f));

    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    initHeightmap();
}

GLuint App::textureInit(const std::string& filename) {
    std::cout << "[Texture] Loading texture: " << filename << "\n";

    // Načtení obrázku včetně alfa kanálu, pokud existuje
    cv::Mat img = cv::imread(filename, cv::IMREAD_UNCHANGED);
    if (img.empty()) {
        std::cerr << "[Texture ERROR] Failed to load image: " << filename << "\n";
        return 0;
    }

    int channels = img.channels();
    int width = img.cols;
    int height = img.rows;
    GLenum internal_format = GL_RGB8;
    GLenum format = GL_RGB;

    if (channels == 4) {
        cv::cvtColor(img, img, cv::COLOR_BGRA2RGBA);
        internal_format = GL_RGBA8;
        format = GL_RGBA;
    }
    else if (channels == 3) {
        cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
        internal_format = GL_RGB8;
        format = GL_RGB;
    }
    else {
        std::cerr << "[Texture ERROR] Unsupported number of channels: " << channels << "\n";
        return 0;
    }

    // Vytvoření a nahrání textury do GPU
    GLuint tex_ID;
    glCreateTextures(GL_TEXTURE_2D, 1, &tex_ID);
    glTextureStorage2D(tex_ID, 1, internal_format, width, height);
    glTextureSubImage2D(tex_ID, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, img.data);

    // Parametry textury
    glTextureParameteri(tex_ID, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(tex_ID, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(tex_ID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(tex_ID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenerateTextureMipmap(tex_ID);

    std::cout << "[Texture] Loaded " << filename << " | " << width << "x" << height << " | channels=" << channels << "\n";
    return tex_ID;
}

int App::run() {
    last_time = glfwGetTime();

    auto updateSun = [&](float dt) {
        sunAngle += dt * 0.1f;
        if (sunAngle > glm::two_pi<float>()) sunAngle -= glm::two_pi<float>();

        glm::vec3 dir{ cos(sunAngle), sin(sunAngle) * 0.7f + 0.7f, sin(sunAngle * 0.5f) };
        sun.direction = glm::normalize(dir);
        float h = glm::clamp(sun.direction.y, 0.0f, 1.0f);

        sun.ambient = glm::vec3(glm::mix(0.05f, 0.2f, h)) * 2.0f;
        sun.diffuse = glm::vec3(0.8f) * glm::mix(0.0f, 1.0f, h) * 4.0f;
        };

    auto handleCameraCollision = [&](glm::vec3 proposedPos) {
        float radius = 0.20f;
        float offsetX = mapa.cols / 2.0f;
        float offsetZ = mapa.rows / 2.0f;
        float halfCell = 0.5f;

        auto isBlocked = [&](const glm::vec3& pos) {
            int min_x = static_cast<int>(floor(pos.x - radius + offsetX));
            int max_x = static_cast<int>(floor(pos.x + radius + offsetX));
            int min_z = static_cast<int>(floor(pos.z - radius + offsetZ));
            int max_z = static_cast<int>(floor(pos.z + radius + offsetZ));

            for (int x = min_x; x <= max_x; ++x)
                for (int z = min_z; z <= max_z; ++z)
                    if (x >= 0 && x < mapa.cols && z >= 0 && z < mapa.rows && getmap(mapa, x, z) == '#') {
                        glm::vec2 delta = glm::abs(glm::vec2(pos.x - (x - offsetX + 0.5f), pos.z - (z - offsetZ + 0.5f)));
                        if (delta.x < (halfCell + radius) && delta.y < (halfCell + radius))
                            return true;
                    }

            return false;
            };

        bool in_maze_layer = (proposedPos.y > -70.0f && proposedPos.y < -65.0f);

        if (noclip_enabled || !in_maze_layer)
            return proposedPos;

        if (!isBlocked(proposedPos)) return proposedPos;

        glm::vec3 moveX = { proposedPos.x - camera.Position.x, 0, 0 };
        glm::vec3 moveZ = { 0, 0, proposedPos.z - camera.Position.z };

        if (!isBlocked(camera.Position + moveX)) return camera.Position + moveX;
        if (!isBlocked(camera.Position + moveZ)) return camera.Position + moveZ;

        return camera.Position; // stuck
        };

    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        deltaTime = static_cast<float>(now - lastFrame);
        lastFrame = now;

        updateSun(deltaTime);
        updateParticles(deltaTime);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader_program.activate();

        // === Camera movement and height ===
        glm::vec3 move = camera.ProcessInput(window, deltaTime);
        camera.Position = handleCameraCollision(camera.Position + move);

        if (!noclip_enabled) updateCameraHeight();  // Keep your existing height logic here as a helper

        // === Upload matrices ===
        glm::mat4 view = camera.GetViewMatrix();
        shader_program.setUniform("uV_m", view);

        // === Light uniforms ===
        shader_program.setUniform("directionalLight_direction", sun.direction);
        shader_program.setUniform("directionalLight_ambient", sun.ambient);
        shader_program.setUniform("directionalLight_diffuse", sun.diffuse);
        shader_program.setUniform("directionalLight_specular", sun.specular);

        spotLight.position = camera.Position;
        spotLight.direction = glm::normalize(camera.Front);

        shader_program.setUniform("spotLight_position", spotLight.position);
        shader_program.setUniform("spotLight_direction", spotLight.direction);
        shader_program.setUniform("spotLight_constant", spotLight.constant);
        shader_program.setUniform("spotLight_linear", spotLight.linear);
        shader_program.setUniform("spotLight_quadratic", spotLight.quadratic);
        shader_program.setUniform("spotLight_cutoff", spotLight.cutoff);
        shader_program.setUniform("spotLight_outerCutoff", spotLight.outerCutoff);

        for (int i = 0; i < 3; ++i) {
            std::string idx = std::to_string(i);
            shader_program.setUniform("pointLightPositions[" + idx + "]", pointLights[i].position);
            shader_program.setUniform("pointLights[" + idx + "].diffuse", pointLights[i].diffuse);
            shader_program.setUniform("pointLights[" + idx + "].constant", pointLights[i].constant);
            shader_program.setUniform("pointLights[" + idx + "].linear", pointLights[i].linear);
            shader_program.setUniform("pointLights[" + idx + "].quadratic", pointLights[i].quadratic);
        }

        shader_program.setUniform("shininess", 32.0f);

        // === Update animated models ===
        float t = static_cast<float>(now);
        for (Model* m : moving_models) {
            m->origin.x = sin(t) * 3.0f;
            m->orientation.y = t * 1.5f;
        }

        // === Draw opaque ===
        for (Model* m : maze_models)
            if (!m->transparent) m->draw(maze_texture_ID);

        if (heightmap_model && !heightmap_model->transparent)
            heightmap_model->draw(heightmap_texture_ID);

        for (Model* m : moving_models)
            if (!m->transparent) m->draw(m->texture_ID, glm::vec3(0.0f), m->orientation);

        // === Transparent sorting + drawing ===
        std::vector<Model*> transparent;
        for (auto* m : maze_models)
            if (m->transparent) transparent.push_back(m);
        if (heightmap_model && heightmap_model->transparent)
            transparent.push_back(heightmap_model);

        std::sort(transparent.begin(), transparent.end(), [&](Model* a, Model* b) {
            return glm::distance(camera.Position, a->origin) > glm::distance(camera.Position, b->origin);
            });

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);

        for (auto* m : transparent) {
            GLuint tex = (m == heightmap_model) ? heightmap_texture_ID : m->texture_ID;
            m->draw(tex);
        }

        drawParticles();

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        updateFPS();
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    return EXIT_SUCCESS;
}

App::~App() {
    if (window) glfwDestroyWindow(window);
    glfwTerminate();

    for (auto* m : maze_models) delete m;
    maze_models.clear();
    if (wall_cube) delete wall_cube;
    if (model) { model->clear(); delete model; }

    shader_program.clear();
    std::cout << "Bye...\n";
}