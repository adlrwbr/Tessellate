#include <iostream>
#include <algorithm>
#include <cmath>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_access.hpp>

#include "Cube.h"


Cube::Cube() 
	: model(1.0f), solveSpeed(2.6f), position(0, 0, 0), selected(0) {
	
	// standard cube layout
	static Color standard[54] = {
		Color::GREEN, Color::GREEN, Color::GREEN,
		Color::GREEN, Color::GREEN, Color::GREEN,
		Color::GREEN, Color::GREEN, Color::GREEN,

		Color::WHITE, Color::WHITE, Color::WHITE,
		Color::WHITE, Color::WHITE, Color::WHITE,
		Color::WHITE, Color::WHITE, Color::WHITE,

		Color::BLUE, Color::BLUE, Color::BLUE,
		Color::BLUE, Color::BLUE, Color::BLUE,
		Color::BLUE, Color::BLUE, Color::BLUE,

		Color::YELLOW, Color::YELLOW, Color::YELLOW,
		Color::YELLOW, Color::YELLOW, Color::YELLOW,
		Color::YELLOW, Color::YELLOW, Color::YELLOW,

		Color::ORANGE, Color::ORANGE, Color::ORANGE,
		Color::ORANGE, Color::ORANGE, Color::ORANGE,
		Color::ORANGE, Color::ORANGE, Color::ORANGE,

		Color::RED, Color::RED, Color::RED,
		Color::RED, Color::RED, Color::RED,
		Color::RED, Color::RED, Color::RED
	};

	// copy initial colors
	for (int i = 0; i < 54; i++)
		initialSqColors[i] = standard[i];

	// create faces
	for (int i = 0; i < 6; i++) { // for each face
		Color colors[9] = { standard[i * 9 + 0], standard[i * 9 + 1], standard[i * 9 + 2], standard[i * 9 + 3], standard[i * 9 + 4], standard[i * 9 + 5], standard[i * 9 + 6], standard[i * 9 + 7], standard[i * 9 + 8], };
		faces[i] = Face(colors);
	}

	generateVertices();
}

Cube::Cube(Color squares[54])
	: model(1.0f), solveSpeed(2.6f), position(0, 0, 0), selected(0) {

	// copy initial colors
	for (int i = 0; i < 54; i++)
		initialSqColors[i] = squares[i];

	// create faces
	for (int i = 0; i < 6; i++) { // for each face
		Color colors[9] = { squares[i * 9 + 0], squares[i * 9 + 1], squares[i * 9 + 2], squares[i * 9 + 3], squares[i * 9 + 4], squares[i * 9 + 5], squares[i * 9 + 6], squares[i * 9 + 7], squares[i * 9 + 8], };
		faces[i] = Face(colors);
	}
	generateVertices();
}

Cube::Cube(Cube* other) 
	: model(other->model), solveSpeed(other->solveSpeed), position(0, 0, 0), selected(other->selected) {
	
	// copy initial colors
	for (int i = 0; i < 54; i++)
		initialSqColors[i] = other->initialSqColors[i];

	// copy face colors
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 9; j++) {
			faces[i].setColorAt(j, other->faces[i].getColorAt(j));
		}
	}
}

void Cube::update(float deltatime) {
	// update pending instruction
	if (queue.size() > 0) {
		if (perform(queue[0].get(), deltatime)) // if rotation was completed
			queue.erase(queue.begin()); // remove the current instruction
	}
}

void Cube::scramble() {
	FaceType randFace;
	bool randDirection;
	for (int i = 0; i < 1000; i++) {
		randFace = static_cast<FaceType>(rand() % 6);
		randDirection = rand() % 2;
		rotateColors(randFace, randDirection); // to-do: replace with instructions
	}
}

// to-do: add support for rotating more than 90 degrees, maybe? or just assert that radians cannot be more than 90
bool Cube::rotate(FaceType face, float radians, bool clockwise) {
	// update the face's angle
	faces[static_cast<int>(face)].rotationAngle += radians;

	// rotate vertices
	rotateVertices(face, radians, clockwise);

	// if face's vertices have been rotated 90 degrees (half pi), undo the rotation, snap vertices, rotate the colors, and return true
	if (faces[static_cast<int>(face)].rotationAngle >= glm::half_pi<float>()) {
		rotateVertices(face, glm::three_over_two_pi<float>(), clockwise);
		snapVertices();
		rotateColors(face, clockwise);
		faces[static_cast<int>(face)].rotationAngle = 0.0f;
		// std::cout << "face #" << static_cast<int>(face) << " has been turned" << std::endl;
		// this->print();
		return true;
	} else {
		return false;
	}
}

