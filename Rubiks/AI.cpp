#include "AI.h"

#include <iostream>

AI::AI(CubeManager* cubemngr)
	: cubemngr(cubemngr), futureCube(new Cube(cubemngr->cube)) {}

AI::~AI() {
	delete futureCube;
}

void AI::calculatePaint(Color pattern[9]) {
	// clear pending instructions
	instructions.clear();

	// create copy of the displayed cube
	delete futureCube;
	futureCube = new Cube(cubemngr->cube);

	// algorithm to "paint" top face
	if (cubemngr->getQueueSize() == 0) {

		// rotate so that white center is on top
		rotateToTopCenter(Color::WHITE);

		static const FaceType const faces_on_xz[4] = { FaceType::FRONT, FaceType::RIGHT, FaceType::BACK, FaceType::LEFT };

		// get all white edges in top layer
		int loopcounter = 0; // to-do: debug only. Delete me.
		while (!(isEdgeInTopLayer(faces_on_xz[0], Color::WHITE)
			&& isEdgeInTopLayer(faces_on_xz[1], Color::WHITE)
			&& isEdgeInTopLayer(faces_on_xz[2], Color::WHITE)
			&& isEdgeInTopLayer(faces_on_xz[3], Color::WHITE)
			)) {
			loopcounter++;
			if (loopcounter > 10) {
				std::cout << "INFINITE LOOP WHEN SOLVING CROSS. Here's the pattern: " << std::endl;
				futureCube->print();
			}
			// for each face on the x and z axes
			for (const FaceType& f : faces_on_xz) {
				// if there is a white edge in the middle layer and not the top layer, rotate the face until that white edge is on top
				if (!isEdgeInTopLayer(f, Color::WHITE) && (isEdgeMiddleLeft(f, Color::WHITE) || isEdgeMiddleRight(f, Color::WHITE))) {
					std::shared_ptr<Instruction> instruction = std::make_shared<FaceInstruction>(f, isEdgeMiddleLeft(f, Color::WHITE));
					addInstruction(instruction);
				}
				// if there is a white edge in the bottom layer and not the top layer, rotate the face until that white edge is on top
				if (!isEdgeInTopLayer(f, Color::WHITE) && isEdgeInBottomLayer(f, Color::WHITE)) {
					std::shared_ptr<Instruction> instruction0 = std::make_shared<FaceInstruction>(f);
					std::shared_ptr<Instruction> instruction1 = std::make_shared<FaceInstruction>(f);
					addInstruction(instruction0);
					addInstruction(instruction1);
				}
				// if there's a white edge in the top layer and middle-left of this face and in top layer of relative left face
				if (isEdgeInTopLayer(f, Color::WHITE) && isEdgeMiddleLeft(f, Color::WHITE) && isEdgeInTopLayer(getRelLeftOnY(f), Color::WHITE)) {
					// rotate face cc, rotate DOWN, rotate face clockwise
					std::shared_ptr<Instruction> instruction0 = std::make_shared<FaceInstruction>(f, false);
					std::shared_ptr<Instruction> instruction1 = std::make_shared<FaceInstruction>(FaceType::DOWN);
					std::shared_ptr<Instruction> instruction2 = std::make_shared<FaceInstruction>(f);
					addInstruction(instruction0);
					addInstruction(instruction1);
					addInstruction(instruction2);
				}
				// while there is a white edge in the top and bottom layer, rotate DOWN clockwise 
				while (isEdgeInTopLayer(f, Color::WHITE) && isEdgeInBottomLayer(f, Color::WHITE)) {
					std::shared_ptr<Instruction> rotateD = std::make_shared<FaceInstruction>(FaceType::DOWN);
					addInstruction(rotateD);
				}
			}
		}

		// flip edges if they are not on the UP face
		while (isEdgeFlipped(faces_on_xz[0], Color::WHITE) || isEdgeFlipped(faces_on_xz[1], Color::WHITE) || isEdgeFlipped(faces_on_xz[2], Color::WHITE) || isEdgeFlipped(faces_on_xz[3], Color::WHITE)) {
			for (const FaceType& f : faces_on_xz) {
				if (isEdgeFlipped(f, Color::WHITE))
					flipEdge(f);
			}
		}

		// corners
		// for each square on the UP face
		for (int i = 0; i < 9; i++) {
			Color c = pattern[i];

			// if square is not a corner, continue
			if (!(i == 0 || i == 2 || i == 6 || i == 8))
				continue;

			// is it already in the correct location?
			if (futureCube->getFace(FaceType::UP)->getColorAt(i) == c)
				continue;

			// if not, locate the tile
			for (const FaceType& f : faces_on_xz) {
				// if corner is in the top layer of a face, bring it to the bottom layer
				if (isCornerTopLeft(f, c)) {
					// rotate face cc, DOWN cc, face clockwise
					std::shared_ptr<Instruction> instruction0 = std::make_shared<FaceInstruction>(f, false);
					std::shared_ptr<Instruction> instruction1 = std::make_shared<FaceInstruction>(FaceType::DOWN, false);
					std::shared_ptr<Instruction> instruction2 = std::make_shared<FaceInstruction>(f);
					addInstruction(instruction0);
					addInstruction(instruction1);
					addInstruction(instruction2);
				}

			}
			// the desired corner is now somewhere in the bottom layer

			// if the tile is on the DOWN face, bring it up to a face on x or z
			if (futureCube->getFace(FaceType::DOWN)->getColorAt(0) == c
				|| futureCube->getFace(FaceType::DOWN)->getColorAt(2) == c
				|| futureCube->getFace(FaceType::DOWN)->getColorAt(6) == c
				|| futureCube->getFace(FaceType::DOWN)->getColorAt(8) == c) {
				// rel left clockwise, down cc, rel left cc, down, down
				FaceType relLeftF;
				if (futureCube->getFace(FaceType::DOWN)->getColorAt(0) == c) {
					relLeftF = FaceType::LEFT;
				} else if (futureCube->getFace(FaceType::DOWN)->getColorAt(2) == c) {
					relLeftF = FaceType::FRONT;
				} else if (futureCube->getFace(FaceType::DOWN)->getColorAt(6) == c) {
					relLeftF = FaceType::BACK;
				} else {
					relLeftF = FaceType::RIGHT;
				}

				std::shared_ptr<Instruction> relLeft = std::make_shared<FaceInstruction>(relLeftF);
				std::shared_ptr<Instruction> downCC = std::make_shared<FaceInstruction>(FaceType::DOWN, false);
				std::shared_ptr<Instruction> relLeftCC = std::make_shared<FaceInstruction>(relLeftF, false);
				std::shared_ptr<Instruction> down0 = std::make_shared<FaceInstruction>(FaceType::DOWN);
				std::shared_ptr<Instruction> down1 = std::make_shared<FaceInstruction>(FaceType::DOWN);
				addInstruction(relLeft);
				addInstruction(downCC);
				addInstruction(relLeftCC);
				addInstruction(down0);
				addInstruction(down1);
			}
		}

		
		for (std::shared_ptr<Instruction>& inst : instructions)
			inst.get()->print();
		// simplify instruction set
		/*simplifyInstructions();
		std::cout << std::endl;
		for (std::shared_ptr<Instruction>& inst : instructions)
			inst.get()->print();*/
	}

}

