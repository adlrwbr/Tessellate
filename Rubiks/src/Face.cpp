#include <iostream>
#include <iomanip>

#include "Face.hpp"

Face::Face() 
	: squares{ Square(Color::WHITE), Square(Color::WHITE), Square(Color::WHITE), Square(Color::WHITE), Square(Color::WHITE), Square(Color::WHITE), Square(Color::WHITE), Square(Color::WHITE), Square(Color::WHITE) }, rotationAngle(0) {}

Face::Face(Color squares[9])
	: squares{ squares[0], squares[1], squares[2], squares[3], squares[4], squares[5], squares[6], squares[7], squares[8] }, rotationAngle(0) {}

Color Face::getColorAt(unsigned int index) const {
	return squares[index].color;
}

void Face::setColorAt(unsigned int index, Color c) {
	squares[index].color = c;
}

void Face::getVerticesInColumn(unsigned int col, glm::vec3* output[18]) const {
	for (int i = col; i < 9; i += 3)
		getVerticesAt(i, output + (i / 3) * 6);
}

void Face::getVerticesInRow(unsigned int row, glm::vec3* output[18]) const {
	for (int i = 0; i < 3; i++)
		getVerticesAt(row * 3 + i, output + i * 6);
}

void Face::getVerticesAt(unsigned int index, glm::vec3* output[6]) const {
	/* Why couldn't I originally declare this method as const? Good question, future me.
	   "output" is a glm::vec3* array. However, when the method is const, "this" (a pointer) becomes a const pointer, so that I cannot modify its members.
	   I can't assign "squares.vertices" (which is now const) to "output", which is not

	   Note: I made "squares.vertices" mutable in order to solve this problem. "sometimes mutable members are used so that methods can return a reference to a result"
	*/
	for (int i = 0; i < 6; i++)
		output[i] = &squares[index].vertices[i];
}

void Face::print() const {
	for (int i = 0; i < 9; i++) {
		const char* c;
		switch (this->getColorAt(i)) {
		case Color::RED:
			c = "Color::RED";
			break;
		case Color::ORANGE:
			c = "Color::ORANGE";
			break;
		case Color::YELLOW:
			c = "Color::YELLOW";
			break;
		case Color::GREEN:
			c = "Color::GREEN";
			break;
		case Color::BLUE:
			c = "Color::BLUE";
			break;
		case Color::WHITE:
			c = "Color::WHITE";
			break;
		default:
			return;
		}
		std::cout << std::setw(15) << std::right << c << ",";

		if (i != 0 && (i+1) % 3 == 0) // print newline every row
			std::cout << "\n";
	}
}
