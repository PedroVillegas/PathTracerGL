#include "consoleLogger.h"

void debugGLCall(const char* file, uint line)
{
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR){
        std::cout << "[ERROR] " << err << " :: " << file << " :: " << line << std::endl;
        err = 0;
    }  
}