// to-do: if I rotate -half_pi radians over a positive axis, this definitely gets jank
bool Cube::rotate(glm::vec3 axis, float radians) {
	// update a face's angle
	FaceType faceToUpdate;
	if (std::abs(axis.x)) { // if x axis
		faceToUpdate = FaceType::LEFT;
	} else if (std::abs(axis.y)) { // if y axis
		faceToUpdate = FaceType::UP;
	} else if (std::abs(axis.z)) { // if z axis (note: not 'else' because the axis could be (0,0,0) 
		faceToUpdate = FaceType::FRONT;
	} else { // if axis is invalid
		return false;
	}

	faces[static_cast<int>(faceToUpdate)].rotationAngle += radians;

	// rotate vertices
	rotateVertices(axis, radians);

	// if face's vertices have been rotated 90 degrees (half pi), undo the rotation, snap vertices, rotate the colors, and return true
	if (faces[static_cast<int>(faceToUpdate)].rotationAngle >= glm::half_pi<float>()) {
		rotateVertices(axis, glm::three_over_two_pi<float>());
		snapVertices();
		rotateColors(axis);
		faces[static_cast<int>(faceToUpdate)].rotationAngle = 0.0f;
		return true;
	} else {
		return false;
	}

}

void Cube::print() const {
	for (int i = 0; i < 6; i++) {
		const char* face;
		switch (i) {
		case 0:
			face = "front";
			break;
		case 1:
			face = "up";
			break;
		case 2:
			face = "back";
			break;
		case 3:
			face = "down";
			break;
		case 4:
			face = "left";
			break;
		case 5:
			face = "right";
			break;
		default:
			return;
		}
		std::cout << "// " << face << "\n";
		faces[i].print();
	}
}

void Cube::reset() {
	// clear queue
	queue.clear();
	// generate new vertices
	generateVertices();
	// revert to original colors
	for (int i = 0; i < 6; i++) { // for each face
		for (int j = 0; j < 9; j++) // for each square
			faces[i].setColorAt(j, initialSqColors[i * 9 + j]);
	}
}

/* returns a pointer to the specified facetype */

Face* Cube::getFace(FaceType type)
{
	return &faces[static_cast<int>(type)];
}

void Cube::getVertexData(GLfloat vertex_buffer_data[]) const {
	for (int i = 0; i < 6; i++) { // for each face
		for (int j = 0; j < 9; j++) { // for each square
			for (int k = 0; k < 6; k++) { // for each vertex
				vertex_buffer_data[i * 9 * 6 * 3 + j * 6 * 3 + k * 3 + 0] = faces[i].squares[j].vertices[k].x;
				vertex_buffer_data[i * 9 * 6 * 3 + j * 6 * 3 + k * 3 + 1] = faces[i].squares[j].vertices[k].y;
				vertex_buffer_data[i * 9 * 6 * 3 + j * 6 * 3 + k * 3 + 2] = faces[i].squares[j].vertices[k].z;
			}
		}
	}
}

void Cube::getColorData(GLfloat color_buffer_data[]) const {
	for (int i = 0; i < 6; i++) { // for each face
		for (int j = 0; j < 9; j++) { // for each square
			glm::vec3 color;
			Color c = faces[i].getColorAt(j);
			switch (c) {
			case Color::RED:
				color = glm::vec3(1.0f, 0.0f, 0.0f);
				break;
			case Color::ORANGE:
				color = glm::vec3(1.0f, 0.6470588f, 0.0f);
				break;
			case Color::YELLOW:
				color = glm::vec3(1.0f, 1.0f, 0.0f);
				break;
			case Color::GREEN:
				color = glm::vec3(0.0f, 1.0f, 0.0f);
				break;
			case Color::BLUE:
				color = glm::vec3(0.0f, 0.0f, 1.0f);
				break;
			case Color::WHITE:
				color = glm::vec3(1.0f, 1.0f, 1.0f);
				break;
			default:
				color = glm::vec3(0.0f, 0.0f, 0.0f);
			}
			for (int k = 0; k < 6; k++) { // for each vector coordinate
				color_buffer_data[i * 9 * 6 * 3 + j * 6 * 3 + k * 3 + 0] = color.r;
				color_buffer_data[i * 9 * 6 * 3 + j * 6 * 3 + k * 3 + 1] = color.g;
				color_buffer_data[i * 9 * 6 * 3 + j * 6 * 3 + k * 3 + 2] = color.b;
			}
		}
	}
}

bool Cube::perform(Instruction* instptr, float deltatime) {
	// if dt is 0, perform instantly
	if (instptr->isFaceInstruction()) {
		FaceInstruction& inst = *static_cast<FaceInstruction*>(instptr);
		if (deltatime)
			return rotate(inst.getFace(), solveSpeed * deltatime, inst.isClockwise());
		else
			return rotate(inst.getFace(), glm::half_pi<float>(), inst.isClockwise());
	} else {
		CubeInstruction& inst = *static_cast<CubeInstruction*>(instptr);
		if (deltatime)
			return rotate(inst.getAxis(), solveSpeed * deltatime);
		else
			return rotate(inst.getAxis(), glm::half_pi<float>());
	}
}

void Cube::addToQueue(std::shared_ptr<Instruction>& instruction) {
	queue.push_back(instruction);
}

size_t Cube::getQueueSize() const
{
	return queue.size();
}

glm::vec3 Cube::getPosition() const {
	return position;
}

void Cube::translate(glm::vec3 dv) {
	position += dv;
	model = glm::translate(model, dv);
}

