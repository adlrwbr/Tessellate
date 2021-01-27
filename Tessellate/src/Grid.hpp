#pragma once
#include <memory>

#include "Cube.hpp"
#include "BMPImage.hpp"

class Grid {
public:
	size_t nRows, nCols;
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

	/* selecs a cube orthagonal to the current selection. does nothing if no cubes are selected or grid bounds are hit */
	void selectRelative(unsigned int dx, unsigned int dy);

	/* return the selected cube, or the first if none */
	std::shared_ptr<Cube> getSelected();
	
	/* deselects all cubes in grid */
	void selectAbsolute(unsigned int row, unsigned int column);

private:
	/* calculates the coordinates of a cube in grid */
	glm::vec3 calcCoords(unsigned int row, unsigned int column);
};