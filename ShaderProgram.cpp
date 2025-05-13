#include "ShaderProgram.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

ShaderProgram::ShaderProgram(const std::filesystem::path& VS_file, const std::filesystem::path& FS_file) {
    std::cout << "[DEBUG] Reading and compiling vertex shader: " << VS_file << "\n";
    std::cout << textFileRead(VS_file) << "\n"; // ✅ <- Add this here

    std::vector<GLuint> shader_ids;
    shader_ids.push_back(compile_shader(VS_file, GL_VERTEX_SHADER));
    shader_ids.push_back(compile_shader(FS_file, GL_FRAGMENT_SHADER));

    ID = link_shader(shader_ids);
    std::cout << "[ShaderProgram] Program created with ID = " << ID << "\n";
}

void ShaderProgram::setUniform(const std::string& name, float val) {
    if (ID == 0) {
        std::cerr << "[Uniform ERROR] Attempted to set uniform '" << name << "' on null program ID.\n";
        return;
    }

    GLint loc = glGetUniformLocation(ID, name.c_str());
    if (loc == -1) {
        std::cerr << "[Uniform WARNING] '" << name << "' not found or unused in shader.\n";
        return;
    }

    glUniform1f(loc, val);
}


void ShaderProgram::setUniform(const std::string& name, int val) {
    GLint loc = glGetUniformLocation(ID, name.c_str());
    if (loc == -1) {
        std::cerr << "Uniform not found: " << name << "\n";
        return;
    }
    glUniform1i(loc, val);
}

void ShaderProgram::setUniform(const std::string& name, const glm::vec3 val) {
    GLint loc = glGetUniformLocation(ID, name.c_str());
    if (loc == -1) {
        std::cerr << "Uniform not found: " << name << "\n";
        return;
    }
    glUniform3fv(loc, 1, glm::value_ptr(val));
}

void ShaderProgram::setUniform(const std::string& name, const glm::vec4 val) {
    GLint loc = glGetUniformLocation(ID, name.c_str());
    if (loc == -1) {
        std::cerr << "Uniform not found: " << name << "\n";
        return;
    }
    glUniform4fv(loc, 1, glm::value_ptr(val));
}

void ShaderProgram::setUniform(const std::string& name, const glm::mat3 val) {
    GLint loc = glGetUniformLocation(ID, name.c_str());
    if (loc == -1) {
        std::cerr << "Uniform not found: " << name << "\n";
        return;
    }
    glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(val));
}

void ShaderProgram::setUniform(const std::string& name, const glm::mat4 val) {
    GLint loc = glGetUniformLocation(ID, name.c_str());
    if (loc == -1) {
        std::cerr << "Uniform not found: " << name << "\n";
        return;
    }
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(val));
}

std::string ShaderProgram::getShaderInfoLog(GLuint obj) {
    GLint log_length = 0;
    glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &log_length);
    if (log_length > 0) {
        std::vector<char> log(log_length);
        glGetShaderInfoLog(obj, log_length, nullptr, log.data());
        return std::string(log.begin(), log.end());
    }
    return "";
}

std::string ShaderProgram::getProgramInfoLog(GLuint obj) {
    GLint log_length = 0;
    glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &log_length);
    if (log_length > 0) {
        std::vector<char> log(log_length);
        glGetProgramInfoLog(obj, log_length, nullptr, log.data());
        return std::string(log.begin(), log.end());
    }
    return "";
}

GLuint ShaderProgram::compile_shader(const std::filesystem::path& source_file, GLenum type) {
    std::cout << "[Shader] Compiling: " << source_file << "\n";

    GLuint shader = glCreateShader(type);
    if (shader == 0) throw std::runtime_error("glCreateShader failed");

    std::string src = textFileRead(source_file);
    const char* src_ptr = src.c_str();
    glShaderSource(shader, 1, &src_ptr, nullptr);
    glCompileShader(shader);

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        std::string log = getShaderInfoLog(shader);
        std::cerr << "[Shader ERROR] Failed to compile " << source_file << ":\n" << log << "\n";
        glDeleteShader(shader);
        throw std::runtime_error("Shader compilation failed");
    }
    else {
        std::string log = getShaderInfoLog(shader);
        if (!log.empty()) std::cout << "[Shader LOG] " << log << "\n";
    }

    return shader;
}


GLuint ShaderProgram::link_shader(const std::vector<GLuint> shader_ids) {
    GLuint program = glCreateProgram();
    if (program == 0) throw std::runtime_error("glCreateProgram failed");

    for (GLuint id : shader_ids)
        glAttachShader(program, id);

    glLinkProgram(program);

    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success) {
        std::string log = getProgramInfoLog(program);
        std::cerr << "[Shader ERROR] Program linking failed:\n" << log << "\n";
        glDeleteProgram(program);
        throw std::runtime_error("Shader program linking failed");
    }
    else {
        std::string log = getProgramInfoLog(program);
        if (!log.empty()) std::cout << "[Linker LOG] " << log << "\n";
    }

    // Detach and delete shaders after linking
    for (GLuint id : shader_ids)
        glDeleteShader(id);

    std::cout << "[Shader] Link successful. Program ID: " << program << "\n";
    return program;
}

std::string ShaderProgram::textFileRead(const std::filesystem::path& filename) {
    std::ifstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("Could not open file: " + filename.string());
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}
