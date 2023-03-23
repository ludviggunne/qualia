
#include "shaders.h"
#include <fstream>

Shader::Shader(GLenum type) : _type(type) {

    GL_CALL( _id = glCreateShader(type) );
}

Shader::~Shader() { glDeleteShader(_id); }

std::string Shader::compile(const std::string &src) const {

    auto c_str = src.c_str();
    GL_CALL( glShaderSource(_id, 1, &c_str, NULL) );
    GL_CALL( glCompileShader(_id) );

    char log[512];
    int status;

    GL_CALL( glGetShaderiv(_id, GL_COMPILE_STATUS, &status) );
    if (!status) {

        GL_CALL( glGetShaderInfoLog(_id, 512, NULL, log) );
        return std::string(log);
    }

    return std::string();
}

Program::Program() {

    GL_CALL( _id = glCreateProgram() );
}

Program::~Program() { GL_CALL( glDeleteProgram(_id) ); }

void Program::attach(const Shader &shader) const {

    GL_CALL( glAttachShader(_id, shader._id) );
}

std::string Program::link() const {

    GL_CALL( glLinkProgram(_id) );

    char log[512];
    int status;

    GL_CALL( glGetProgramiv(_id, GL_LINK_STATUS, &status) );
    if (!status) {

        GL_CALL( glGetProgramInfoLog(_id, 512, NULL, log) );
        return std::string(log);
    }

    return std::string();
}

void Program::use() const {

    GL_CALL( glUseProgram(_id) );
}

GLint Program::queryUniformLocation(const char *name) const {

    GLint location;
    GL_CALL( location = glGetUniformLocation(_id, name) );
    return location;
}

std::string loadFile(const std::string &fileName) {

    std::ifstream stream(fileName);
    if (stream.fail()) return {};

    return std::string(
        std::istreambuf_iterator<char>(stream),
        std::istreambuf_iterator<char>()
    );
}