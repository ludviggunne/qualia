#pragma once

#include "glad/glad.h"

class Texture {

public:
    Texture(uint32_t width, uint32_t height, GLenum format);
    ~Texture();

    void bind() const;

    void bind_to_unit(GLuint unit) const;

    void bind_as_image(GLuint unit) const;

    void set_parameter(GLenum param, GLenum value) const;

    uint32_t width() const;
    uint32_t height() const;

    void load_image(const void *image_data, GLenum format, GLenum data_type = GL_UNSIGNED_BYTE);

    void get_image(void *image_data, GLenum format, GLenum data_type = GL_UNSIGNED_BYTE);

private:
    GLuint _id;
    uint32_t _width;
    uint32_t _height;
    GLenum _format;
};