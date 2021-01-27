#pragma once

#include <glm/glm.hpp>

enum class Color {
	RED, ORANGE, YELLOW, GREEN, BLUE, WHITE
};

class Square {
public:
	mutable glm::vec3 vertices[6]; // top left triangle then bottom right triangle of each square // for triangles, I'm going to start at the right angle and go clockwise.
	Color color;
public:
	Square(Color color);
};