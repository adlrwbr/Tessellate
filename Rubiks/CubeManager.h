#pragma once
#include <memory>

#include "Cube.h"

class CubeManager {
private:
	Color initialSqColors[54];
	std::vector<std::shared_ptr<Instruction>> queue;
public:
	Cube* cube;
public:
	CubeManager(Color colors[54]);

	~CubeManager();
	
	/* updates the cube's animation when supplied with deltatime */
	void update(float deltatime);

	/* Destroys and creates a new cube with the original layout */
	void resetCube();

	/* appends the supplied instruction to the queue */
	void addToQueue(std::shared_ptr<Instruction>& instruction);

	/* returns the number of pending instructions in the queue */
	int getQueueSize();
};