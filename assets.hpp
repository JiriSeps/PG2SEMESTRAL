#pragma once

#include <string>
#include <GL/glew.h> 
#include <GL/wglew.h> 
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// vertex description
struct vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoords;
    glm::vec3 color;

    // useful for creating new vertices in procedural geometry
    vertex(glm::vec3 pos, glm::vec3 norm, glm::vec2 tex)
        : position(pos), normal(norm), texcoords(tex), color(glm::vec3(1.0f)) {
    }

    vertex() = default;
};

