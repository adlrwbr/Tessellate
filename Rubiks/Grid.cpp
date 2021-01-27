#include <iostream>
#include <math.h>

#include "Grid.h"
#include "AI.h"

Grid::Grid(size_t rows, size_t columns) {
    resize(rows, columns);
}

void Grid::resize(size_t rows, size_t columns) {
    
    static Color custom[54] = {
// front
   Color::GREEN,   Color::GREEN,   Color::GREEN,
   Color::GREEN,    Color::BLUE,    Color::BLUE,
     Color::RED,    Color::BLUE,    Color::BLUE,
// up
  Color::YELLOW,  Color::YELLOW,   Color::WHITE,
  Color::YELLOW,  Color::YELLOW,   Color::WHITE,
   Color::WHITE,   Color::WHITE,   Color::WHITE,
// back
     Color::RED,     Color::RED,     Color::RED,
   Color::GREEN,   Color::GREEN,    Color::BLUE,
   Color::GREEN,   Color::GREEN,  Color::ORANGE,
// down
   Color::WHITE,  Color::YELLOW,  Color::YELLOW,
   Color::WHITE,   Color::WHITE,   Color::WHITE,
  Color::YELLOW,  Color::YELLOW,  Color::YELLOW,
// left
  Color::ORANGE,  Color::ORANGE,  Color::ORANGE,
  Color::ORANGE,  Color::ORANGE,     Color::RED,
    Color::BLUE,     Color::RED,    Color::BLUE,
// right
     Color::RED,    Color::BLUE,    Color::BLUE,
  Color::ORANGE,     Color::RED,     Color::RED,
  Color::ORANGE,  Color::ORANGE,   Color::GREEN
    };

    // set rows and cols
    nRows = rows;
    nCols = columns;

    // destroy all cubes
    cubes.clear();

    // cubes.push_back(std::make_shared<Cube>());
    // cubes[0]->translate(glm::vec3(-10.0f, 0.0f, -5.0f));
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < columns; c++) {
            // create cube
            std::shared_ptr<Cube> cube = std::make_shared<Cube>();

            // translate
            cube->translate(calcCoords(r, c));

            // add to cubes
            cubes.push_back(cube);
        }
    }
}

void Grid::update(float deltatime) {
	for (std::shared_ptr<Cube>& cube : cubes)
		cube->update(deltatime);
}

void Grid::reset() {
	for (std::shared_ptr<Cube>& cubeptr : cubes) {
		cubeptr->reset();
	}
	std::cout << "Reset cubes" << std::endl;
}

void Grid::solveImage(BMPImage& bmp) {
    size_t width = bmp.getWidth();
    size_t height = bmp.getHeight();

    // resize grid
    resize(ceil(height / 3.0f), ceil(width / 3.0f));

    // allocate and fill memory for Color array
    Color* pixels = new Color[width * height];
    bmp.getPixels(pixels);

    for (int r = 0; r < width; r += 3) { // per row
        for (int c = 0; c < width; c += 3) { // per column
            Color paintpattern[9] = {
                pixels[(r + 0) * width + c], pixels[(r + 0) * width + c + 1], pixels[(r + 0) * width + c + 2],
                pixels[(r + 1) * width + c], pixels[(r + 1) * width + c + 1], pixels[(r + 1) * width + c + 2],
                pixels[(r + 2) * width + c], pixels[(r + 2) * width + c + 1], pixels[(r + 2) * width + c + 2]
            };
            AI ai(cubes[r / 3 * nCols + c / 3].get());
            ai.calculatePaint(paintpattern);
            ai.start();
        }
    }

    // release memory
    delete[] pixels;
}

void Grid::selectRelative(unsigned int dx, unsigned int dy) {
    // if none are selected, select first cube and return
    bool foundSelected = false;
    for (std::shared_ptr<Cube> c : cubes) {
        if (c->isSelected()) {
            foundSelected = true;
            break;
        }
    }
    if (!foundSelected) {
        selectAbsolute(0, 0);
        return; 
    }

    for (int r = 0; r < nRows; r++) {
        for (int c = 0; c < nCols; c++) {
            if (cubes[r * nCols + c]->isSelected()) {
                int newIndex = r * nCols + c + dx + dy * nCols; // index of newly selected cube
                if (newIndex >= 0 && newIndex < cubes.size()) { // if bounds check
                    // deselect old cube
                    cubes[r * nCols + c]->deselect();
                    // select new cube
                    cubes[newIndex]->select();
                    return;
                }
            }
        }
    }
}

std::shared_ptr<Cube> Grid::getSelected()
{
    for (std::shared_ptr<Cube> c : cubes) {
        if (c->isSelected())
            return c;
    }
    // if none were selected, select and return first cube
    selectAbsolute(0, 0);
    return cubes[0];
}

void Grid::selectAbsolute(unsigned int row, unsigned int column) {
    cubes[row * nCols + column]->select();
}

glm::vec3 Grid::calcCoords(unsigned int row, unsigned int column) {
    static const double EXTRA_GAP = 0; // adjust this to preference
    static const double MIN_GAP = 1.5 * sqrt(2) / 2.0; // the minimum gap between cubes in order for vertices to not touch each other when rotating
    static const double CUBE_WIDTH = 3.0;

    static const double CUBE_AND_GAP_WIDTH = (CUBE_WIDTH + MIN_GAP + EXTRA_GAP);

    double x = column * CUBE_AND_GAP_WIDTH - ((nCols-1) * CUBE_AND_GAP_WIDTH / 2.0);
    static const double y = 0;
    double z = row * CUBE_AND_GAP_WIDTH - ((nRows-1) * CUBE_AND_GAP_WIDTH / 2.0);

    return glm::vec3(x, y, z);
}
