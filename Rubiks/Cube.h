#pragma once

#include <vector>
#include <memory>

#include <GL/glew.h>
#include <glm/gtc/constants.hpp>

#include "Face.h"
#include "Instruction.h"

class Cube {
	friend class FaceInstruction; // so that an instruction can execute rotate() by itself
	friend class CubeInstruction;
private:
	Color initialSqColors[54];
	Face faces[6]; // Front, Up, Back, Down, Left, Right
	std::vector<std::shared_ptr<Instruction>> queue; // pending instructions - to-do: should these be unique instead of shared?
	glm::vec3 position;
public:
	glm::mat4 model; // model to world transformation matrix
	float modelRotSpeed; // how fast the cube rotates on Y axis
	float solveSpeed; // how fast a face rotates
public:

	/* standard colors */
	Cube();

	/* custom colors */
	Cube(Color squares[54]);

	/* copy constructor */
	Cube(Cube* other);

	void update(float deltatime);

	void scramble();

	void print() const;

	void reset();

	/* returns a pointer to the specified facetype */
	Face* getFace(FaceType type);

	/* when supplied an array, inserts current vertices into array */
	void getVertexData(GLfloat vertex_buffer_data[]) const;

	/* when supplied an array, inserts the colors of vertices into array */
	void getColorData(GLfloat color_buffer_data[]) const;

	/* increments a given instruction, returns true when completed.
	* If deltatime == 0, performs instantly
	*/
	bool perform(Instruction* instptr, float deltatime);

	/* appends instruction to the queue */
	void addToQueue(std::shared_ptr<Instruction>& instruction);

	/* returns the number of pending instructions in the queue */
	size_t getQueueSize() const;

	/* translate graphical cube */
	void translate(glm::vec3 dv);

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
