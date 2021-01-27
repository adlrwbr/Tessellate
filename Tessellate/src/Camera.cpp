#include "Camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(glm::vec3 eyePosition, glm::vec3 refPosition, glm::vec3 up)
	: defaultEyePosition(eyePosition), eyePosition(eyePosition), refPosition(refPosition), up(up), focusMode(false),
		vyaw(0), vpitch(0) {
	view = glm::lookAt(eyePosition, refPosition, up);
}

void Camera::update(float deltatime) {
	if (vyaw != 0 || vpitch != 0)
		moveAround(vyaw * deltatime, vpitch * deltatime);
}

void Camera::reset() {
	translateTo(defaultEyePosition);
	lookAt(glm::vec3(0, 0, 0));
	up = glm::vec3(0, 1, 0);
	vyaw = 0; vpitch = 0;
}

void Camera::translate(glm::vec3 dv) {
	eyePosition += dv;
	view = glm::translate(view, dv);
}

void Camera::translateTo(glm::vec3 coordinates) {
	translate(coordinates - eyePosition);
}

void Camera::lookAt(glm::vec3 refPosition) {
	this->refPosition = refPosition;
	view = glm::lookAt(eyePosition, refPosition, up);
}

void Camera::setDefaultEyePosition(glm::vec3 defaultEyePosition) { this->defaultEyePosition = defaultEyePosition; }

bool Camera::isInFocusMode() const { return focusMode; }

void Camera::toggleFocusMode(bool state) { focusMode = state; }

/* move close to a cube if in focus mode */

void Camera::focusOn(Cube* cube) {
	if (!isInFocusMode())
		throw std::runtime_error("Cannot focus camera if focus mode is disabled.");
	translateTo(cube->getPosition() + glm::vec3(4, 7, 6));
	lookAt(cube->getPosition());
}

void Camera::moveAround(float dyaw, float dpitch) {
	glm::vec3 camForward = eyePosition - refPosition; // this is the vector we'll be rotating. not normalized bc we want to preserve magnitude (how far away camera is from reference)

	// calculate coordinates: see https://gamedev.stackexchange.com/questions/20758/how-can-i-orbit-a-camera-about-its-target-point
	// first, find the camera's up and right vectors
	//	- up is already known
	//	- right can be calculated using the cross product of the direction vector (camera to target) and the up vector (camera to up)
	//	the cross product is a vector perpendicular to a plane (made up by two vectors)
	glm::vec3 camRight = glm::cross<float>(glm::normalize(camForward), up); // this should be normalized now

	glm::mat4 yawRotation = glm::rotate(glm::mat4(1.0f), dyaw, up); // create a yaw rotation matrix
	glm::mat4 pitchRotation = glm::rotate(glm::mat4(1.0f), dpitch, camRight); // create a pitch rotation matrix

	// this is the new position of the camera relative to the origin
	glm::vec3 newVector = (glm::vec3)(yawRotation * pitchRotation * glm::vec4(camForward, 0.0f));

	// convert back to world space
	glm::vec3 newEyePosition = newVector + refPosition;

	// execute transformations
	translateTo(newEyePosition);
	lookAt(refPosition);
}