bool Cube::isSelected() const {
	return selected;
}

void Cube::select() {
	selected = true;
	translate(glm::vec3(0, 10, 0)); // translate up
}

void Cube::deselect() {
	if (selected) {
		selected = false;
		translate(glm::vec3(0, -10, 0)); // translate down
	}
}

void Cube::swapVertices(glm::vec3* v1[], glm::vec3* v2[], size_t length) {
	// swaps all vec3's in v1 with those in v2
	glm::vec3 swap;
	for (int i = 0; i < length; i++) {
		swap = *v1[i];
		*v1[i] = *v2[i];
		*v2[i] = swap;
	}
}

void Cube::swapColors(Square* s1[], Square* s2[], size_t length) {
	// swaps all square colors in v1 with those in v2
	Square swap(Color::RED);
	for (int i = 0; i < length; i++) {
		swap.color = s1[i]->color;
		s1[i]->color = s2[i]->color;
		s2[i]->color = swap.color;
	}
}

void Cube::generateVertices() {
	using namespace glm;

	// std::cout << "generating vertices..." << std::endl;

	float x = -1.5;
	float y = 1.5;
	float z = 1.5;

	// generate the front face's vertices
	for (int i = 0; i < 9; i++) {
		faces[static_cast<int>(FaceType::FRONT)].squares[i].vertices[0] = vec3(x, y, z); // top left triangle, starting at the right angle going clockwise
		faces[static_cast<int>(FaceType::FRONT)].squares[i].vertices[1] = vec3(x + 1, y, z);
		faces[static_cast<int>(FaceType::FRONT)].squares[i].vertices[2] = vec3(x, y - 1, z);
		faces[static_cast<int>(FaceType::FRONT)].squares[i].vertices[3] = vec3(x + 1, y - 1, z); // bottom right triangle, same as above
		faces[static_cast<int>(FaceType::FRONT)].squares[i].vertices[4] = vec3(x, y - 1, z);
		faces[static_cast<int>(FaceType::FRONT)].squares[i].vertices[5] = vec3(x + 1, y, z);

		x += 1;
		if (x >= 1.5) {
			x = -1.5; // go back to beginning column
			y -= 1; // go down a row
		}
	}

	// generate other faces' vertices by rotating the front face's vertices around the model origin
	for (int i = 0; i < 9; i++) { // for each square
		for (int j = 0; j < 6; j++) { // for each vertex
			// begin with identity matrix at 0, 0, 0
			mat4 model = mat4(1.0f);
			// determine the vertex to rotate
			vec3 vertex_to_rotate = faces[static_cast<int>(FaceType::FRONT)].squares[i].vertices[j];
			// rotate the vertex around different axes
			faces[static_cast<int>(FaceType::UP)].squares[i].vertices[j] = glm::rotate(model, -half_pi<float>(), vec3(1.0f, 0.0f, 0.0f)) * vec4(vertex_to_rotate, 0.0f); // multiply the front vertex by a rotation matrix
			faces[static_cast<int>(FaceType::BACK)].squares[i].vertices[j] = glm::rotate(model, pi<float>(), vec3(1.0f, 0.0f, 0.0f)) * vec4(vertex_to_rotate, 0.0f);
			faces[static_cast<int>(FaceType::DOWN)].squares[i].vertices[j] = glm::rotate(model, half_pi<float>(), vec3(1.0f, 0.0f, 0.0f)) * vec4(vertex_to_rotate, 0.0f);
			faces[static_cast<int>(FaceType::LEFT)].squares[i].vertices[j] = glm::rotate(model, -half_pi<float>(), vec3(0.0f, 1.0f, 0.0f)) * vec4(vertex_to_rotate, 0.0f);
			faces[static_cast<int>(FaceType::RIGHT)].squares[i].vertices[j] = glm::rotate(model, half_pi<float>(), vec3(0.0f, 1.0f, 0.0f)) * vec4(vertex_to_rotate, 0.0f);
		}
	}
}

