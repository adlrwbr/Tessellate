#pragma once

#include <glm/glm.hpp>

#include "Square.hpp"

enum class FaceType {
	FRONT, UP, BACK, DOWN, LEFT, RIGHT
};

class Face {
public:
	Square squares[9]; // top left, top center, top right, center left, center center, center right, bottom left, bottom center, bottom right
	float rotationAngle;
public:
	Face(); // default constructor. To-do: remove this and figure out the cube's member initializer list
	Face(Color squares[9]);
	Color getColorAt(unsigned int index) const;
	void setColorAt(unsigned int index, Color c);
	void getVerticesInColumn(unsigned int col, glm::vec3* output[18]) const; // inserts the pointers to the vectors that are in a specified column into the output array
	void getVerticesInRow(unsigned int row, glm::vec3* output[18]) const; // inserts the pointers to the vectors that are in a specified row into the output array
	void getVerticesAt(unsigned int index, glm::vec3* output[6]) const; // outputs a square's vertices
	void print() const;
};