#pragma once

#include <glad/glad.h>
#include <signal.h>

#define DEBUG 1

#ifdef DEBUG
#   define ASSERT(x) if((x) != 0) raise(SIGTRAP);
#   define GLCall ASSERT(debugGLCall(__FILE__, __LINE__))
#else
#   define GLCall
#endif

GLenum debugGLCall(const char* file, uint32_t line);