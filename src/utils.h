#pragma once

#include <cstdint>
#include <vector>
#include <glad/glad.h>

struct ApplicationSettings
{
    bool vsync = true;
    int tonemap = 3;
    bool BVHEnabled = false;
    bool debugBVHVisualisation = false;
};

void GenerateAndCreateVAO(std::vector<float> vertices, std::vector<uint32_t> indices,
        uint32_t &VAO, uint32_t &VBO, uint32_t &IBO);