void Cube::rotateColors(FaceType face, bool clockwise) {
	Square* FRONT_left[3] = { &faces[static_cast<int>(FaceType::FRONT)].squares[0], &faces[static_cast<int>(FaceType::FRONT)].squares[3], &faces[static_cast<int>(FaceType::FRONT)].squares[6] };
	Square* FRONT_bottom[3] = { &faces[static_cast<int>(FaceType::FRONT)].squares[6], &faces[static_cast<int>(FaceType::FRONT)].squares[7], &faces[static_cast<int>(FaceType::FRONT)].squares[8] };
	Square* FRONT_bottom_rev[3] = { &faces[static_cast<int>(FaceType::FRONT)].squares[8], &faces[static_cast<int>(FaceType::FRONT)].squares[7], &faces[static_cast<int>(FaceType::FRONT)].squares[6] };
	Square* FRONT_top[3] = { &faces[static_cast<int>(FaceType::FRONT)].squares[0], &faces[static_cast<int>(FaceType::FRONT)].squares[1], &faces[static_cast<int>(FaceType::FRONT)].squares[2] };
	Square* FRONT_top_rev[3] = { &faces[static_cast<int>(FaceType::FRONT)].squares[2], &faces[static_cast<int>(FaceType::FRONT)].squares[1], &faces[static_cast<int>(FaceType::FRONT)].squares[0] };
	Square* FRONT_right[3] = { &faces[static_cast<int>(FaceType::FRONT)].squares[2], &faces[static_cast<int>(FaceType::FRONT)].squares[5], &faces[static_cast<int>(FaceType::FRONT)].squares[8] };
	Square* UP_top_rev[3] = { &faces[static_cast<int>(FaceType::UP)].squares[2], &faces[static_cast<int>(FaceType::UP)].squares[1], &faces[static_cast<int>(FaceType::UP)].squares[0] };
	Square* UP_left[3] = { &faces[static_cast<int>(FaceType::UP)].squares[0], &faces[static_cast<int>(FaceType::UP)].squares[3], &faces[static_cast<int>(FaceType::UP)].squares[6] };
	Square* UP_bottom[3] = { &faces[static_cast<int>(FaceType::UP)].squares[6], &faces[static_cast<int>(FaceType::UP)].squares[7], &faces[static_cast<int>(FaceType::UP)].squares[8] };
	Square* UP_right[3] = { &faces[static_cast<int>(FaceType::UP)].squares[2], &faces[static_cast<int>(FaceType::UP)].squares[5], &faces[static_cast<int>(FaceType::UP)].squares[8] };
	Square* BACK_top[3] = { &faces[static_cast<int>(FaceType::BACK)].squares[0], &faces[static_cast<int>(FaceType::BACK)].squares[1], &faces[static_cast<int>(FaceType::BACK)].squares[2] };
	Square* BACK_top_rev[3] = { &faces[static_cast<int>(FaceType::BACK)].squares[2], &faces[static_cast<int>(FaceType::BACK)].squares[1], &faces[static_cast<int>(FaceType::BACK)].squares[0] };
	Square* BACK_left[3] = { &faces[static_cast<int>(FaceType::BACK)].squares[0], &faces[static_cast<int>(FaceType::BACK)].squares[3], &faces[static_cast<int>(FaceType::BACK)].squares[6] };
	Square* BACK_bottom[3] = { &faces[static_cast<int>(FaceType::BACK)].squares[6], &faces[static_cast<int>(FaceType::BACK)].squares[7], &faces[static_cast<int>(FaceType::BACK)].squares[8] };
	Square* BACK_right[3] = { &faces[static_cast<int>(FaceType::BACK)].squares[2], &faces[static_cast<int>(FaceType::BACK)].squares[5], &faces[static_cast<int>(FaceType::BACK)].squares[8] };
	Square* DOWN_top[3] = { &faces[static_cast<int>(FaceType::DOWN)].squares[0], &faces[static_cast<int>(FaceType::DOWN)].squares[1], &faces[static_cast<int>(FaceType::DOWN)].squares[2] };
	Square* DOWN_top_rev[3] = { &faces[static_cast<int>(FaceType::DOWN)].squares[2], &faces[static_cast<int>(FaceType::DOWN)].squares[1], &faces[static_cast<int>(FaceType::DOWN)].squares[0] };
	Square* DOWN_bottom[3] = { &faces[static_cast<int>(FaceType::DOWN)].squares[6], &faces[static_cast<int>(FaceType::DOWN)].squares[7], &faces[static_cast<int>(FaceType::DOWN)].squares[8] };
	Square* DOWN_left[3] = { &faces[static_cast<int>(FaceType::DOWN)].squares[0], &faces[static_cast<int>(FaceType::DOWN)].squares[3], &faces[static_cast<int>(FaceType::DOWN)].squares[6] };
	Square* DOWN_right[3] = { &faces[static_cast<int>(FaceType::DOWN)].squares[2], &faces[static_cast<int>(FaceType::DOWN)].squares[5], &faces[static_cast<int>(FaceType::DOWN)].squares[8] };
	Square* LEFT_bottom[3] = { &faces[static_cast<int>(FaceType::LEFT)].squares[6], &faces[static_cast<int>(FaceType::LEFT)].squares[7], &faces[static_cast<int>(FaceType::LEFT)].squares[8] };
	Square* LEFT_bottom_rev[3] = { &faces[static_cast<int>(FaceType::LEFT)].squares[8], &faces[static_cast<int>(FaceType::LEFT)].squares[7], &faces[static_cast<int>(FaceType::LEFT)].squares[6] };
	Square* LEFT_right[3] = { &faces[static_cast<int>(FaceType::LEFT)].squares[2], &faces[static_cast<int>(FaceType::LEFT)].squares[5], &faces[static_cast<int>(FaceType::LEFT)].squares[8] };
	Square* LEFT_right_rev[3] = { &faces[static_cast<int>(FaceType::LEFT)].squares[8], &faces[static_cast<int>(FaceType::LEFT)].squares[5], &faces[static_cast<int>(FaceType::LEFT)].squares[2] };
	Square* LEFT_top[3] = { &faces[static_cast<int>(FaceType::LEFT)].squares[2], &faces[static_cast<int>(FaceType::LEFT)].squares[1], &faces[static_cast<int>(FaceType::LEFT)].squares[0] };
	Square* LEFT_left[3] = { &faces[static_cast<int>(FaceType::LEFT)].squares[0], &faces[static_cast<int>(FaceType::LEFT)].squares[3], &faces[static_cast<int>(FaceType::LEFT)].squares[6] };
	Square* RIGHT_left[3] = { &faces[static_cast<int>(FaceType::RIGHT)].squares[0], &faces[static_cast<int>(FaceType::RIGHT)].squares[3], &faces[static_cast<int>(FaceType::RIGHT)].squares[6] };
	Square* RIGHT_right_rev[3] = { &faces[static_cast<int>(FaceType::RIGHT)].squares[8], &faces[static_cast<int>(FaceType::RIGHT)].squares[5], &faces[static_cast<int>(FaceType::RIGHT)].squares[2] };
	Square* RIGHT_top[3] = { &faces[static_cast<int>(FaceType::RIGHT)].squares[0], &faces[static_cast<int>(FaceType::RIGHT)].squares[1], &faces[static_cast<int>(FaceType::RIGHT)].squares[2] };
	Square* RIGHT_top_rev[3] = { &faces[static_cast<int>(FaceType::RIGHT)].squares[2], &faces[static_cast<int>(FaceType::RIGHT)].squares[1], &faces[static_cast<int>(FaceType::RIGHT)].squares[0] };
	Square* RIGHT_bottom[3] = { &faces[static_cast<int>(FaceType::RIGHT)].squares[6], &faces[static_cast<int>(FaceType::RIGHT)].squares[7], &faces[static_cast<int>(FaceType::RIGHT)].squares[8] };
	Square* RIGHT_bottom_rev[3] = { &faces[static_cast<int>(FaceType::RIGHT)].squares[8], &faces[static_cast<int>(FaceType::RIGHT)].squares[7], &faces[static_cast<int>(FaceType::RIGHT)].squares[6] };

	// rotate the rows/columns around the face
	switch (face) {
	case FaceType::FRONT:
		if (clockwise) {
			// swap up's bottom row with right's left column
			swapColors(UP_bottom, RIGHT_left, 3);
			// swap up's bottom row with down's top row (reversed)
			swapColors(UP_bottom, DOWN_top_rev, 3);
			// swap up's bottom row with left's right column (reversed)
			swapColors(UP_bottom, LEFT_right_rev, 3);
		} else {
			swapColors(UP_bottom, LEFT_right_rev, 3);
			swapColors(UP_bottom, DOWN_top_rev, 3);
			swapColors(UP_bottom, RIGHT_left, 3);
		}

		break;

	case FaceType::UP:
		if (clockwise) {
			// swap back's bottom row with right's top row (reversed)
			swapColors(BACK_bottom, RIGHT_top_rev, 3);
			// swap back's bottom row with front's top row (reversed)
			swapColors(BACK_bottom, FRONT_top_rev, 3);
			// swap back's bottom row with left's top row
			swapColors(BACK_bottom, LEFT_top, 3);
		} else {
			swapColors(BACK_bottom, LEFT_top, 3);
			swapColors(BACK_bottom, FRONT_top_rev, 3);
			swapColors(BACK_bottom, RIGHT_top_rev, 3);
		}

		break;

	case FaceType::BACK:
		if (clockwise) {
			// swap up's top row (reversed) with left's left column
			swapColors(UP_top_rev, LEFT_left, 3);
			// swap up's top row (reversed) with down's bottom row
			swapColors(UP_top_rev, DOWN_bottom, 3);
			// swap up's top row (reversed) with right's right column (reversed)
			swapColors(UP_top_rev, RIGHT_right_rev, 3);
		} else {
			swapColors(UP_top_rev, RIGHT_right_rev, 3);
			swapColors(UP_top_rev, DOWN_bottom, 3);
			swapColors(UP_top_rev, LEFT_left, 3);
		}
		break;

	case FaceType::DOWN:
		if (clockwise) {
			// swap front's bottom row with right's bottom row
			swapColors(FRONT_bottom, RIGHT_bottom, 3);
			// swap front's bottom row with back's top row
			swapColors(FRONT_bottom, BACK_top_rev, 3);
			// swap front's bottom row with left's bottom row
			swapColors(FRONT_bottom, LEFT_bottom, 3);
		} else {
			swapColors(FRONT_bottom, LEFT_bottom, 3);
			swapColors(FRONT_bottom, BACK_top_rev, 3);
			swapColors(FRONT_bottom, RIGHT_bottom, 3);
		}

		break;

	case FaceType::LEFT:
		if (clockwise) {
			// swap up's left column with front's left column
			swapColors(UP_left, FRONT_left, 3);
			// swap up's left column with down's left column
			swapColors(UP_left, DOWN_left, 3);
			// swap up's left column with back's left column
			swapColors(UP_left, BACK_left, 3);
		} else {
			swapColors(UP_left, BACK_left, 3);
			swapColors(UP_left, DOWN_left, 3);
			swapColors(UP_left, FRONT_left, 3);
		}

		break;

	case FaceType::RIGHT:
		if (clockwise) {
			// swap up's right column with back's right column
			swapColors(UP_right, BACK_right, 3);
			// swap up's right column with down's right column
			swapColors(UP_right, DOWN_right, 3);
			// swap up's right column with front's right column
			swapColors(UP_right, FRONT_right, 3);
		} else {
			swapColors(UP_right, FRONT_right, 3);
			swapColors(UP_right, DOWN_right, 3);
			swapColors(UP_right, BACK_right, 3);
		}

		break;
	}

	// rotate the face's data
	// clockwise		
	Square* FRONT_cross[3] = { &faces[static_cast<int>(face)].squares[1], &faces[static_cast<int>(face)].squares[1], &faces[static_cast<int>(face)].squares[1] };
	Square* FRONT_cross_swap[3] = { &faces[static_cast<int>(face)].squares[5], &faces[static_cast<int>(face)].squares[7], &faces[static_cast<int>(face)].squares[3] };
	Square* FRONT_corners[3] = { &faces[static_cast<int>(face)].squares[0], &faces[static_cast<int>(face)].squares[0], &faces[static_cast<int>(face)].squares[6] };
	Square* FRONT_corners_swap[3] = { &faces[static_cast<int>(face)].squares[2], &faces[static_cast<int>(face)].squares[6], &faces[static_cast<int>(face)].squares[8] };
	// cclockwise
	Square* FRONT_cross_swap_rev[3] = { &faces[static_cast<int>(face)].squares[3], &faces[static_cast<int>(face)].squares[7], &faces[static_cast<int>(face)].squares[5] };
	Square* FRONT_corners_cc[3] = { &faces[static_cast<int>(face)].squares[0], &faces[static_cast<int>(face)].squares[2], &faces[static_cast<int>(face)].squares[6] };
	Square* FRONT_corners_swap_cc[3] = { &faces[static_cast<int>(face)].squares[2], &faces[static_cast<int>(face)].squares[8], &faces[static_cast<int>(face)].squares[8] };

	if (clockwise) {
		swapColors(FRONT_cross, FRONT_cross_swap, 3);
		swapColors(FRONT_corners, FRONT_corners_swap, 3);
	} else {
		swapColors(FRONT_cross, FRONT_cross_swap_rev, 3);
		swapColors(FRONT_corners_cc, FRONT_corners_swap_cc, 3);
	}
}

