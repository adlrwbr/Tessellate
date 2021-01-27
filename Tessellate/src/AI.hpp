#pragma once

#include <vector>
#include <memory>

#include "Cube.hpp"

class AI {
private:
	/* only start() is allowed to touch the displayed cube */
	Cube* cube;

	std::vector<std::shared_ptr<Instruction>> instructions;

	/* when calculating future moves, rotations are made to futureCube behind the scenes instead of to the displayed cube */
	Cube* futureCube;
	
public:
	AI(Cube* cube);
	~AI();

	/* generates the instructions to rotate the cube in order to achieve the supplied pattern on the UP face */
	void calculatePaint(Color pattern[9]);

	/* adds instruction set to the cube's queue */
	void start();
private:
	/* adds instructions to AI's queue and adjusts data cube */
	void addInstruction(std::shared_ptr<Instruction>& instruction);

	/**
	* simplifies the instruction set. Ex. F, F' cancel out. F, F, F turns into F'
	* Postcondition: state of cube does not change before and after instruction simplification
	*/
	void simplifyInstructions(std::vector<std::shared_ptr<Instruction>>& instructions);

	void printInstructions();

	/**
	* returns the relatively left face on the y axis.
	* Ex. getRelLeftOnY(FaceType::FRONT) returns FaceType::LEFT
	* Precondition: face must be FaceType::FRONT, BACK, LEFT, or RIGHT
	*/
	FaceType getRelLeftOnY(FaceType face);

	/**
	* returns the relatively right face on the y axis.
	* Ex. getRelRightOnY(FaceType::FRONT) returns FaceType::RIGHT
	* Precondition: face must be FaceType::FRONT, BACK, LEFT, or RIGHT
	*/
	FaceType getRelRightOnY(FaceType face);

	/* pushes instructions to rotate the cube so that Color c becomes the top center square */
	void rotateToTopCenter(Color c);
	
	/* returns the color of the edge that needs to be in the top layer of this face based on the UP pattern supplied */
	Color getTargetEdgeColorOf(FaceType face, Color pattern[9]);

	/** 
	* returns true if an edge piece of Color color is in the top layer of a face
	* Preconditions:
	*	- face is FRONT, BACK, LEFT, or RIGHT
	*/
	bool isEdgeInTopLayer(FaceType face, Color color);

	/* returns true if an edge of Color color is on the left of the middle layer of a face */
	bool isEdgeMiddleLeft(FaceType face, Color color);

	/* returns true if an edge of Color color is on the left of the middle layer of a face */
	bool isEdgeMiddleRight(FaceType face, Color color);

	/* returns true if an edge piece of Color color is in the bottom layer of a face */
	bool isEdgeInBottomLayer(FaceType face, Color color);

	/**
	* returns true if the Color square is not on the UP face.
	* 
	* Sometimes, when you place the edge in the top layer, the Color tile is not on the UP face and it needs to be “flipped"
	*/
	bool isEdgeFlipped(FaceType face, Color color);

	/* adds instructions to "flip" the top edge of a specified face */
	void flipEdge(FaceType face);

	/**
	* returns true if the top left corner piece of this face has a tile of color
	* Preconditions:
	*	- face is FRONT, BACK, LEFT, or RIGHT
	*/
	bool isCornerTopLeft(FaceType face, Color color);

	/* returns true if the bottom left corner piece of this face has a tile of color */
	bool isCornerBottomLeft(FaceType face, Color color);
};