#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <string>
#include <unordered_map>
#include "consoleLogger.h"

class Shader
{
private:
    uint m_ID = 0;
    std::unordered_map<std::string, uint> m_UniformLocationCache;
    
public:
    // constructor reads and builds shader
    Shader(const char* vs_path, const char* fs_path);
    ~Shader();
    // use/activate the shader
    void Bind() const;
    void Unbind() const;

    inline uint GetID() { return m_ID; }
    // setters
    void SetUniformInt(const std::string& name, int val);
    void SetUniformFloat(const std::string& name, float val);
    void SetUniformVec2(const std::string& name, float val0, float val1);
    void SetUniformVec3(const std::string& name, float val0, float val1, float val2);
    void SetUniformVec4(const std::string& name, float val0, float val1, float val2, float val3);
    void SetUniformMat4(const std::string& name, const glm::mat4& matrix);

private:
    std::string ParseShader(const std::string& filepath);
    uint CreateShader(const std::string& vertexShader, const std::string& fragmentShader);
    uint CompileShader(uint type, const std::string& source);
    uint GetUniformLocation(const std::string& name);
};
