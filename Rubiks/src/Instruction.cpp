#include <iostream>

#include "Instruction.hpp"

Instruction::Instruction() {}

FaceInstruction::FaceInstruction(FaceType face, bool clockwise)
	: face(face), clockwise(clockwise) {}

void FaceInstruction::print() const {
	char faceLetter{};
	switch (face) {
	case FaceType::FRONT:
		faceLetter = 'F';
		break;
	case FaceType::UP:
		faceLetter = 'U';
		break;
	case FaceType::BACK:
		faceLetter = 'B';
		break;
	case FaceType::DOWN:
		faceLetter = 'D';
		break;
	case FaceType::LEFT:
		faceLetter = 'L';
		break;
	case FaceType::RIGHT:
		faceLetter = 'R';
		break;
	}
	std::cout << faceLetter << (clockwise ? "" : "'");
}

bool FaceInstruction::isFaceInstruction() const
{
	return true;
}

FaceType FaceInstruction::getFace() const
{
	return face;
}

bool FaceInstruction::isClockwise() const
{
	return clockwise;
}

void FaceInstruction::setDirection(bool clockwise) {
	this->clockwise = clockwise;
}

bool FaceInstruction::operator==(const FaceInstruction& other) const {
	return face == other.face && clockwise == other.clockwise;
}

CubeInstruction::CubeInstruction(glm::vec3 axis)
	: axis(axis) {}

void CubeInstruction::print() const {
	char axisNotation{};
	if (axis[0] == 1 || axis[0] == -1)
		axisNotation = 'x';
	else if (axis[1] == 1 || axis[1] == -1)
		axisNotation = 'y';
	else if (axis[2] == 1 || axis[2] == -1)
		axisNotation = 'z';
	else
		axisNotation = '-';
	std::cout << axisNotation << (axis[0] + axis[1] + axis[2] < 0 ? "'" : "");
}

glm::vec3 CubeInstruction::getAxis() const {
	return axis;
}

bool CubeInstruction::isFaceInstruction() const
{
	return false;
}

bool CubeInstruction::operator==(const CubeInstruction& other) const {
	return axis == other.axis;
}