void Cube::rotateColors(glm::vec3 axis) {
	// check if axis is clockwise or cclockwise
	bool clockwise = (axis.x + axis.y + axis.z) > 0;

	Square* FRONT_verti[3] = { &faces[static_cast<int>(FaceType::FRONT)].squares[1], &faces[static_cast<int>(FaceType::FRONT)].squares[4], &faces[static_cast<int>(FaceType::FRONT)].squares[7] };
	Square* FRONT_horiz[3] = { &faces[static_cast<int>(FaceType::FRONT)].squares[3], &faces[static_cast<int>(FaceType::FRONT)].squares[4], &faces[static_cast<int>(FaceType::FRONT)].squares[5] };
	Square* UP_verti[3] = { &faces[static_cast<int>(FaceType::UP)].squares[1], &faces[static_cast<int>(FaceType::UP)].squares[4], &faces[static_cast<int>(FaceType::UP)].squares[7] };
	Square* UP_horiz[3] = { &faces[static_cast<int>(FaceType::UP)].squares[3], &faces[static_cast<int>(FaceType::UP)].squares[4], &faces[static_cast<int>(FaceType::UP)].squares[5] };
	Square* BACK_verti[3] = { &faces[static_cast<int>(FaceType::BACK)].squares[1], &faces[static_cast<int>(FaceType::BACK)].squares[4], &faces[static_cast<int>(FaceType::BACK)].squares[7] };
	Square* BACK_horiz_rev[3] = { &faces[static_cast<int>(FaceType::BACK)].squares[5], &faces[static_cast<int>(FaceType::BACK)].squares[4], &faces[static_cast<int>(FaceType::BACK)].squares[3] };
	Square* DOWN_verti[3] = { &faces[static_cast<int>(FaceType::DOWN)].squares[1], &faces[static_cast<int>(FaceType::DOWN)].squares[4], &faces[static_cast<int>(FaceType::DOWN)].squares[7] };
	Square* DOWN_horiz_rev[3] = { &faces[static_cast<int>(FaceType::DOWN)].squares[5], &faces[static_cast<int>(FaceType::DOWN)].squares[4], &faces[static_cast<int>(FaceType::DOWN)].squares[3] };
	Square* LEFT_verti_rev[3] = { &faces[static_cast<int>(FaceType::LEFT)].squares[7], &faces[static_cast<int>(FaceType::LEFT)].squares[4], &faces[static_cast<int>(FaceType::LEFT)].squares[1] };
	Square* LEFT_horiz[3] = { &faces[static_cast<int>(FaceType::LEFT)].squares[3], &faces[static_cast<int>(FaceType::LEFT)].squares[4], &faces[static_cast<int>(FaceType::LEFT)].squares[5] };
	Square* RIGHT_verti[3] = { &faces[static_cast<int>(FaceType::RIGHT)].squares[1], &faces[static_cast<int>(FaceType::RIGHT)].squares[4], &faces[static_cast<int>(FaceType::RIGHT)].squares[7] };
	Square* RIGHT_horiz[3] = { &faces[static_cast<int>(FaceType::RIGHT)].squares[3], &faces[static_cast<int>(FaceType::RIGHT)].squares[4], &faces[static_cast<int>(FaceType::RIGHT)].squares[5] };
	

	if (std::abs(axis.x)) { // if x axis
		rotateColors(FaceType::LEFT, clockwise);
		rotateColors(FaceType::RIGHT, !clockwise);
		if (clockwise) {
			swapColors(FRONT_verti, DOWN_verti, 3);
			swapColors(FRONT_verti, BACK_verti, 3);
			swapColors(FRONT_verti, UP_verti, 3);
		} else {
			swapColors(FRONT_verti, UP_verti, 3);
			swapColors(FRONT_verti, BACK_verti, 3);
			swapColors(FRONT_verti, DOWN_verti, 3);
		}

	} else if (std::abs(axis.y)) { // if y axis
		rotateColors(FaceType::UP, !clockwise);
		rotateColors(FaceType::DOWN, clockwise);
		if (clockwise) {
			swapColors(FRONT_horiz, RIGHT_horiz, 3);
			swapColors(FRONT_horiz, BACK_horiz_rev, 3);
			swapColors(FRONT_horiz, LEFT_horiz, 3);
		} else {
			swapColors(FRONT_horiz, LEFT_horiz, 3);
			swapColors(FRONT_horiz, BACK_horiz_rev, 3);
			swapColors(FRONT_horiz, RIGHT_horiz, 3);
		}

	} else if (std::abs(axis.z)) { // if z axis (note: not 'else' because the axis could be (0,0,0) 
		rotateColors(FaceType::FRONT, !clockwise);
		rotateColors(FaceType::BACK, clockwise);
		if (clockwise) {
			swapColors(UP_horiz, LEFT_verti_rev, 3);
			swapColors(UP_horiz, DOWN_horiz_rev, 3);
			swapColors(UP_horiz, RIGHT_verti, 3);
		} else {
			swapColors(UP_horiz, RIGHT_verti, 3);
			swapColors(UP_horiz, DOWN_horiz_rev, 3);
			swapColors(UP_horiz, LEFT_verti_rev, 3);
		}
	}
}

