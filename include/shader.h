#pragma once

#include <glad/glad.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <string>
#include <unordered_map>

class Shader
{
private:
    unsigned int m_ID;
    std::unordered_map<std::string, unsigned int> m_UniformLocationCache;
    
public:
    // constructor reads and builds shader
    Shader(const char* vs_path, const char* fs_path);
    ~Shader();
    // use/activate the shader
    void Bind() const;
    void Unbind() const;
    // setters
    void SetUniform4f(const std::string& name, float f0, float f1, float f2, float f3);
    void SetUniform2f(const std::string& name, float f0, float f2);

private:
    std::string ParseShader(const std::string& filepath);
    unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader);
    unsigned int CompileShader(unsigned int type, const std::string& source);
    unsigned int GetUniformLocation(const std::string& name);
};
