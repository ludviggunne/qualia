
#include "texture.h"

#include "gl.h"


Texture::Texture(uint32_t width, uint32_t height, GLenum format)
    : _width(width), _height(height), _format(format)
{

    GL_CALL( glGenTextures(1, &_id) );
}

Texture::~Texture() {

    GL_CALL( glDeleteTextures(1, &_id) );
}

void Texture::bind() const {

    GL_CALL( glBindTexture(GL_TEXTURE_2D, _id) );
}

void Texture::bind_to_unit(GLuint unit) const {

    GL_CALL( glActiveTexture(GL_TEXTURE0 + unit) );
    bind();
}

void Texture::bind_as_image(GLuint unit) const {

    bind_to_unit(0);
    GL_CALL( glBindImageTexture(0, _id, 0, GL_FALSE, 0, GL_READ_ONLY, _format) );
}

void Texture::set_parameter(GLenum param, GLenum value) const {

    bind();
    GL_CALL( glTexParameteri(GL_TEXTURE_2D, param, value) );
}
//
//
//
uint32_t Texture::width() const { return _width; }
uint32_t Texture::height() const { return _height; }

void Texture::load_image(const void *image_data, GLenum format, GLenum data_type) {

    bind();
    bind_to_unit(0);
    GL_CALL( glTexImage2D(GL_TEXTURE_2D, 0, _format, _width, _height, 0, format, data_type, image_data) );
}


void Texture::get_image(void *image_data, GLenum format, GLenum data_type) {

    bind();

    glGetTexImage(GL_TEXTURE_2D, 0, format, data_type, image_data);
}