// include std
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <chrono>
#include <thread>
#include <stdexcept>
#include <functional> // for std::ref
#include <time.h> // for seeding rng
#include <algorithm>

// include GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// include GLFW
#include <GLFW/glfw3.h>

// include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "App.hpp"
#include "Shader.hpp"
#include "BMPImage.hpp"
#include "AI.hpp"

App::App()
 : running(true), camera(glm::vec3(0, 23, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)), fps(0) { // 0, 110, 5

    // Create grid
    grid = new Grid(1, 1);

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
    window = glfwCreateWindow(1024, 800, "Tessellate", NULL, NULL);
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
    programID = LoadShaders("src/shaders/TransformVertexShader.vertexshader", "src/shaders/ColorFragmentShader.fragmentshader");

    // Get a handle for our MVP uniform
    MatrixID = glGetUniformLocation(programID, "MVP");

    // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 1000.0f);

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
    // delete grid
    delete grid;

    // Cleanup VBO and shader
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &colorbuffer);
    glDeleteProgram(programID);
    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();
}

void App::start() {
    std::thread CLIinput(&App::beginInputHandler, this);
    loop();
}

void App::stop() {
    running = false;
}

bool App::isRunning() {
    return running && !glfwWindowShouldClose(window);
}

void App::loop() {

    // set up delta time variables
    double currentTime = glfwGetTime();
    double lastTime;
    float deltaTime;

    /* Loop until the user closes the window */
    // to-do: render on a separate thread than calculations
    while (isRunning()) {

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use our shader
        glUseProgram(programID);

        // get delta time and fps
        lastTime = currentTime;
        currentTime = glfwGetTime();
        deltaTime = float(currentTime - lastTime); // to-do: fix bug that distorts vertices when deltaTime is very large
        // deltaTime = 0.1;
        fps = 1 / deltaTime;

        // main update function
        update(deltaTime);

        for (int i = 0; i < grid->cubes.size(); i++) { // for each cube

            glm::mat4 MVP = Projection * camera.view * grid->cubes[i]->model; // Remember, matrix multiplication is the other way around
            // Send our transformation to the currently bound shader, 
            // in the "MVP" uniform
            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

            // update vertex buffer data
            grid->cubes[i]->getVertexData(g_vertex_buffer_data);
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
            grid->cubes[i]->getColorData(g_color_buffer_data);
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

            // draw
            glDrawArrays(GL_TRIANGLES, 0, 3 * 2 * 9 * 6); // 3 * 2 * 9 * 6 total vertices
            
            // disable vertices and colors
            glDisableVertexAttribArray(0);
            glDisableVertexAttribArray(1);
        }

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();

        // sleep for the remainder of the frame 
        //std::this_thread::sleep_for(std::chrono::seconds(long long(1/60.0 - deltaTime)));
        while (glfwGetTime() - lastTime < 1 / 60.0) {};
    }
}

void App::update(float deltatime) {
    grid->update(deltatime);
    camera.update(deltatime);
}

void App::beginInputHandler() {
    using namespace std;
    while (true) {
        cout << endl << "Enter valid commands (ex. F', B, L, R', U, D):" << endl;

        string commands;
        cin >> commands;

        bool clockwise, invalid;
        FaceType face{};
        // parse input
        while (commands.length() > 0) {
            invalid = false;
            clockwise = !(commands.length() > 1 && commands.at(1) == '\'');
            switch (toupper(commands.at(0))) {
            case 'F':
                face = FaceType::FRONT;
                break;
            case 'B':
                face = FaceType::BACK;
                break;
            case 'U':
                face = FaceType::UP;
                break;
            case 'D':
                face = FaceType::DOWN;
                break;
            case 'R':
                face = FaceType::RIGHT;
                break;
            case 'L':
                face = FaceType::LEFT;
                break;
            default:
                cout << "Invalid command. Skipping." << endl;
                invalid = true;
            }
            // remove chars
            commands.erase(commands.begin()); // remove first char
            if (!clockwise) // if there is a ', remove another char
                commands.erase(commands.begin());
            if (!invalid) {
                std::shared_ptr<Instruction> instruction = std::make_shared<FaceInstruction>(face, clockwise);
                grid->getSelected()->addToQueue(instruction);
            }
        }
        cout << "Added instruction set to queue." << endl;
    }
}

