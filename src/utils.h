#pragma once

#include <cstdint>
#include <vector>
#include <glad/glad.h>
#include <glfw/include/GLFW/glfw3.h>
#include <string>

#define PATH_TO_SHADERS std::string("../../src/shaders/")
#define PROJECT_PATH std::string("../../")
#define PATH_TO_HDR std::string("../../assets/hdr/")

//"Jodie-Reinhard\0ACES film\0ACES fitted\0Tony McMapface\0AgX Punchy\0"
enum { JODIE_REINHARD = 0, ACES_FILM, ACES_FITTED, TONY_MCMAPFACE, AGX_PUNCHY };

struct ApplicationSettings
{
    int tonemap = TONY_MCMAPFACE;
    bool enableVsync = false;
    bool enableBVH = false;
    bool enableDebugBVHVisualisation = false;
    bool enableGui = true;
    bool enableCrosshair = true;
    bool enableBlueNoise = true;
};

void GenerateAndCreateVAO(std::vector<float> vertices, std::vector<uint32_t> indices,
        uint32_t &VAO, uint32_t &VBO, uint32_t &IBO);