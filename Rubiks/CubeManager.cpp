#include <iostream>

#include "CubeManager.h"

CubeManager::CubeManager(Color colors[54]) {
	for (int i = 0; i < 54; i++)
		initialSqColors[i] = colors[i];

	cube = new Cube(colors);
}

CubeManager::~CubeManager() {
	delete cube;
}

void CubeManager::update(float deltatime) {
	cube->update(deltatime);
	if (queue.size() > 0) {
		if (queue[0]->execute(deltatime, cube)) { // if rotation was completed
			queue.erase(queue.begin()); // remove the current instruction
		}
	}
}

void CubeManager::resetCube() {
	delete cube;
	cube = new Cube(initialSqColors);
	queue.clear();
	std::cout << "Reset cube" << std::endl;
}

void CubeManager::addToQueue(std::shared_ptr<Instruction>& instruction) {
	queue.push_back(instruction);
}

int CubeManager::getQueueSize()
{
	return queue.size();
}
