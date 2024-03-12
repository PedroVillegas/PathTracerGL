#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include <iostream>
#include "lodepng.h"

class Texture
{
public:
	Texture()
		: width(0)
		, height(0)
	{};

	void LoadTexture(std::string filepath);

	int width;
	int height;
	std::vector<uint8_t> data;
};