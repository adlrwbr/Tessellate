#pragma once
#include <stdexcept>

#include <glm/glm.hpp>

#include "Cube.h"

class Camera {
private:
	glm::vec3 defaultEyePosition;
	glm::vec3 eyePosition; // position in world space
	glm::vec3 refPosition; // where the camera is looking
	glm::vec3 up; // specifies the up vector. Ex. (0, -1, 0) would be upside down
	bool focusMode;

public:
	float vyaw, vpitch; // velocity of yaw and pitch around refPosition
	glm::mat4 view; // The View Matrix

public:

	Camera(glm::vec3 eyePosition, glm::vec3 refPosition, glm::vec3 up);

	/* update camera movement each frame */
	void update(float deltatime);

	/* reset orientation and movement */
	void reset();

	void translate(glm::vec3 dv);

	void translateTo(glm::vec3 coordinates);

	/* pan/tilt to look at a specific reference */
	void lookAt(glm::vec3 refPosition);

	/* sets the default position of the camera */
	void setDefaultEyePosition(glm::vec3 defaultEyePosition);

	/* returns if camera is in focus mode */
	bool isInFocusMode() const;

	/* toggles focus mode on or off */
	void toggleFocusMode(bool state);

	/* move close to a cube if in focus mode */
	void focusOn(Cube* cube);

private:
	/* translate around refPosition while maintaining constant distance from it. Adjusts pan/tilt to look at refPosition, too. Aka an arcball camera. */
	void moveAround(float dyaw, float dpitch);
};