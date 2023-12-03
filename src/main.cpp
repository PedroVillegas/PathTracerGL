#include "application.h"


void SetupQuad(uint32_t& VAO, uint32_t& VBO, uint32_t& IBO);

int main(void) 
{
    Application app = Application("Path Tracing", 1280, 720);
    app.Run();

    return 0;
}

void SetupQuad(uint32_t& VAO, uint32_t& VBO, uint32_t& IBO)
{
    float vertices[] = 
    {
        // pos                 // col
        -1.0f, -1.0f, 0.0f,    0.25f, 0.52f, 0.96f,
         1.0f, -1.0f, 0.0f,    0.86f, 0.27f, 0.22f,
         1.0f,  1.0f, 0.0f,    0.96f, 0.71f,  0.0f,
        -1.0f,  1.0f, 0.0f,    0.06f, 0.62f, 0.35f
    };

    uint32_t indices[] = 
    {
        0, 1, 2,
        2, 3, 0
    };
 
    glGenVertexArrays  (1, &VAO); 
    glGenBuffers       (1, &VBO); 
    glGenBuffers       (1, &IBO); 
    
    glBindVertexArray  (VAO); 
    glBindBuffer       (GL_ARRAY_BUFFER, VBO); 
    glBufferData       (GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); 
    
    glBindBuffer       (GL_ELEMENT_ARRAY_BUFFER, IBO); 
    glBufferData       (GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); 

    // position attrib
    glVertexAttribPointer      (0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); 
    glEnableVertexAttribArray  (0); 

    // colour attrib  
    glVertexAttribPointer      (1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3* sizeof(float))); 
    glEnableVertexAttribArray  (1); 
}
