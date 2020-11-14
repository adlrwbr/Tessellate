#include "Cube.h"
#include "AI.h"
#include "CubeManager.h"

class App {
private:
	GLFWwindow* window;
	CubeManager* cubemngr;
	AI ai;
	/* handle for the shaders */
	GLuint programID;
	GLuint VertexArrayID;
	/* handle for the MVP uniform */
	GLuint MatrixID;
	glm::mat4 Projection;
	glm::mat4 View;
	glm::mat4 Model;
	/* vertices */
	GLfloat g_vertex_buffer_data[3 * 3 * 2 * 9 * 6]; // 3 components that make up one vertex * 3 vertices that make up one triangle * 2 triangles that make up one square * 9 squares * 6 faces
	/* colors */
	GLfloat g_color_buffer_data[3 * 3 * 2 * 9 * 6];
	/* buffer names */
	GLuint vertexbuffer;
	GLuint colorbuffer;
	/* model rotation speed */
	float modelRotSpeed;
	float fps;
public:
	App(CubeManager* cubemngr, AI& ai);
	~App();
	
	void start();

	void loop();
	/* callback function on key down 
	   Needed to make static. Why? When it isn't static, its signature looks like:
	   func(void* this, GLFWwindow* window, ...). However, glfwSetKeyCallback wants 
	   func(GLFW* window, ...). Therefore it must be static.
	   Instead of accessing "this" through the argument list, we assign "this" as a glfw window user pointer,
	   which we can query at any time through the window. */
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
};