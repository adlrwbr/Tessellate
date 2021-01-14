#include "AI.h"

#include <iostream>
#include <typeinfo>

AI::AI(Cube* cube)
	: cube(cube), futureCube(new Cube(cube)) {}

AI::~AI() {
	delete futureCube;
}

void AI::calculatePaint(Color pattern[9]) {
	// clear pending instructions
	instructions.clear();

	// create copy of the displayed cube
	delete futureCube;
	futureCube = new Cube(cube);

	// algorithm to "paint" top face
	if (cube->getQueueSize() == 0) {

		// rotate so that center is on top
		rotateToTopCenter(pattern[4]);

		static const FaceType faces_on_xz[4] = { FaceType::FRONT, FaceType::RIGHT, FaceType::BACK, FaceType::LEFT };

		// get all correct edges in top layer
		int loopcounter = 0; // to-do: debug only. Delete me.
		while (!(isEdgeInTopLayer(faces_on_xz[0], pattern[7])
			&& isEdgeInTopLayer(faces_on_xz[1], pattern[5])
			&& isEdgeInTopLayer(faces_on_xz[2], pattern[1])
			&& isEdgeInTopLayer(faces_on_xz[3], pattern[3])
			)) {
			loopcounter++;
			if (loopcounter > 10) {
				std::cout << "INFINITE LOOP WHEN SOLVING CROSS. Here's the pattern: " << std::endl;
				futureCube->print();
			}
			// for each face on the x and z axes
			for (const FaceType& f : faces_on_xz) {

				Color c = getTargetEdgeColorOf(f, pattern); // the color of the edge that needs to be in the top layer of this face

				// if the edge is in the middle layer and not the top layer, rotate the face until the edge is on top
				if (!isEdgeInTopLayer(f, c) && (isEdgeMiddleLeft(f, c) || isEdgeMiddleRight(f, c))) {
					std::shared_ptr<Instruction> instruction = std::make_shared<FaceInstruction>(f, isEdgeMiddleLeft(f, Color::WHITE));
					addInstruction(instruction);
				}
				// if the edge is in the bottom layer and not the top layer, rotate the face until the edge is on top
				if (!isEdgeInTopLayer(f, c) && isEdgeInBottomLayer(f, c)) {
					std::shared_ptr<Instruction> instruction0 = std::make_shared<FaceInstruction>(f);
					std::shared_ptr<Instruction> instruction1 = std::make_shared<FaceInstruction>(f);
					addInstruction(instruction0);
					addInstruction(instruction1);
				}
				// if the correct edge is in the top layer of this face, and the correct edge is in the top layer of the relatively left face
				// and there is a correct edge of ANOTHER face in the middle-left of this face
				if (isEdgeInTopLayer(f, c) 
					&& (isEdgeInTopLayer(getRelLeftOnY(f), getTargetEdgeColorOf(getRelLeftOnY(f), pattern)))
					&& (isEdgeMiddleLeft(f, getTargetEdgeColorOf(getRelRightOnY(f), pattern)) // RIGHT face's target edge is in middle left
						|| isEdgeMiddleLeft(f, getTargetEdgeColorOf(getRelRightOnY(getRelRightOnY(f)), pattern)) // BACK face's target edge is in middle left
						)) { 
					// rotate face cc, rotate DOWN, rotate face clockwise
					std::shared_ptr<Instruction> instruction0 = std::make_shared<FaceInstruction>(f, false);
					std::shared_ptr<Instruction> instruction1 = std::make_shared<FaceInstruction>(FaceType::DOWN);
					std::shared_ptr<Instruction> instruction2 = std::make_shared<FaceInstruction>(f);
					addInstruction(instruction0);
					addInstruction(instruction1);
					addInstruction(instruction2);
				}

				// move missing target colors from bottom layer into their proper positions
				Color relRightTarg = getTargetEdgeColorOf(getRelRightOnY(f), pattern);
				Color relBackTarg = getTargetEdgeColorOf(getRelRightOnY(getRelRightOnY(f)), pattern);
				Color relLeftTarg = getTargetEdgeColorOf(getRelLeftOnY(f), pattern);

				bool isRTargInBottom = isEdgeInBottomLayer(f, relRightTarg) && !isEdgeInTopLayer(getRelRightOnY(f), relRightTarg); // RIGHT face's target edge is in bottom
				bool isBTargInBottom = isEdgeInBottomLayer(f, relBackTarg) && !isEdgeInTopLayer(getRelRightOnY(getRelRightOnY(f)), relBackTarg); // BACK face's target edge is in bottom
				bool isLTargInBottom = isEdgeInBottomLayer(f, relLeftTarg) && !isEdgeInTopLayer(getRelLeftOnY(f), relLeftTarg); // LEFT face's target edge is in bottom

				if (isRTargInBottom) { // if the RIGHT face's target color is in the bottom layer of this face and it is not in the top layer of the RIGHT face
					std::shared_ptr<Instruction> rotateD = std::make_shared<FaceInstruction>(FaceType::DOWN);
					std::shared_ptr<Instruction> rotateRelR = std::make_shared<FaceInstruction>(getRelRightOnY(f));
					addInstruction(rotateD);
					addInstruction(rotateRelR);
					addInstruction(rotateRelR);
				} else if (isLTargInBottom) {
					std::shared_ptr<Instruction> rotateDCC = std::make_shared<FaceInstruction>(FaceType::DOWN);
					std::shared_ptr<Instruction> rotateRelL = std::make_shared<FaceInstruction>(getRelLeftOnY(f));
					addInstruction(rotateDCC);
					addInstruction(rotateRelL);
					addInstruction(rotateRelL);
				} else if (isBTargInBottom) {
					std::shared_ptr<Instruction> rotateD = std::make_shared<FaceInstruction>(FaceType::DOWN);
					std::shared_ptr<Instruction> rotateRelB = std::make_shared<FaceInstruction>(getRelRightOnY(getRelRightOnY(f)));
					addInstruction(rotateD);
					addInstruction(rotateD);
					addInstruction(rotateRelB);
					addInstruction(rotateRelB);
				}
			}
		}

		// flip edges if they are not on the UP face
		for (const FaceType& f : faces_on_xz) {
			if (isEdgeFlipped(f, getTargetEdgeColorOf(f, pattern)))
				flipEdge(f);
		}
		
		// corners
		// for each square on the UP face
		for (int i = 0; i < 9; i++) {
			// if square is not a corner, continue
			if (!(i == 0 || i == 2 || i == 6 || i == 8))
				continue;
			
			Color c = pattern[i];

			// is it already in the correct location?
			if (futureCube->getFace(FaceType::UP)->getColorAt(i) == c)
				continue;

			// if not, locate the tile
			FaceType faceWCorner{};
			bool faceFound = false;

			// search the bottom left corner of every face
			for (const FaceType& f : faces_on_xz) {
				// if corner is in the top layer of a face, bring it to the bottom layer
				if (isCornerBottomLeft(f, c)) {
					faceFound = true;
					faceWCorner = f;
					break;
				}
			}
			// search the top left corner of every face
			if (!faceFound) {
				for (const FaceType& f : faces_on_xz) {
					// if corner is in the top left of a face and that face's top left corner is not correct, bring it to the bottom layer
					if (isCornerTopLeft(f, c) && !(f == FaceType::FRONT && futureCube->getFace(FaceType::UP)->getColorAt(6) == pattern[6]
						|| f == FaceType::RIGHT && futureCube->getFace(FaceType::UP)->getColorAt(8) == pattern[8]
						|| f == FaceType::BACK && futureCube->getFace(FaceType::UP)->getColorAt(2) == pattern[2]
						|| f == FaceType::LEFT && futureCube->getFace(FaceType::UP)->getColorAt(0) == pattern[0])) {
						// rotate face cc, DOWN cc, face clockwise, DOWN clockwise
						std::shared_ptr<Instruction> instruction0 = std::make_shared<FaceInstruction>(f, false);
						std::shared_ptr<Instruction> instruction1 = std::make_shared<FaceInstruction>(FaceType::DOWN, false);
						std::shared_ptr<Instruction> instruction2 = std::make_shared<FaceInstruction>(f);
						std::shared_ptr<Instruction> instruction3 = std::make_shared<FaceInstruction>(FaceType::DOWN);
						addInstruction(instruction0);
						addInstruction(instruction1);
						addInstruction(instruction2);
						addInstruction(instruction3);

						faceFound = true;
						faceWCorner = f;
						break;
					}
				}
			}
				
			// the desired corner is now the bottom left corner of faceWCorner

			// rotate until the corner is under i
			while (i == 0 && faceWCorner != FaceType::LEFT
				|| i == 2 && faceWCorner != FaceType::BACK
				|| i == 6 && faceWCorner != FaceType::FRONT
				|| i == 8 && faceWCorner != FaceType::RIGHT) {
				std::shared_ptr<Instruction> down = std::make_shared<FaceInstruction>(FaceType::DOWN);
				addInstruction(down);
				faceWCorner = getRelRightOnY(faceWCorner);
			}

			// if the tile is on the DOWN face, bring it up to a face on x or z
			if (faceWCorner == FaceType::FRONT && futureCube->getFace(FaceType::DOWN)->getColorAt(0) == c
				|| faceWCorner == FaceType::RIGHT && futureCube->getFace(FaceType::DOWN)->getColorAt(2) == c
				|| faceWCorner == FaceType::LEFT && futureCube->getFace(FaceType::DOWN)->getColorAt(6) == c
				|| faceWCorner == FaceType::BACK && futureCube->getFace(FaceType::DOWN)->getColorAt(8) == c) {

				// rel left clockwise, down cc, rel left cc, down, down
				std::shared_ptr<Instruction> relLeft = std::make_shared<FaceInstruction>(getRelLeftOnY(faceWCorner));
				std::shared_ptr<Instruction> downCC = std::make_shared<FaceInstruction>(FaceType::DOWN, false);
				std::shared_ptr<Instruction> relLeftCC = std::make_shared<FaceInstruction>(getRelLeftOnY(faceWCorner), false);
				std::shared_ptr<Instruction> down0 = std::make_shared<FaceInstruction>(FaceType::DOWN);
				std::shared_ptr<Instruction> down1 = std::make_shared<FaceInstruction>(FaceType::DOWN);
				addInstruction(relLeft);
				addInstruction(downCC);
				addInstruction(relLeftCC);
				addInstruction(down0);
				addInstruction(down1);
			}

			// the desired tile is now in the bottom left corner of faceWCorner on an x_z face

			// if tile is in the bottom left of this face
			if (faceWCorner != FaceType::BACK && futureCube->getFace(faceWCorner)->getColorAt(6) == c
				|| faceWCorner == FaceType::BACK && futureCube->getFace(faceWCorner)->getColorAt(2) == c) {
				// down, rel left, down cc, rel left cc
				std::shared_ptr<Instruction> instruction0 = std::make_shared<FaceInstruction>(FaceType::DOWN);
				std::shared_ptr<Instruction> instruction1 = std::make_shared<FaceInstruction>(getRelLeftOnY(faceWCorner));
				std::shared_ptr<Instruction> instruction2 = std::make_shared<FaceInstruction>(FaceType::DOWN, false);
				std::shared_ptr<Instruction> instruction3 = std::make_shared<FaceInstruction>(getRelLeftOnY(faceWCorner), false);
				addInstruction(instruction0);
				addInstruction(instruction1);
				addInstruction(instruction2);
				addInstruction(instruction3);
			} else { // tile is in the bottom right of the relatively left face
				// down cc, face cc, down, face
				std::shared_ptr<Instruction> instruction0 = std::make_shared<FaceInstruction>(FaceType::DOWN, false);
				std::shared_ptr<Instruction> instruction1 = std::make_shared<FaceInstruction>(faceWCorner, false);
				std::shared_ptr<Instruction> instruction2 = std::make_shared<FaceInstruction>(FaceType::DOWN);
				std::shared_ptr<Instruction> instruction3 = std::make_shared<FaceInstruction>(faceWCorner);
				addInstruction(instruction0);
				addInstruction(instruction1);
				addInstruction(instruction2);
				addInstruction(instruction3);
			}
		}
		/*// verify pattern is correct
		bool match = true;
		for (int j = 0; j < 9; j++) {
			if (futureCube->getFace(static_cast<FaceType>(FaceType::UP))->getColorAt(j) != pattern[j]) {
				match = false;
				break;
			}
		}
		std::cout << "UP " << (match ? "matches" : "DOES NOT MATCH") << " the pattern" << std::endl;
		if (!match)
			cube->print();

		// simplify instruction set
		std::cout << "simplification: " << '\n';
		printInstructions();*/
		simplifyInstructions(instructions);

	}

}

