// include std
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <thread>
#include <stdexcept>
#include <functional> // for std::ref
#include <time.h> // for seeding rng

// include GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// include GLFW
#include <GLFW/glfw3.h>

// include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "App.h"
#include "Shader.h"

App::App(CubeManager* cubemngr, AI& ai)
 : cubemngr(cubemngr), ai(ai), modelRotSpeed(0.2f) {

    // seed RNG
    srand(static_cast<unsigned int>(time(0)));


    // Initialize GLFW
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW\n");
    }

    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // version 3.x
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // version 3.3
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // use glfw core profile

    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(1024, 768, "Rubik's Cube", NULL, NULL);
    if (window == NULL) {
        throw std::runtime_error("Failed to open GLFW window\n");
    }

    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        throw std::runtime_error("Failed to initialize GLEW\n");
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // assign "this" as a window user pointer
    glfwSetWindowUserPointer(window, this);

    // keyboard callback
    glfwSetKeyCallback(window, key_callback);

    // background
    glClearColor((float)(42.0 / 255), (float)(42.0 / 255), (float)(42.0 / 255), (float)(42.0 / 255)); // rgba

    // Enable depth test
    glEnable(GL_DEPTH_TEST);

    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    programID = LoadShaders("shaders/TransformVertexShader.vertexshader", "shaders/ColorFragmentShader.fragmentshader");

    // Get a handle for our MVP uniform
    MatrixID = glGetUniformLocation(programID, "MVP");

    // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
    // Camera matrix
    View = glm::lookAt(
        glm::vec3(4, 3, 5), // Camera position in World Space
        glm::vec3(0, 0, 0), // and looks at the origin
        glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
    );
    // Model matrix : an identity matrix (model will be at the origin)
    Model = glm::mat4(1.0f);

    // generate 1 buffer name (an ID) and store it at &vertexbuffer
    glGenBuffers(1, &vertexbuffer);
    // same as above for color data
    glGenBuffers(1, &colorbuffer);

    // initialize buffer data arrays to null (they will be filled in by the cube every frame)
    for (int i = 0; i < 972; i++) {
        g_vertex_buffer_data[i] = 0;
        g_color_buffer_data[i] = 0;
    }
}

App::~App() {
    // Cleanup VBO and shader
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &colorbuffer);
    glDeleteProgram(programID);
    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();
}

void App::start() {
    loop();
}

void App::loop() {

    // set up delta time variables
    double currentTime = glfwGetTime();
    double lastTime;
    float deltaTime;

    /* Loop until the user closes the window */
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && !glfwWindowShouldClose(window)) {
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use our shader
        glUseProgram(programID);

        // get delta time
        lastTime = currentTime;
        currentTime = glfwGetTime();
        deltaTime = float(currentTime - lastTime);

        Model = glm::rotate(Model, modelRotSpeed * deltaTime, glm::vec3(0, 1, 0));
        // Our ModelViewProjection : multiplication of our 3 matrices
        glm::mat4 MVP = Projection * View * Model; // Remember, matrix multiplication is the other way around

        // turn cube
        cubemngr->update(deltaTime);

        // Send our transformation to the currently bound shader, 
        // in the "MVP" uniform
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

        // update vertex buffer data
        cubemngr->cube->updateVertexData(g_vertex_buffer_data);
        // 1st attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer); // bind buffer to be modified next
        glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_DYNAMIC_DRAW); // modify data
        glVertexAttribPointer(
            0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
        );

        // update color vertex buffer data
        cubemngr->cube->updateColorData(g_color_buffer_data);
        // 2nd attribute buffer : colors
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(
            1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
            3,                                // size
            GL_FLOAT,                         // type
            GL_FALSE,                         // normalized?
            0,                                // stride
            (void*)0                          // array buffer offset
        );

        // Draw the triangle !
        glDrawArrays(GL_TRIANGLES, 0, 3 * 2 * 9 * 6); // 3 * 2 * 9 * 6 total vertices

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }
}

void App::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    
    // Query for window user pointer (the app instance)
    App* app = (App*)glfwGetWindowUserPointer(window);

    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) { // rotate model left
        app->modelRotSpeed = 2.0f;
    } else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) { // rotate model right
        app->modelRotSpeed = -2.0f;
    } else if ((key == GLFW_KEY_RIGHT || key == GLFW_KEY_LEFT) && action == GLFW_RELEASE) {
        app->modelRotSpeed = 0;
    } else if (key == GLFW_KEY_M && action == GLFW_PRESS) {
        app->cubemngr->cube->solveSpeed += 0.5f;
    } else if (key == GLFW_KEY_N && action == GLFW_PRESS) {
        app->cubemngr->cube->solveSpeed -= 0.5f;
    }

    if (key == GLFW_KEY_S && action == GLFW_PRESS) { // scramble cube
        app->cubemngr->cube->scramble();
    } else if (key == GLFW_KEY_R && action == GLFW_PRESS) { // reset cube
        app->cubemngr->resetCube();
    } else if (key == GLFW_KEY_P && action == GLFW_PRESS) { // paint cube
        // by the time this goes out of scope it is no longer needed
        Color paintPattern[9] = { Color::WHITE,    Color::WHITE,   Color::WHITE,
                                  Color::WHITE, Color::WHITE, Color::WHITE,
                                  Color::WHITE, Color::WHITE, Color::WHITE };
        app->ai.calculatePaint(paintPattern);
        app->ai.start();
    } else if (key == GLFW_KEY_X && action == GLFW_PRESS) { // rotate cube across x axis
        std::shared_ptr<Instruction> instruction = std::make_shared<CubeInstruction>(glm::vec3(1, 0, 0));
        app->cubemngr->addToQueue(instruction);
    } else if (key == GLFW_KEY_Y && action == GLFW_PRESS) { // rotate cube across y axis
        std::shared_ptr<Instruction> instruction = std::make_shared<CubeInstruction>(glm::vec3(0, 1, 0));
        app->cubemngr->addToQueue(instruction);
    } else if (key == GLFW_KEY_Z && action == GLFW_PRESS) { // rotate cube across z axis
        std::shared_ptr<Instruction> instruction = std::make_shared<CubeInstruction>(glm::vec3(0, 0, 1));
        app->cubemngr->addToQueue(instruction);
    }
}
