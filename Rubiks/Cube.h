#pragma once

#include <vector>
#include <memory>

#include <GL/glew.h>
#include <glm/gtc/constants.hpp>

#include "Face.h"

enum class FaceType {
	FRONT, UP, BACK, DOWN, LEFT, RIGHT
};


class Cube {
	friend class FaceInstruction; // so that an instruction can execute rotate() by itself
	friend class CubeInstruction;
private:
	Face faces[6]; // Front, Up, Back, Down, Left, Right
public:
	float solveSpeed; // how fast a face rotates
public:

	Cube(Color squares[54]);

	/* copy constructor */
	Cube(Cube* other);

	void scramble();

	void print() const;

	/* when supplied an array, inserts current vertices into array */
	void updateVertexData(GLfloat vertex_buffer_data[]) const;

	/* when supplied an array, inserts the colors of vertices into array */
	void updateColorData(GLfloat color_buffer_data[]) const;

	/* returns a pointer to the specified facetype */
	Face* getFace(FaceType type);

private:
	void generateVertices();

	/* returns true when face has completed a 90-degree turn */
	bool rotate(FaceType face, float radians, bool clockwise);

	/* returns true when cube has completed a 90-degree turn along a specified axis */
	/* note: an axis looks like this: (0, 0, 1), where the cube rotates clockwise from -z to +z */
	bool rotate(glm::vec3 axis, float radians);

	/* Instantly rotates a face by 90 degrees by swapping colors with where they should be. */
	void rotateColors(FaceType face, bool clockwise);
	
	/* Instantly rotates cube by 90 degrees by swapping colors */
	void rotateColors(glm::vec3 axis);

	/* Rotates a face by radians by performing rotational transformations on the face and surrounding edges' vertices */
	void rotateVertices(FaceType face, float radians, bool clockwise);

	/* Rotates cube by radians on axis by performing transformations on vertices */
	void rotateVertices(glm::vec3 axis, float radians);

	/* Snaps all vertices to the nearest increment of 0.5
	   Corrects vertex location when rotateVertices() accidentally rotates too far */
	void snapVertices();

	/* when supplied two or more vec3 pointers, this will swap the data at those pointers */
	static void swapVertices(glm::vec3* v1[], glm::vec3* v2[], size_t length);

	/* when supplied two or more square pointers, this will swap the colors at those pointers */
	static void swapColors(Square* s1[], Square* s2[], size_t length);
};

class Instruction {
protected:
	Instruction(); // ban default constructor
public:
	virtual bool execute(float deltatime, Cube* cube) = 0;
	/* executes the instruction instantly. Ex. instantly rotates 90 degrees instead of slowly */
	virtual bool executeInstantly(Cube* cube) = 0;
	virtual void print() = 0;
};

class FaceInstruction : public Instruction {
protected:
	FaceType face;
	bool clockwise;
public:
	FaceInstruction(FaceType face, bool clockwise = true);
	bool execute(float deltatime, Cube* cube);
	bool executeInstantly(Cube* cube);
	void print();
	bool operator==(const FaceInstruction& other);
};


class CubeInstruction : public Instruction {
protected:
	glm::vec3 axis;
public:
	CubeInstruction(glm::vec3 axis);
	bool execute(float deltatime, Cube* cube);
	bool executeInstantly(Cube* cube);
	void print();
	bool operator==(const CubeInstruction& other);
};