#pragma once

#include <string>
#include <filesystem>
#include <iostream>
#include "stb/stb_image.h"


class HDRI
{
public:
	HDRI()
		: width(0)
		, height(0)
		, totalSum(0.0f)
		, data(nullptr)
		, cdf(nullptr)
	{} ;
	~HDRI()
	{
		stbi_image_free(data);
		stbi_image_free(cdf);
	};

	int width;
	int height;
	float totalSum;
	float* data;
	float* cdf;

	void LoadHDRI(std::string filepath);
};