void AI::start() {
	// to-do: print solution
	for (std::shared_ptr<Instruction>& instruction : instructions) { // use the & to avoid copying the instruction objects into the scope
		cubemngr->addToQueue(instruction);
	}
	instructions.clear();
}

void AI::addInstruction(std::shared_ptr<Instruction>& instruction) {
	instruction->executeInstantly(futureCube); // execute the instruction on the futureCube
	instructions.push_back(instruction);
}

void AI::simplifyInstructions() {
}

FaceType AI::getRelLeftOnY(FaceType face)
{
	switch (face) {
	case FaceType::FRONT:
		return FaceType::LEFT;
	case FaceType::LEFT:
		return FaceType::BACK;
	case FaceType::BACK:
		return FaceType::RIGHT;
	case FaceType::RIGHT:
		return FaceType::FRONT;
	default:
		throw "incorrect FaceType parameter";
	}
}

FaceType AI::getRelRightOnY(FaceType face)
{
	switch (face) {
	case FaceType::FRONT:
		return FaceType::RIGHT;
	case FaceType::LEFT:
		return FaceType::FRONT;
	case FaceType::BACK:
		return FaceType::LEFT;
	case FaceType::RIGHT:
		return FaceType::BACK;
	default:
		throw "incorrect FaceType parameter";
	}
}

