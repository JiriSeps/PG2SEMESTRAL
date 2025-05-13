#ifndef __GL_ERR_CALLBACK_H__
#define __GL_ERR_CALLBACK_H__

#include <GL/glew.h> 
#include <GL/wglew.h> 
#include <GLFW/glfw3.h>

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

#endif // __GL_ERR_CALLBACK_H__
