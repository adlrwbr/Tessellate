#pragma once

#include "glm/common.hpp"
#include <glm/gtc/constants.hpp>

#include "Face.hpp"

/*
* probably bad alternative
*
class Instruction {
	Instruction(FaceType face, bool clockwise = true);
	Instruction(glm::vec3 axis);
	execute();
};
*
* another alternative
*
class Cube {
	void execute(Instruction& inst) {
		if (inst.isFaceInstruction())
			
	}
};
* 
* another alternative
* 
enum class InstructionType:
	FRONT_CLOCKWISE
	FRONT_CC
	...
	X_CLOCKWISE
	X_CC
	...

class Cube
	void addToQueue(InstructionType);
	bool perform(InstructionType, dt); // if dt == 0, perform instantly

helper functions
	bool isClockwise(InstructionType);
	bool isFaceInstruction(InstructionType);

*/

class Instruction {
protected:
	Instruction(); // ban default constructor
public:
	virtual void print() const = 0;
	virtual bool isFaceInstruction() const = 0;
};

class FaceInstruction : public Instruction {
protected:
	FaceType face;
	bool clockwise;
public:
	FaceInstruction(FaceType face, bool clockwise = true);
	void print() const;
	bool isFaceInstruction() const;
	FaceType getFace() const;
	bool isClockwise() const;
	void setDirection(bool clockwise);
	bool operator==(const FaceInstruction& other) const;
};

class CubeInstruction : public Instruction {
protected:
	glm::vec3 axis;
public:
	CubeInstruction(glm::vec3 axis);
	void print() const;
	glm::vec3 getAxis() const;
	bool isFaceInstruction() const;
	bool operator==(const CubeInstruction& other) const;
};