void AI::rotateToTopCenter(Color c) {
	// find which face has the color
	FaceType hasColor = static_cast<FaceType>(0);
	for (int i = 0; i < 6; i++) {
		if (futureCube->getFace(static_cast<FaceType>(i))->getColorAt(4) == c) {
			hasColor = static_cast<FaceType>(i);
			break;
		}
	}

	// add instructions to rotate cube
	if (hasColor == FaceType::FRONT) {
		std::shared_ptr<Instruction> instruction = std::make_shared<CubeInstruction>(glm::vec3(-1, 0, 0));
		addInstruction(instruction);

	} else if (hasColor == FaceType::UP) {
		return;

	} else if (hasColor == FaceType::BACK) {
		std::shared_ptr<Instruction> instruction = std::make_shared<CubeInstruction>(glm::vec3(1, 0, 0));
		addInstruction(instruction);

	} else if (hasColor == FaceType::DOWN) {
		std::shared_ptr<Instruction> instruction0 = std::make_shared<CubeInstruction>(glm::vec3(1, 0, 0));
		std::shared_ptr<Instruction> instruction1 = std::make_shared<CubeInstruction>(glm::vec3(1, 0, 0));
		addInstruction(instruction0);
		addInstruction(instruction1);


	} else if (hasColor == FaceType::LEFT) {
		std::shared_ptr<Instruction> instruction = std::make_shared<CubeInstruction>(glm::vec3(0, 0, -1));
		addInstruction(instruction);

	} else { // right
		std::shared_ptr<Instruction> instruction = std::make_shared<CubeInstruction>(glm::vec3(0, 0, 1));
		addInstruction(instruction);

	}
}

bool AI::isEdgeInTopLayer(FaceType face, Color color)
{
	unsigned int edgeSquare = 0; // index of square on edge of this face
	unsigned int upSquare = 0; // index of square on the UP face that connects to edgeSquare
	switch (face) {
	case FaceType::FRONT:
		edgeSquare = 1;
		upSquare = 7;
		break;
	case FaceType::BACK:
		edgeSquare = 7;
		upSquare = 1;
		break;
	case FaceType::LEFT:
		edgeSquare = 1;
		upSquare = 3;
		break;
	case FaceType::RIGHT:
		edgeSquare = 1;
		upSquare = 5;
		break;
	}
	return (futureCube->getFace(FaceType::UP)->getColorAt(upSquare) == color || futureCube->getFace(face)->getColorAt(edgeSquare) == color);
}

bool AI::isEdgeMiddleLeft(FaceType face, Color color)
{
	Color edgeColor = Color::RED; // Color of square on left edge of this face
	Color relLeftColor = Color::RED; // Color of square on the right edge of the relatively left face
	switch (face) {
	case FaceType::FRONT:
		edgeColor = futureCube->getFace(face)->getColorAt(3);
		relLeftColor = futureCube->getFace(FaceType::LEFT)->getColorAt(5);
		break;
	case FaceType::BACK:
		edgeColor = futureCube->getFace(face)->getColorAt(5);
		relLeftColor = futureCube->getFace(FaceType::RIGHT)->getColorAt(5);
		break;
	case FaceType::LEFT:
		edgeColor = futureCube->getFace(face)->getColorAt(3);
		relLeftColor = futureCube->getFace(FaceType::BACK)->getColorAt(3);
		break;
	case FaceType::RIGHT:
		edgeColor = futureCube->getFace(face)->getColorAt(3);
		relLeftColor = futureCube->getFace(FaceType::FRONT)->getColorAt(5);
		break;
	}
	return (edgeColor == color || relLeftColor == color);
}

bool AI::isEdgeMiddleRight(FaceType face, Color color)
{
	Color edgeColor = Color::RED; // Color of square on left edge of this face
	Color relRightColor = Color::RED; // Color of square on the right edge of the relatively left face
	switch (face) {
	case FaceType::FRONT:
		edgeColor = futureCube->getFace(face)->getColorAt(5);
		relRightColor = futureCube->getFace(FaceType::RIGHT)->getColorAt(3);
		break;
	case FaceType::BACK:
		edgeColor = futureCube->getFace(face)->getColorAt(3);
		relRightColor = futureCube->getFace(FaceType::LEFT)->getColorAt(3);
		break;
	case FaceType::LEFT:
		edgeColor = futureCube->getFace(face)->getColorAt(5);
		relRightColor = futureCube->getFace(FaceType::FRONT)->getColorAt(3);
		break;
	case FaceType::RIGHT:
		edgeColor = futureCube->getFace(face)->getColorAt(5);
		relRightColor = futureCube->getFace(FaceType::BACK)->getColorAt(5);
		break;
	}
	return (edgeColor == color || relRightColor == color);
}

