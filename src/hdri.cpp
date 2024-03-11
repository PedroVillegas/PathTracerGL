#include "hdri.h"


float Luminance(float r, float g, float b)
{
    return 0.212671f * r + 0.715160f * g + 0.072169f * b;
}

void HDRI::LoadHDRI(std::string filepath)
{
    std::filesystem::directory_entry file(filepath);
    if (!file.exists()) std::cout << filepath << " does not exist!" << std::endl;

    if (data) stbi_image_free(data);
    if (cdf)  stbi_image_free(cdf);

    std::cout << "Loading HDRI: " << filepath << "..." << std::endl;
    data = stbi_loadf(filepath.c_str(), &width, &height, NULL, 4);
    if (data == nullptr)
        std::cout << "Unable to load " << filepath << std::endl;
    else
        std::cout << "Environment map loaded!" << std::endl;
    
}