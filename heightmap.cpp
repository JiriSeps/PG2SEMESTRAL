// === heightmap.cpp (fixed and integrated version with debug prints + corrected terrain orientation) ===
#include "app.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <filesystem>

// Choose subtexture based on height
glm::vec2 get_subtex_by_height(float height) {
    if (height > 0.9f)
        return glm::vec2(2, 11) / 16.0f; // snow
    else if (height > 0.8f)
        return glm::vec2(3, 11) / 16.0f; // ice
    else if (height > 0.5f)
        return glm::vec2(0, 14) / 16.0f; // rock
    else if (height > 0.3f)
        return glm::vec2(2, 15) / 16.0f; // soil
    else
        return glm::vec2(0, 11) / 16.0f; // grass
}

void App::initHeightmap() {
    std::filesystem::path hm_file("C:/Users/Jirka/source/repos/my_app/resources/textures/heights.png");
    cv::Mat hmap = cv::imread(hm_file.string(), cv::IMREAD_GRAYSCALE);
    if (hmap.empty()) {
        throw std::runtime_error("ERR: Height map empty? File: " + hm_file.string());
    }

    heightmap_img = hmap.clone(); // Copy image for use in Y-smoothing



    if (hmap.empty()) {
        throw std::runtime_error("ERR: Height map empty? File: " + hm_file.string());
    }

    std::cout << "[Heightmap] Loaded: " << hm_file << " (" << hmap.cols << "x" << hmap.rows << ")\n";

    std::vector<vertex> vertices;
    std::vector<GLuint> indices;

    const unsigned int step = 10;
    float height_scale = 0.25f;

    for (unsigned int x = 0; x < hmap.cols - step; x += step) {
        for (unsigned int z = 0; z < hmap.rows - step; z += step) {
            // Invert height to correct flipped orientation
            float h0 = - hmap.at<uchar>(z, x) * height_scale;
            float h1 = - hmap.at<uchar>(z, x + step) * height_scale;
            float h2 = - hmap.at<uchar>(z + step, x + step) * height_scale;
            float h3 = - hmap.at<uchar>(z + step, x) * height_scale;

            glm::vec3 p0(x, h0, z);
            glm::vec3 p1(x + step, h1, z);
            glm::vec3 p2(x + step, h2, z + step);
            glm::vec3 p3(x, h3, z + step);

            float max_h = std::max({ h0, h1, h2, h3 }) / (255.0f * height_scale);

            glm::vec2 base_tc = get_subtex_by_height(max_h);
            glm::vec2 offset(1.0f / 16.0f, 1.0f / 16.0f);

            glm::vec2 t0 = base_tc;
            glm::vec2 t1 = base_tc + glm::vec2(offset.x, 0);
            glm::vec2 t2 = base_tc + offset;
            glm::vec2 t3 = base_tc + glm::vec2(0, offset.y);

            glm::vec3 n1 = glm::normalize(glm::cross(p1 - p0, p2 - p0));
            glm::vec3 n2 = glm::normalize(glm::cross(p2 - p0, p3 - p0));
            glm::vec3 navg = glm::normalize(n1 + n2);

            GLuint base = vertices.size();

            vertices.emplace_back(vertex(p0, navg, t0));
            vertices.emplace_back(vertex(p1, n1, t1));
            vertices.emplace_back(vertex(p2, navg, t2));
            vertices.emplace_back(vertex(p3, n2, t3));

            indices.push_back(base + 0);
            indices.push_back(base + 1);
            indices.push_back(base + 2);
            indices.push_back(base + 0);
            indices.push_back(base + 2);
            indices.push_back(base + 3);
        }
    }

    std::cout << "[Heightmap] Generated vertices: " << vertices.size() << ", indices: " << indices.size() << "\n";

    heightmap_model = new Model("manual", shader_program);
    heightmap_model->meshes.emplace_back(GL_TRIANGLES, shader_program, vertices, indices, glm::vec3(0), glm::vec3(0));

    // Lower and reposition terrain to ground level below maze
    heightmap_model->origin = glm::vec3(
        -((hmap.cols / 2.0f) * 0.5f), // Adjusted by scale factor (0.5f)
        -10.0f,                       // Keep your vertical offset
        -((hmap.rows / 2.0f) * 0.5f)  // Adjusted by scale factor (0.5f)
    );

    heightmap_model->scale = glm::vec3(0.5f, 1.0f, 0.5f);


    std::cout << "[Heightmap] Model created and ready to draw.\n";
}


// C:/Users/Jirka/Downloads/