bool AI::isEdgeInBottomLayer(FaceType face, Color color)
{
	unsigned int edgeSquare = 0; // index of square on edge of this face
	unsigned int downSquare = 0; // index of square on the DOWN face that connects to edgeSquare
	switch (face) {
	case FaceType::FRONT:
		edgeSquare = 7;
		downSquare = 1;
		break;
	case FaceType::BACK:
		edgeSquare = 1;
		downSquare = 7;
		break;
	case FaceType::LEFT:
		edgeSquare = 7;
		downSquare = 3;
		break;
	case FaceType::RIGHT:
		edgeSquare = 7;
		downSquare = 5;
		break;
	}
	return (futureCube->getFace(FaceType::DOWN)->getColorAt(downSquare) == color || futureCube->getFace(face)->getColorAt(edgeSquare) == color);
}

bool AI::isEdgeFlipped(FaceType face, Color color)
{
	// return false if a square of Color is on the face
	if (face == FaceType::FRONT || face == FaceType::RIGHT || face == FaceType::LEFT)
		return futureCube->getFace(face)->getColorAt(1) == color;
	else // back face
		return futureCube->getFace(face)->getColorAt(7) == color;
}

void AI::flipEdge(FaceType face) {
	// rotate face counterclockwise
	std::shared_ptr<Instruction> rotFaceCC = std::make_shared<FaceInstruction>(face, false);

	// rotate UP clockwise
	std::shared_ptr<Instruction> rotUP = std::make_shared<FaceInstruction>(FaceType::UP);

	// rotate the relatively left face counterclockwise
	std::shared_ptr<Instruction> rotRelLeftCC = std::make_shared<FaceInstruction>(getRelLeftOnY(face), false);

	// add instructions
	addInstruction(rotFaceCC);
	addInstruction(rotUP);
	addInstruction(rotRelLeftCC);
}

bool AI::isCornerTopLeft(FaceType face, Color color)
{
	
	unsigned int relUpSquareLeft = 0; // the tile on the UP face that corresponds with the top left corner of this face
	unsigned int topLeft = 0; // tile on the top left of this face
	unsigned int relLeftTopRight = 2; // the relative left face's top right
	switch (face) {
	case FaceType::FRONT:
		relUpSquareLeft = 6;
		break;
	case FaceType::RIGHT:
		relUpSquareLeft = 8;
		break;
	case FaceType::BACK:
		relUpSquareLeft = 2;
		topLeft = 8;
		break;
	case FaceType::LEFT:
		relUpSquareLeft = 0;
		relLeftTopRight = 6;
	}
	return (futureCube->getFace(face)->getColorAt(topLeft) == color // top left of this face
		|| futureCube->getFace(getRelLeftOnY(face))->getColorAt(relLeftTopRight) == color // relative left's top right
		|| futureCube->getFace(FaceType::UP)->getColorAt(relUpSquareLeft) == color); // up
}

bool AI::isCornerBottomLeft(FaceType face, Color color) {
	unsigned int relDownSquare = 0; // the tile on the DOWN face that corresponds with the bottom left corner of this face
	unsigned int bottomLeft = 6; // tile on the bottom left of this face
	unsigned int relLeftBottomRight = 8; // the relative left face's top right
	switch (face) {
	case FaceType::FRONT:
		relDownSquare = 0;
		break;
	case FaceType::RIGHT:
		relDownSquare = 2;
		break;
	case FaceType::BACK:
		relDownSquare = 8;
		bottomLeft = 2;
		break;
	case FaceType::LEFT:
		relDownSquare = 6;
		relLeftBottomRight = 0;
	}
	return (futureCube->getFace(face)->getColorAt(bottomLeft) == color // bottom left of this face
		|| futureCube->getFace(getRelLeftOnY(face))->getColorAt(relLeftBottomRight) == color // relative left's bottom right
		|| futureCube->getFace(FaceType::DOWN)->getColorAt(relDownSquare) == color); // down 
}
