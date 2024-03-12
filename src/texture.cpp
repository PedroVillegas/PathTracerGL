#include "texture.h"


void Texture::LoadTexture(std::string filepath)
{
    std::cout << "Loading Texture: " << filepath << "..." << std::endl;
	std::filesystem::directory_entry file(filepath);
    if (!file.exists()) std::cout << filepath << " does not exist!" << std::endl;
    uint32_t w = width;
    uint32_t h = height;
    auto error = lodepng::decode(data, w, h, filepath);
    
    if (error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
    width = w;
    height = h;
    if (data.empty() == true)
        std::cout << "Unable to load " << filepath << std::endl;
    else
        std::cout << "Texture loaded!" << std::endl;
}
