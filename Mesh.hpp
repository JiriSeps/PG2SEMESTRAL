#pragma once

#include <string>
#include <vector>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "assets.hpp"
#include "ShaderProgram.hpp"

class Mesh {
public:
    glm::vec3 origin{};
    glm::vec3 orientation{};

    GLuint texture_id{ 0 };
    GLenum primitive_type = GL_TRIANGLES;
    ShaderProgram shader;

    glm::vec4 ambient_material{ 1.0f };
    glm::vec4 diffuse_material{ 1.0f };
    glm::vec4 specular_material{ 1.0f };
    float reflectivity{ 1.0f };

    Mesh(GLenum primitive_type,
        ShaderProgram shader,
        const std::vector<vertex>& vertices,
        const std::vector<GLuint>& indices,
        const glm::vec3& origin,
        const glm::vec3& orientation,
        GLuint texture_id = 0)
        : primitive_type(primitive_type),
        shader(shader),
        vertices(vertices),
        indices(indices),
        origin(origin),
        orientation(orientation),
        texture_id(texture_id)
    {
        glCreateVertexArrays(1, &VAO);
        glCreateBuffers(1, &VBO);
        glCreateBuffers(1, &EBO);

        glNamedBufferData(VBO, vertices.size() * sizeof(vertex), vertices.data(), GL_STATIC_DRAW);
        glNamedBufferData(EBO, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

        glVertexArrayVertexBuffer(VAO, 0, VBO, 0, sizeof(vertex));
        glVertexArrayElementBuffer(VAO, EBO);

        // Position -> location 0
        glEnableVertexArrayAttrib(VAO, 0);
        glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, offsetof(vertex, position));
        glVertexArrayAttribBinding(VAO, 0, 0);

        // Color -> location 1
        glEnableVertexArrayAttrib(VAO, 1);
        glVertexArrayAttribFormat(VAO, 1, 3, GL_FLOAT, GL_FALSE, offsetof(vertex, color));
        glVertexArrayAttribBinding(VAO, 1, 0);

        // Texcoords -> location 2 (optional for now)
        glEnableVertexArrayAttrib(VAO, 2);
        glVertexArrayAttribFormat(VAO, 2, 2, GL_FLOAT, GL_FALSE, offsetof(vertex, texcoords));
        glVertexArrayAttribBinding(VAO, 2, 0);


    }

    // In Mesh.hpp
    void draw() {
        shader.activate();
        glBindVertexArray(VAO);
        glDrawElements(primitive_type, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }


    void clear() {
        if (VBO) glDeleteBuffers(1, &VBO);
        if (EBO) glDeleteBuffers(1, &EBO);
        if (VAO) glDeleteVertexArrays(1, &VAO);
        VBO = EBO = VAO = 0;
    }

private:
    std::vector<vertex> vertices;
    std::vector<GLuint> indices;

    GLuint VAO{ 0 }, VBO{ 0 }, EBO{ 0 };
};
