#pragma once

#include <stdlib.h>

#include "glad/glad.h"

#include "gl.h"

class Buffer {

public:
    Buffer(GLenum target);
    ~Buffer();

    void bind() const;

    template<typename T>
    void push(const T *data, std::size_t count, GLenum usage = GL_STATIC_DRAW) const {

        GL_CALL( glBufferData(_target, sizeof(T) * count, static_cast<const void*>(data), usage) );  
    }

    void set_target(GLenum target);

private:
    GLenum _target;
    GLuint _id;
};