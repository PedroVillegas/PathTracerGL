#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <string>
#include <unordered_map>

#include "utils.h"
#include "shader.h"


Shader::Shader(const char* vs_path, const char* fs_path)
{
    m_VSPath = vs_path;
    m_FSPath = fs_path;
    std::string vs = ParseShader(vs_path);
    std::string fs = ParseShader(fs_path);
    m_ID = CreateShader(vs, fs);
    std::cout << std::endl;
}

Shader::Shader(std::string vs_path, std::string fs_path)
{
    m_VSPath = vs_path.c_str();
    m_FSPath = fs_path.c_str();
    std::string vs = ParseShader(vs_path);
    std::string fs = ParseShader(fs_path);
    m_ID = CreateShader(vs, fs);
    std::cout << std::endl;
}

Shader::~Shader()
{
    glDeleteProgram(m_ID); 
    m_ID = 0;
}

void Shader::ReloadShader()
{
    std::string vs_path = std::string(m_VSPath);
    std::string fs_path = std::string(m_FSPath);
    std::cout << m_VSPath << std::endl;
    std::cout << vs_path << std::endl;
    std::string vs = ParseShader(vs_path);
    std::string fs = ParseShader(fs_path);
    uint32_t tempID = CreateShader(vs, fs);
    glDeleteProgram(m_ID); 
    m_ID = tempID;
    b_Reloaded = true;
    std::cout << std::endl;
}

std::string Shader::ParseShader(const std::string& filepath)
{
    std::ifstream stream(filepath); // opens file

    std::string line;
    std::stringstream ss;

    while (getline(stream, line))
    {
        if (line.find("#include") != std::string::npos && line.rfind("//", 0) == std::string::npos)
        {
            uint32_t start = (uint32_t) line.find_first_of("<") + 1;
            uint32_t end = (uint32_t) line.find_first_of(">");
            std::string includeFile = line.substr(start, end - start);
            ss << ParseShader(PATH_TO_SHADERS + includeFile);
        }
        else
        {
            ss << line << "\n";
        }
    }

    return ss.str();
}

uint32_t Shader::CompileShader(uint32_t type, const std::string& source)
{
    uint32_t id = glCreateShader(type); 
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, NULL); 
    glCompileShader(id); 

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result); 
    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length); 
        char* message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message); 
        std::cout << "\033[1;31m[ERROR]\033[0;37m " << (type == GL_VERTEX_SHADER ? "Vertex" : "Frag") << " Shader Did Not Compile - ";
        std::cout << (type == GL_VERTEX_SHADER ? m_VSPath : m_FSPath) << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id); 

        return 0;
    }

    std::cout << "\033[1;32m[SUCCESS]\033[0;37m " << (type == GL_VERTEX_SHADER ? "Vertex" : "Frag") << " Shader Compiled - ";
    std::cout << (type == GL_VERTEX_SHADER ? m_VSPath : m_FSPath) << std::endl;

    return id;
}

uint32_t Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    // create program for shaders to link to
    uint32_t programID = glCreateProgram(); 
    uint32_t vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    uint32_t fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    if (!vs || !fs) 
        return 0;

    // link shaders into one program
    glAttachShader(programID, vs); 
    glAttachShader(programID, fs); 
    glLinkProgram(programID); 
    glValidateProgram(programID); 

    int result;
    glGetProgramiv(programID, GL_LINK_STATUS, &result); 
    if (result == GL_FALSE)
    {
        int length;
        glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &length); 
        char* message = (char*)alloca(length * sizeof(char));
        glGetProgramInfoLog(programID, length, &length, message); 
        std::cout << "Failed to link shaders :: " << message << std::endl;
        glDeleteProgram(programID); 

        return 0;
    }

    // std::cout << "Successfully linked shaders" << std::endl; 

    glDeleteShader(vs); 
    glDeleteShader(fs); 

    return programID;
}

void Shader::Bind() const
{
    glUseProgram(m_ID);
}

void Shader::Unbind() const
{
    glUseProgram(0);
}

void Shader::SetUniformInt(const std::string& name, int val)
{
    glUniform1i(GetUniformLocation(name), val);
}

void Shader::SetUniformFloat(const std::string& name, float val)
{
    glUniform1f(GetUniformLocation(name), val);
}

void Shader::SetUniformVec2(const std::string& name, float val0, float val1)
{
    glUniform2f(GetUniformLocation(name), val0, val1);
}

void Shader::SetUniformVec3(const std::string& name, float val0, float val1, float val2)
{
    glUniform3f(GetUniformLocation(name), val0, val1, val2);
}

void Shader::SetUniformVec4(const std::string& name, float val0, float val1, float val2, float val3)
{
    glUniform4f(GetUniformLocation(name), val0, val1, val2, val3);
}

void Shader::SetUniformMat4(const std::string& name, const glm::mat4& matrix)
{
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
}
   
void Shader::SetUBO(const std::string& name, uint32_t bind)
{
    uint32_t block = glGetUniformBlockIndex(m_ID, name.c_str());
    glUniformBlockBinding(m_ID, block, bind);
}

uint32_t Shader::GetUniformLocation(const std::string &name)
{
    if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
        return m_UniformLocationCache[name];

    uint32_t location = glGetUniformLocation(m_ID, name.c_str());
    if (location == -1)
        std::cout << "[Warning] Uniform '" << name << "' doesn't exist!" << std::endl;
    
    m_UniformLocationCache[name] = location;

    return location;
}
