#pragma once

#include <string>
#include <vector>
#include <filesystem>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class ShaderProgram {
public:
    ShaderProgram() = default;
    ShaderProgram(const std::filesystem::path& VS_file, const std::filesystem::path& FS_file);

    void activate() { glUseProgram(ID); }
    void deactivate() { glUseProgram(0); }

    void clear() {
        deactivate();
        glDeleteProgram(ID);
        ID = 0;
    }

    // Uniforms
    void setUniform(const std::string& name, float val);
    void setUniform(const std::string& name, int val);
    void setUniform(const std::string& name, const glm::vec3 val);
    void setUniform(const std::string& name, const glm::vec4 val);
    void setUniform(const std::string& name, const glm::mat3 val);
    void setUniform(const std::string& name, const glm::mat4 val);

    GLuint ID{ 0 };

private:
    std::string getShaderInfoLog(GLuint obj);
    std::string getProgramInfoLog(GLuint obj);

    GLuint compile_shader(const std::filesystem::path& source_file, GLenum type);
    GLuint link_shader(const std::vector<GLuint> shader_ids);

    std::string textFileRead(const std::filesystem::path& filename);
};
