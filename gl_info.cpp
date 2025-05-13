// gl_info.cpp
// Author: JJ

#include "gl_info.hpp"

void printGLInfo() {
    // Get and print GL string values safely
    auto printGLString = [](GLenum name, const char* description) {
        const char* value = reinterpret_cast<const char*>(glGetString(name));
        if (value == nullptr) {
            std::cout << description << ": <Unknown>\n";
        }
        else {
            std::cout << description << ": " << value << '\n';
        }
        };

    printGLString(GL_VENDOR, "GL Vendor");
    printGLString(GL_RENDERER, "GL Renderer");
    printGLString(GL_VERSION, "GL Version");
    printGLString(GL_SHADING_LANGUAGE_VERSION, "GL Shading Language Version");

    // Get and print GL integer values
    auto printGLInt = [](GLenum name, const char* description) {
        GLint value;
        glGetIntegerv(name, &value);
        std::cout << description << ": " << value << '\n';
        };

    printGLInt(GL_MAJOR_VERSION, "GL Major Version");
    printGLInt(GL_MINOR_VERSION, "GL Minor Version");

    // Verify OpenGL version (should be at least 4.6)
    GLint major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);

    if (major < 4 || (major == 4 && minor < 6)) {
        throw std::runtime_error("OpenGL version 4.6 or higher required!");
    }

    // Get and print GL context profile
    GLint profile;
    glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profile);

    if (profile == 0) {
        std::cerr << "Error: No OpenGL profile detected!\n";
    }
    else if (profile & GL_CONTEXT_CORE_PROFILE_BIT) {
        std::cout << "OpenGL Profile: CORE\n";
    }
    else if (profile & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT) {
        std::cout << "OpenGL Profile: COMPATIBILITY\n";
    }
    else {
        throw std::runtime_error("Unknown OpenGL profile!");
    }


    // Get and print GL context flags
    GLint contextFlags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &contextFlags);

    std::cout << "OpenGL Context Flags: ";
    if (contextFlags & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT) std::cout << "FORWARD_COMPATIBLE ";
    if (contextFlags & GL_CONTEXT_FLAG_DEBUG_BIT) std::cout << "DEBUG ";
    if (contextFlags & GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT) std::cout << "ROBUST_ACCESS ";
    if (contextFlags & GL_CONTEXT_FLAG_NO_ERROR_BIT) std::cout << "NO_ERROR ";

    std::cout << '\n';
}
