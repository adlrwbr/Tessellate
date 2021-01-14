#pragma once
#include <memory>

#include "Cube.h"
#include "BMPImage.h"

class Grid {
private:
	size_t nRows, nCols;
public:
	std::vector<std::shared_ptr<Cube>> cubes;
public:
	Grid(size_t rows, size_t cols);
	
	/* destroy all cubes and create more to fulfill size parameters */
	void resize(size_t rows, size_t cols);

	/* updates the cubes' animations when supplied with deltatime */
	void update(float deltatime);

	/* reset all cubes and queues to their original states */
	void reset();

	/** solve grid for supplied image
	* 
	*/
	void solveImage(BMPImage& bmp);

private:
	/* calculates the coordinates of a cube in grid */
	glm::vec3 calcCoords(unsigned int row, unsigned int column);
};