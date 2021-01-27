#include <thread>

#include "Cube.hpp"
#include "Grid.hpp"
#include "Camera.hpp"

class App {
private:
	bool running;
	GLFWwindow* window;
	Grid* grid;
	/* handle for the shaders */
	GLuint programID;
	GLuint VertexArrayID;
	/* handle for the MVP uniform */
	GLuint MatrixID;
	glm::mat4 Projection;
	Camera camera;
	/* vertices */
	GLfloat g_vertex_buffer_data[3 * 3 * 2 * 9 * 6]; // 3 components that make up one vertex * 3 vertices that make up one triangle * 2 triangles that make up one square * 9 squares * 6 faces
	/* colors */
	GLfloat g_color_buffer_data[3 * 3 * 2 * 9 * 6];
	/* buffer names */
	GLuint vertexbuffer;
	GLuint colorbuffer;
	float fps;
public:
	App();

	~App();
	
	void start();

	void stop();

	bool isRunning();

private:
	/* main update/draw loop */
	void loop();

	/* update all relevant objects */
	void update(float deltatime);

	/* listen for CLI input */
	void beginInputHandler(); // to-do: replace CLI input with GUI text box.

	/* GUI input callback
	   Needed to make static. Why? When it isn't static, its signature looks like:
	   func(void* this, GLFWwindow* window, ...). However, glfwSetKeyCallback wants 
	   func(GLFW* window, ...). Therefore it must be static.
	   Instead of accessing "this" through the argument list, we assign "this" as a glfw window user pointer,
	   which we can query at any time through the window. */
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
};