#pragma once

#include <string>

#include "glad/glad.h"
#include "glm_config.hpp"
#include "gl.h"

std::string loadFile(const std::string &fileName);

class Shader {

public:
    Shader(GLenum type);
    ~Shader();

    // Returns empty string on succesfull compilation, otherwise error message
    std::string compile(const std::string &src) const;

private:
    GLenum _type;
    GLint _id;

    friend class Program;
};

class Program {

public:
    Program();
    ~Program();

    void attach(const Shader &shader) const;

    // Returns empty string on succesfull compilation, otherwise error message
    std::string link() const; 

    void use() const;

    GLint queryUniformLocation(const char *name) const; 

    template<typename T>
    void setUniform(const T &value, GLint location); 

private:
    GLuint _id;
};

template<>
inline void Program::setUniform<int32_t>(const int32_t &value, GLint location) {

    GL_CALL( glUniform1i(location, value) );
}

template<>
inline void Program::setUniform<float>(const float &value, GLint location) {

    GL_CALL( glUniform1f(location, value) );
}

template<>
inline void Program::setUniform<glm::vec2>(const glm::vec2 &value, GLint location) {

    GL_CALL( glUniform2f(location, value.x, value.y) );
}

template<>
inline void Program::setUniform<glm::vec4>(const glm::vec4 &value, GLint location) {

    GL_CALL( glUniform4f(location, value.x, value.y, value.z, value.w) );
}

template<>
inline void Program::setUniform<glm::vec3>(const glm::vec3 &value, GLint location) {

    GL_CALL( glUniform3f(location, value.x, value.y, value.z) );
}

template<>
inline void Program::setUniform<glm::mat4>(const glm::mat4 &value, GLint location) {

    GL_CALL( glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value)) );
}

template<>
inline void Program::setUniform<glm::ivec2>(const glm::ivec2 &value, GLint location) {

    GL_CALL( glUniform2i(location, value.x, value.y) );
}