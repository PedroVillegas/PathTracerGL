#include <iostream>

#include "consoleLogger.h"

GLenum debugGLCall(const char* file, uint32_t line)
{
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR)
    {
        std::string error;
        switch (err)
        {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        std::cout << "\e[1;31m[ERROR]\e[0;37m " << error << " :: " << file << " :: " << line << std::endl;
        return err;
    }
    return 0;
}