void AI::start() {
	for (std::shared_ptr<Instruction>& instruction : instructions) { // use the & to avoid copying the instruction objects into the scope
		cube->addToQueue(instruction);
	}
	instructions.clear();
}

void AI::addInstruction(std::shared_ptr<Instruction>& instruction) {
	futureCube->perform(instruction.get(), 0); // perform the instruction instantly on futureCube
	instructions.push_back(instruction);
}

void AI::simplifyInstructions(std::vector<std::shared_ptr<Instruction>>& instructions) {

	bool simplified = false; // becomes true if a simplification occurred

	// for each instruction
	for (int i = 0; i < static_cast<int>(instructions.size()); i++) {
		std::shared_ptr<Instruction>& inst = instructions[i];
		
		// simplify face instructions
		if (inst.get()->isFaceInstruction()) {

			// remove unnecessary rotations ex. FFFF
			if (i + 3 < static_cast<int>(instructions.size())
				&& instructions[i + 1].get()->isFaceInstruction()
				&& instructions[i + 2].get()->isFaceInstruction()
				&& instructions[i + 3].get()->isFaceInstruction()
				&& *static_cast<FaceInstruction*>(inst.get()) == *static_cast<FaceInstruction*>(instructions[i + 1].get())
				&& *static_cast<FaceInstruction*>(inst.get()) == *static_cast<FaceInstruction*>(instructions[i + 2].get())
				&& *static_cast<FaceInstruction*>(inst.get()) == *static_cast<FaceInstruction*>(instructions[i + 3].get())
				) {
				instructions.erase(instructions.begin() + i, instructions.begin() + i + 4);
				i--;
				simplified = true;



			// remove opposing face instructions ex. FF'
			} else if (i + 1 < static_cast<int>(instructions.size()) && instructions[i + 1].get()->isFaceInstruction()
				&& static_cast<FaceInstruction*>(inst.get())->getFace() == static_cast<FaceInstruction*>(instructions[i + 1].get())->getFace()
				&& static_cast<FaceInstruction*>(inst.get())->isClockwise() != static_cast<FaceInstruction*>(instructions[i + 1].get())->isClockwise()
				) {
				instructions.erase(instructions.begin() + i, instructions.begin() + i + 2);
				i--;
				simplified = true;

			// optimize inefficient face instructions ex. FFF -> F'
			} else if (i + 2 < static_cast<int>(instructions.size())
				&& instructions[i + 1].get()->isFaceInstruction()
				&& instructions[i + 2].get()->isFaceInstruction()
				&& *static_cast<FaceInstruction*>(inst.get()) == *static_cast<FaceInstruction*>(instructions[i + 1].get())
				&& *static_cast<FaceInstruction*>(inst.get()) == *static_cast<FaceInstruction*>(instructions[i + 2].get())
				) {
				static_cast<FaceInstruction*>(inst.get())->setDirection(!static_cast<FaceInstruction*>(inst.get())->isClockwise()); // flip direction of first instruction
				instructions.erase(instructions.begin() + i + 1, instructions.begin() + i + 3); // erase next and next-next instructions
				// did not erase inst from instructions; no need to decrement i
				simplified = true;
			}

		} else { // to-do: simplify cube instructions

		}
	}

	// recursion
	if (simplified)
		simplifyInstructions(instructions);
	else
		return; // termination condition
}

void AI::printInstructions() {
	for (std::shared_ptr<Instruction>& inst : instructions)
		inst.get()->print();
	std::cout << std::endl;
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

Color AI::getTargetEdgeColorOf(FaceType face, Color pattern[9])
{
	if (face == FaceType::FRONT)
		return pattern[7];
	else if (face == FaceType::RIGHT)
		return pattern[5];
	else if (face == FaceType::BACK)
		return pattern[1];
	else // LEFT
		return pattern[3];
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

	// rotate UP counterclockwise
	std::shared_ptr<Instruction> rotUPCC = std::make_shared<FaceInstruction>(FaceType::UP, false);

	// add instructions
	addInstruction(rotFaceCC);
	addInstruction(rotUP);
	addInstruction(rotRelLeftCC);
	addInstruction(rotUPCC);
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
