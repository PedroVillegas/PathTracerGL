#include "utils.h"

void GenerateAndCreateVAO(std::vector<float> vertices, std::vector<uint32_t> indices,
        uint32_t &VAO, uint32_t &VBO, uint32_t &IBO)
{
    glGenVertexArrays  (1, &VAO); 
    glGenBuffers       (1, &VBO); 
    glGenBuffers       (1, &IBO); 
    
    glBindVertexArray  (VAO); 
    glBindBuffer       (GL_ARRAY_BUFFER, VBO); 
    glBufferData       (GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW); 
    
    glBindBuffer       (GL_ELEMENT_ARRAY_BUFFER, IBO); 
    glBufferData       (GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indices.size(), indices.data(), GL_STATIC_DRAW);
}