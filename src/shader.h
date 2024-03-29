#pragma once

#include <unordered_map>
#include <string>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include "utils.h"

class Shader
{
public:
    // Constructor reads and builds shader
    Shader(std::string vs_path, std::string fs_path);
    ~Shader();

    void Bind() const;
    void Unbind() const;
    void ReloadShader();
    uint32_t GetID() { return m_ID; }

    void SetUniformInt(const std::string& name, int val);
    void SetUniformFloat(const std::string& name, float val);
    void SetUniformVec2(const std::string& name, float val0, float val1);
    void SetUniformVec3(const std::string& name, float val0, float val1, float val2);
    void SetUniformVec4(const std::string& name, float val0, float val1, float val2, float val3);
    void SetUniformMat4(const std::string& name, const glm::mat4& matrix);
    void SetUBO(const std::string& name, uint32_t bind);

    bool hasReloaded;

private:
    std::string ParseShader(const std::string& filepath);
    uint32_t CreateShader(const std::string& vertexShader, const std::string& fragmentShader);
    uint32_t CompileShader(uint32_t type, const std::string& source);
    uint32_t GetUniformLocation(const std::string& name);

    uint32_t m_ID;
    std::unordered_map<std::string, uint32_t> m_UniformLocationCache;
    std::string m_FSPath;
    std::string m_VSPath;
};