void App::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

    // Query for window user pointer (the app instance)
    App* app = (App*)glfwGetWindowUserPointer(window);

    // exit app
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        app->stop();

    /* CUBE SELECTION */
    } else if (mods != GLFW_MOD_SHIFT && key == GLFW_KEY_UP && action == GLFW_PRESS) { // select cube above selection
        app->grid->selectRelative(0, -1);
        if (app->camera.isInFocusMode()) // focus on cube if camera in focus mode
            app->camera.focusOn(app->grid->getSelected().get());
    } else if (mods != GLFW_MOD_SHIFT && key == GLFW_KEY_DOWN && action == GLFW_PRESS) { // select cube below selection
        app->grid->selectRelative(0, 1);
        if (app->camera.isInFocusMode()) // focus on cube if camera in focus mode
            app->camera.focusOn(app->grid->getSelected().get());
    } else if (mods != GLFW_MOD_SHIFT && key == GLFW_KEY_LEFT && action == GLFW_PRESS) { // select cube left of selection
        app->grid->selectRelative(-1, 0);
        if (app->camera.isInFocusMode()) // focus on cube if camera in focus mode
            app->camera.focusOn(app->grid->getSelected().get());
    } else if (mods != GLFW_MOD_SHIFT && key == GLFW_KEY_RIGHT && action == GLFW_PRESS) { // select cube right of selection
        app->grid->selectRelative(1, 0);
        if (app->camera.isInFocusMode()) // focus on cube if camera in focus mode
            app->camera.focusOn(app->grid->getSelected().get());

    
    /* CAMERA CONTROLS */
    } else if (mods == GLFW_MOD_SHIFT && key == GLFW_KEY_R && action == GLFW_PRESS) { // deselect all cubes and reset camera
        for (std::shared_ptr<Cube> c : app->grid->cubes) // deselect all cubes
            c->deselect();
        app->camera.toggleFocusMode(false); // turn off focus mode
        app->camera.reset(); // reset camera
    } else if (mods == GLFW_MOD_SHIFT && key == GLFW_KEY_F && action == GLFW_PRESS) { // toggle focus mode
        bool inFocus = app->camera.isInFocusMode();
        // toggle focus mode
        app->camera.toggleFocusMode(!inFocus);
        // adjust camera
        if (inFocus)
            app->camera.reset();
        else
            app->camera.focusOn(app->grid->getSelected().get());
    } else if (mods == GLFW_MOD_SHIFT && key == GLFW_KEY_RIGHT && action == GLFW_PRESS) { // orbit camera right
        app->camera.vyaw = 1.5f;
    } else if (mods == GLFW_MOD_SHIFT && key == GLFW_KEY_LEFT && action == GLFW_PRESS) { // orbit camera left
        app->camera.vyaw = -1.5f;
    } else if ((key == GLFW_KEY_RIGHT || key == GLFW_KEY_LEFT) && action == GLFW_RELEASE) { // stop orbitting laterally on release
        app->camera.vyaw = 0;
    } else if (mods == GLFW_MOD_SHIFT && key == GLFW_KEY_UP && action == GLFW_PRESS) { // orbit camera up
        app->camera.vpitch = 0.6f;
    } else if (mods == GLFW_MOD_SHIFT && key == GLFW_KEY_DOWN && action == GLFW_PRESS) { // orbit camera down
        app->camera.vpitch = -0.6f;
    } else if ((key == GLFW_KEY_UP || key == GLFW_KEY_DOWN) && action == GLFW_RELEASE) { // stop orbitting longitudinally on release
        app->camera.vpitch = 0;
    
    
    } else if (key == GLFW_KEY_M && action == GLFW_PRESS) { // increase solve speed
        app->grid->getSelected()->solveSpeed += 0.5f;
    } else if (key == GLFW_KEY_N && action == GLFW_PRESS) { // decrease solve speed
        app->grid->getSelected()->solveSpeed -= 0.5f;
    
    
    } else if (key == GLFW_KEY_F && action == GLFW_PRESS) { // print fps
        std::cout << app->fps << std::endl;
    } else if (key == GLFW_KEY_D && action == GLFW_PRESS) { // print cube 
        app->grid->getSelected()->print();
    } else if (key == GLFW_KEY_S && action == GLFW_PRESS) { // scramble cube
        app->grid->getSelected()->scramble();
    } else if (key == GLFW_KEY_R && action == GLFW_PRESS) { // reset grid
        app->grid->reset();
    
    
    } else if (key == GLFW_KEY_O && action == GLFW_PRESS) { // load image and paint grid
        BMPImage bmp("../dependencies/images/output marilyn.bmp");
        app->grid->solveImage(bmp);
        // set default camera position to an aerial view 
        // determine the y value of camera based off of how max rows/columns there are - lower dimension = zoomed out by a higher factor
        size_t maxDimension = std::max(app->grid->nCols, app->grid->nRows);
        float y = (maxDimension == 1 ? maxDimension * 23 : maxDimension < 4 ? maxDimension * 11 : maxDimension * 7);
        app->camera.setDefaultEyePosition(glm::vec3(0, y, 5));
    } else if (key == GLFW_KEY_P && action == GLFW_PRESS) { // paint selected cube
        Color paintPattern[9] = { 
            Color::BLUE,   Color::WHITE,   Color::GREEN,
            Color::WHITE,   Color::ORANGE,   Color::WHITE,
            Color::GREEN,   Color::WHITE,   Color::BLUE };
        AI ai(app->grid->getSelected().get());
        ai.calculatePaint(paintPattern);
        ai.start();
    
    
    } else if (key == GLFW_KEY_X && action == GLFW_PRESS) { // rotate cube across x axis
        std::shared_ptr<Instruction> instruction = std::make_shared<CubeInstruction>(glm::vec3(1, 0, 0));
        app->grid->getSelected()->addToQueue(instruction);
    } else if (key == GLFW_KEY_Y && action == GLFW_PRESS) { // rotate cube across y axis
        std::shared_ptr<Instruction> instruction = std::make_shared<CubeInstruction>(glm::vec3(0, 1, 0));
        app->grid->getSelected()->addToQueue(instruction);
    } else if (key == GLFW_KEY_Z && action == GLFW_PRESS) { // rotate cube across z axis
        std::shared_ptr<Instruction> instruction = std::make_shared<CubeInstruction>(glm::vec3(0, 0, 1));
        app->grid->getSelected()->addToQueue(instruction);
    }
}

