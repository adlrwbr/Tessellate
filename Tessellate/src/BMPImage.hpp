#pragma once

#include <fstream>

#include "Square.hpp"

static Color getClosestColor(glm::vec3 color);

/* return the euchlidian distance between two colors */
static int getEuchlidianDistance(glm::vec3 color1, glm::vec3 color2);

class BMPImage {
private:
	size_t width, height;
	char header[54]; // file metadata 54 bytes long
	char* data;
	size_t dataSize;
public:

	BMPImage(const char* const filepath);
	~BMPImage();

	size_t getWidth() const;
	size_t getHeight() const;
	void getPixels(Color output[]) const;

};