#include <cmath>
#include <map>
#include <stdexcept>
#include <string>
#include <iostream>

#include "BMPImage.hpp"


BMPImage::BMPImage(const char* const filepath) {

	std::ifstream inFile;

	inFile.open(filepath, std::ios::binary | std::ios::in);

	// check state
	if (!inFile) {
		throw std::runtime_error("cannot find image file");
	}

	// read file header
	inFile.read(header, 54);

	width = *(int*)&header[18];
	height = *(int*)&header[22];
	
	// determine how many bytes are in a row (with padding)
	size_t byteWidthWPadding( (width * 3 + 3) & (~3));
	size_t nPaddingBytes = byteWidthWPadding - width * 3; // byte width w/ padding minus byte width

	// allocate space for data
	dataSize = 3 * width * height; // 3 bytes per pixel. no room for padding.
	data = new char[dataSize];

	// read file into data excluding padding bytes
	unsigned int nPaddingBytesSeen = 0; // how many padding bytes have been thrown away so far
	char shitByte; // used to read worthless padding bytes into
	for (int i = 0; i < dataSize + nPaddingBytesSeen; i++) { // for each byte
		if (i % byteWidthWPadding >= width * 3) { // if this byte is a padding byte
			// don't store it
			inFile.read(&shitByte, 1);
			nPaddingBytesSeen++;
		} else {
			inFile.read(data + i - nPaddingBytesSeen, 1);
		}
	}

	// reflect image vertically
	char temp = 0;
	for (int r = 0; r < height / 2; r++) { // for each row of pixels
		for (int c = 0; c < width * 3; c++) { // for each byte
			temp = data[(height - 1 - r) * width * 3 + c];
			data[(height - 1 - r) * width * 3 + c] = data[r * width * 3 + c];
			data[r * width * 3 + c] = temp;
		}
	}

	inFile.close();
}

BMPImage::~BMPImage() {
	delete[] data;
}

size_t BMPImage::getWidth() const {
	return width;
}

size_t BMPImage::getHeight() const
{
	return height;
}

void BMPImage::getPixels(Color output[]) const {
	for (int i = 0; i < dataSize; i += 3) { // for each pixel
		// extract rgb values
		unsigned char b = data[i + 0];
		unsigned char g = data[i + 1];
		unsigned char r = data[i + 2]; 
		glm::vec3 pixel(r, g, b);
		output[i / 3] = getClosestColor(pixel);
	}
}

Color getClosestColor(glm::vec3 color)
{
	static const glm::vec3 const RGBVALUES[6] = {
		glm::vec3(255, 0, 0), // RED
		glm::vec3(255, 165, 0), // ORANGE
		glm::vec3(255, 255, 0), // YELLOW
		glm::vec3(0, 255, 0), // GREEN
		glm::vec3(0, 0, 255), // BLUE
		glm::vec3(255, 255, 255) }; // WHITE
	Color closestC = Color::RED;
	int closestD = INT_MAX;

	for (int i = 0; i < 6; i++) { // for each color
		int distance = getEuchlidianDistance(color, RGBVALUES[i]);
		if (distance < closestD) {
			closestD = distance;
			closestC = static_cast<Color>(i);
		}
	}
	return closestC;
}

int getEuchlidianDistance(glm::vec3 color1, glm::vec3 color2)
{
	int dr = color1.r - color2.r;
	int dg = color1.g - color2.g;
	int db = color1.b - color2.b;
	return sqrt(dr * dr + dg * dg + db * db);
}
