// include std
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <thread>
#include <memory>
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
#include "Cube.h"
#include "Shader.h"
#include "AI.h"
#include "CubeManager.h"

using namespace glm;

void beginInputHandler(CubeManager* cube);

int main() {
    // Cube patterns
    Color custom[54] = {
        // front
           Color::GREEN,  Color::YELLOW,   Color::WHITE,
           Color::GREEN,   Color::GREEN,  Color::YELLOW,
           Color::GREEN,    Color::BLUE,   Color::WHITE,
        // up
           Color::WHITE,   Color::GREEN,  Color::YELLOW,
           Color::WHITE,   Color::WHITE,    Color::BLUE,
          Color::ORANGE,   Color::GREEN,  Color::ORANGE,
        // back
           Color::GREEN,  Color::ORANGE,  Color::ORANGE,
          Color::YELLOW,    Color::BLUE,  Color::ORANGE,
           Color::GREEN,   Color::WHITE,     Color::RED,
        // down
             Color::RED,     Color::RED,     Color::RED,
          Color::YELLOW,  Color::YELLOW,     Color::RED,
           Color::WHITE,    Color::BLUE,  Color::YELLOW,
        // left
             Color::RED,     Color::RED,  Color::YELLOW,
             Color::RED,  Color::ORANGE,  Color::ORANGE,
          Color::ORANGE,  Color::ORANGE,  Color::YELLOW,
        // right
            Color::BLUE,   Color::WHITE,    Color::BLUE,
            Color::BLUE,     Color::RED,   Color::WHITE,
            Color::BLUE,   Color::GREEN,    Color::BLUE
    };
    Color standard[54] = {
        Color::GREEN, Color::GREEN, Color::GREEN,
        Color::GREEN, Color::GREEN, Color::GREEN,
        Color::GREEN, Color::GREEN, Color::GREEN,

        Color::WHITE, Color::WHITE, Color::WHITE,
        Color::WHITE, Color::WHITE, Color::WHITE,
        Color::WHITE, Color::WHITE, Color::WHITE,

        Color::BLUE, Color::BLUE, Color::BLUE,
        Color::BLUE, Color::BLUE, Color::BLUE,
        Color::BLUE, Color::BLUE, Color::BLUE,

        Color::YELLOW, Color::YELLOW, Color::YELLOW,
        Color::YELLOW, Color::YELLOW, Color::YELLOW,
        Color::YELLOW, Color::YELLOW, Color::YELLOW,

        Color::ORANGE, Color::ORANGE, Color::ORANGE,
        Color::ORANGE, Color::ORANGE, Color::ORANGE,
        Color::ORANGE, Color::ORANGE, Color::ORANGE,

        Color::RED, Color::RED, Color::RED,
        Color::RED, Color::RED, Color::RED,
        Color::RED, Color::RED, Color::RED
    };

    // Create cube manager
    CubeManager cubemngr(standard);

    // print cube
    cubemngr.cube->print();

    // Create AI
    AI ai(&cubemngr);

    // Create app
    App app(&cubemngr, ai);

    // Begin input handler
    // to-do: put graphics on separate thread and move CLI input to main thread
    std::thread inputThread(beginInputHandler, &cubemngr);

    // Start app
    app.start();

    return 0;
}

/* This function only handles the CLI input. See app->key_callback for GUI input. */
void beginInputHandler(CubeManager* cubemngr) {
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
                cubemngr->addToQueue(instruction);
            }
        }
        cout << "Added instruction set to queue." << endl;
    }
}