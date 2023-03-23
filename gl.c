#pragma once

#include <iostream>
#include <glad/glad.h>


    GLenum e;
    bool error = false;
    while ( (e = glGetError()) != GL_NO_ERROR) {

        error = true;
        std::cout << "[OpenGL Error] (" << e << ") at " << file << "(" << line << "): " << expression << std::endl;
    }

    return error;
}