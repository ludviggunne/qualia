#include "gl.h"

#include <iostream>
#include <cstdint>
#include "glad/glad.h"

bool logGLErrors(uint32_t line, const char *expression, const char *file) {

    GLenum e;
    bool error = false;

    while ( (e = glGetError()) != GL_NO_ERROR ) {

        std::cout << "[OpenGL error] (" << std::hex << e << std::dec << ") at " << file << ":" << line << " - " << expression << std::endl;
        error = true;
    }

    return error;
}