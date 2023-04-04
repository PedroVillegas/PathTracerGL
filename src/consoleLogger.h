#pragma once

#include <glad/glad.h>
#include <iostream>

#define DEBUG_MODE 0

#if DEBUG_MODE == 1
#define GLCall debugGLCall(__FILE__, __LINE__)
#else
#define GLCall
#endif

void debugGLCall(const char* file, uint line);