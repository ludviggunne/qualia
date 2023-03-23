
#include "buffer.h"


Buffer::Buffer(GLenum target) {

    set_target(target);
    GL_CALL( glCreateBuffers(1, &_id) );
}

Buffer::~Buffer() {

    GL_CALL( glDeleteBuffers(1, &_id) );
}

void Buffer::bind() const {

    GL_CALL( glBindBuffer(_target, _id) );
}

void Buffer::set_target(GLenum target) {

    _target = target;
}