#pragma once

#include <cstdint>
#include <assert.h>
#include <iostream>

bool logGLErrors(uint32_t line, const char *expression, const char *file); 

#if 0
#   define GL_CALL(x) x; if (logGLErrors(__LINE__, #x, __FILE__)) { assert(0 && "See previous OpenGL errors"); } 
#else
#   define GL_CALL(x) x;
#endif

#define GL_QUERY_INT_IV(param, id) { GLint v; glGetIntegeri_v(param, id, &v); std::cout << #param << "[" << id<< "]: " << v << std::endl; } 