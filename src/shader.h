#pragma once

#include <unordered_map>
#include <string>
#include <glm/glm.hpp>

class Shader
{
private:
    uint32_t m_ID = 0;
    std::unordered_map<std::string, uint32_t> m_UniformLocationCache;
    const char* m_FSPath;
    const char* m_VSPath;
public:
    // constructor reads and builds shader
    Shader(const char* vs_path, const char* fs_path);
    ~Shader();
    // use/activate the shader
    void Bind() const;
    void Unbind() const;

    void ReloadShader();

    uint32_t GetID() { return m_ID; }
    // setters
    void SetUniformInt(const std::string& name, int val);
    void SetUniformFloat(const std::string& name, float val);
    void SetUniformVec2(const std::string& name, float val0, float val1);
    void SetUniformVec3(const std::string& name, float val0, float val1, float val2);
    void SetUniformVec4(const std::string& name, float val0, float val1, float val2, float val3);
    void SetUniformMat4(const std::string& name, const glm::mat4& matrix);
private:
    std::string ParseShader(const std::string& filepath);
    uint32_t CreateShader(const std::string& vertexShader, const std::string& fragmentShader);
    uint32_t CompileShader(uint32_t type, const std::string& source);
    uint32_t GetUniformLocation(const std::string& name);
};