void Cube::rotateVertices(FaceType face, float radians, bool clockwise) {
	// direction
	if (!clockwise)
		radians *= -1;

	// collect vertices to rotate
	glm::vec3* vertices_to_rotate[54 + 6 * 3 * 4]; // 54 vertices on front face + 6 vertices per square + 3 squares per edge + 4 edges to rotate

	// collect the face's vertices
	for (int i = 0; i < 9; i++) { // for each square 
		for (int j = 0; j < 6; j++) // for each vertex
			vertices_to_rotate[i * 6 + j] = &faces[static_cast<int>(face)].squares[i].vertices[j];
	}

	switch (face) {
	case FaceType::FRONT:
		// collect the up face's bottom row
		faces[static_cast<int>(FaceType::UP)].getVerticesInRow(2, vertices_to_rotate + 54);
		// collect the down face's top row
		faces[static_cast<int>(FaceType::DOWN)].getVerticesInRow(0, vertices_to_rotate + 54 + 18);
		// collect the left face's right column
		faces[static_cast<int>(FaceType::LEFT)].getVerticesInColumn(2, vertices_to_rotate + 54 + 36);
		// collect the right face's left column
		faces[static_cast<int>(FaceType::RIGHT)].getVerticesInColumn(0, vertices_to_rotate + 54 + 54);

		// rotate vertices
		for (int i = 0; i < 54 + 6 * 3 * 4; i++) // for each vertex
			*vertices_to_rotate[i] = glm::rotate(glm::mat4(1.0f), radians, glm::vec3(0, 0, -1)) * glm::vec4(*vertices_to_rotate[i], 0);

		break;

	case FaceType::UP:
		// collect the front face's top row
		faces[static_cast<int>(FaceType::FRONT)].getVerticesInRow(0, vertices_to_rotate + 54);
		// collect the right face's top row
		faces[static_cast<int>(FaceType::RIGHT)].getVerticesInRow(0, vertices_to_rotate + 54 + 18);
		// collect the left face's top row
		faces[static_cast<int>(FaceType::LEFT)].getVerticesInRow(0, vertices_to_rotate + 54 + 36);
		// collect the back face's bottom row
		faces[static_cast<int>(FaceType::BACK)].getVerticesInRow(2, vertices_to_rotate + 54 + 54);

		// rotate vertices
		for (int i = 0; i < 54 + 6 * 3 * 4; i++)
			*vertices_to_rotate[i] = glm::rotate(glm::mat4(1.0f), radians, glm::vec3(0, -1, 0)) * glm::vec4(*vertices_to_rotate[i], 0);

		break;

	case FaceType::BACK:
		// collect the up face's top row
		faces[static_cast<int>(FaceType::UP)].getVerticesInRow(0, vertices_to_rotate + 54);
		// collect the down face's bottom row
		faces[static_cast<int>(FaceType::DOWN)].getVerticesInRow(2, vertices_to_rotate + 54 + 18);
		// collect the left face's left column
		faces[static_cast<int>(FaceType::LEFT)].getVerticesInColumn(0, vertices_to_rotate + 54 + 36);
		// collect the right face's right column
		faces[static_cast<int>(FaceType::RIGHT)].getVerticesInColumn(2, vertices_to_rotate + 54 + 54);

		// rotate vertices
		for (int i = 0; i < 54 + 6 * 3 * 4; i++)
			*vertices_to_rotate[i] = glm::rotate(glm::mat4(1.0f), radians, glm::vec3(0, 0, 1)) * glm::vec4(*vertices_to_rotate[i], 0);

		break;

	case FaceType::DOWN:
		// collect the front face's bottom row
		faces[static_cast<int>(FaceType::FRONT)].getVerticesInRow(2, vertices_to_rotate + 54);
		// collect the right face's bottom row
		faces[static_cast<int>(FaceType::RIGHT)].getVerticesInRow(2, vertices_to_rotate + 54 + 18);
		// collect the left face's bottom row
		faces[static_cast<int>(FaceType::LEFT)].getVerticesInRow(2, vertices_to_rotate + 54 + 36);
		// collect the back face's top row
		faces[static_cast<int>(FaceType::BACK)].getVerticesInRow(0, vertices_to_rotate + 54 + 54);

		// rotate vertices
		for (int i = 0; i < 54 + 6 * 3 * 4; i++)
			*vertices_to_rotate[i] = glm::rotate(glm::mat4(1.0f), radians, glm::vec3(0, 1, 0)) * glm::vec4(*vertices_to_rotate[i], 0);

		break;

	case FaceType::LEFT:
		// collect the front face's left column
		faces[static_cast<int>(FaceType::FRONT)].getVerticesInColumn(0, vertices_to_rotate + 54);
		// collect the up face's left column
		faces[static_cast<int>(FaceType::UP)].getVerticesInColumn(0, vertices_to_rotate + 54 + 18);
		// collect the down face's left column
		faces[static_cast<int>(FaceType::DOWN)].getVerticesInColumn(0, vertices_to_rotate + 54 + 36);
		// collect the back face's left column
		faces[static_cast<int>(FaceType::BACK)].getVerticesInColumn(0, vertices_to_rotate + 54 + 54);
		
		// rotate vertices
		for (int i = 0; i < 54 + 6 * 3 * 4; i++)
			*vertices_to_rotate[i] = glm::rotate(glm::mat4(1.0f), radians, glm::vec3(1, 0, 0)) * glm::vec4(*vertices_to_rotate[i], 0);

		break;

	case FaceType::RIGHT:
		// collect the front face's right column
		faces[static_cast<int>(FaceType::FRONT)].getVerticesInColumn(2, vertices_to_rotate + 54);
		// collect the up face's right column
		faces[static_cast<int>(FaceType::UP)].getVerticesInColumn(2, vertices_to_rotate + 54 + 18);
		// collect the down face's right column
		faces[static_cast<int>(FaceType::DOWN)].getVerticesInColumn(2, vertices_to_rotate + 54 + 36);
		// collect the back face's right column
		faces[static_cast<int>(FaceType::BACK)].getVerticesInColumn(2, vertices_to_rotate + 54 + 54);

		// rotate vertices
		for (int i = 0; i < 54 + 6 * 3 * 4; i++)
			*vertices_to_rotate[i] = glm::rotate(glm::mat4(1.0f), radians, glm::vec3(-1, 0, 0)) * glm::vec4(*vertices_to_rotate[i], 0);

		break;

	default:
		std::cout << "Invalid cube face." << std::endl;
		return;
	}
}

void Cube::rotateVertices(glm::vec3 axis, float radians) {
	glm::mat4 rotationalMatrix = glm::rotate(glm::mat4(1.0f), radians, axis);
	for (Face& face : faces) {
		for (Square& square : face.squares) {
			for (glm::vec3& vertex : square.vertices) {
				vertex = rotationalMatrix * glm::vec4(vertex, 0);
			}
		}
	}
}

void Cube::snapVertices() {
	for (Face& face : faces) {
		for (Square& square : face.squares) {
			for (glm::vec3& vertex : square.vertices) {
				// round each component to the nearest 0.5
				vertex.x = round(vertex.x * 2) / 2.0;
				vertex.y = round(vertex.y * 2) / 2.0;
				vertex.z = round(vertex.z * 2) / 2.0;
			}
		}
	}
}
