#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "assets.hpp"
#include "Mesh.hpp"
#include "ShaderProgram.hpp"
#include "OBJloader.h"

class Model {
public:
    std::vector<Mesh> meshes;
    std::string name;

    // Transformation properties
    glm::vec3 origin{};
    glm::vec3 orientation{}; // rotation in radians
    glm::vec3 scale{ 1.0f, 1.0f, 1.0f }; // default scale 1
    glm::mat4 local_model_matrix{ 1.0f }; // optional custom transform
    GLuint texture_ID{ 0 }; // each model can have its own texture


    ShaderProgram shader;
    bool transparent{ false }; // ✅ transparency flag

    Model(const std::filesystem::path& filename, ShaderProgram shader)
        : shader(shader)
    {
        std::cout << "[Model] Loading model from: " << filename << "\n";

        if (filename == "manual") {
            std::cout << "[Model] Skipping OBJ load for manual geometry.\n";
            return; // ✅ manually-added mesh will be used
        }

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> texcoords;
        std::vector<glm::vec3> normals;

        if (!loadOBJ(filename.string().c_str(), positions, texcoords, normals)) {
            std::cerr << "[Model ERROR] Failed to load OBJ: " << filename << "\n";
            return;
        }

        std::vector<vertex> combined_vertices;
        std::vector<GLuint> indices;

        for (size_t i = 0; i < positions.size(); ++i) {
            vertex v{};
            v.position = positions[i];
            v.normal = (i < normals.size()) ? normals[i] : glm::vec3(0.0f);
            v.texcoords = (i < texcoords.size()) ? texcoords[i] : glm::vec2(0.0f);
            v.color = glm::vec3(1.0f, 0.0f, 0.0f); // fallback color
            combined_vertices.push_back(v);
            indices.push_back(static_cast<GLuint>(i));
        }

        std::cout << "[Model] Loaded " << combined_vertices.size() << " vertices\n";
        meshes.emplace_back(GL_TRIANGLES, shader, combined_vertices, indices, origin, orientation);
        name = filename.filename().string();
    }

    void setTransparent(bool value) {
        transparent = value;
    }

    void update(const float delta_t) {
        // Example: origin.x += 3.0f * delta_t;
    }

    void draw(GLuint tex_ID, const glm::vec3& offset = glm::vec3(0.0f),
        const glm::vec3& rotation = glm::vec3(0.0f)) {
        glm::mat4 model_matrix = glm::mat4(1.0f);
        model_matrix = glm::translate(model_matrix, origin + offset);
        model_matrix = glm::rotate(model_matrix, rotation.x, glm::vec3(1, 0, 0));
        model_matrix = glm::rotate(model_matrix, rotation.y, glm::vec3(0, 1, 0));
        model_matrix = glm::rotate(model_matrix, rotation.z, glm::vec3(0, 0, 1));
        model_matrix = glm::scale(model_matrix, scale);

        glBindTextureUnit(0, tex_ID); // bind texture

        for (auto& mesh : meshes) {
            mesh.shader.setUniform("uM_m", model_matrix);
            mesh.draw();
        }

        // Save matrix for transparency sorting
        local_model_matrix = model_matrix;
    }

    void draw(glm::mat4 const& model_matrix) {
        for (auto& mesh : meshes) {
            glm::mat4 final_model = model_matrix * local_model_matrix;
            mesh.shader.setUniform("uM_m", final_model);
            mesh.draw();
        }
    }

    void clear() {
        for (auto& mesh : meshes) {
            mesh.clear();
        }
        meshes.clear();
    }
};
