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
#include "Grid.h"

using namespace glm;

void beginInputHandler(Grid* grid);

int main() {
    // Create grid
    Grid grid(1, 1);

    // print cube
    grid.cubes[0]->print();

    // Create app
    App app(&grid);

    // Begin input handler
    std::thread inputThread(beginInputHandler, &grid);

    // Start app
    app.start();

    return 0;
}

/* This function only handles the CLI input. See app->key_callback for GUI input. */
void beginInputHandler(Grid* grid) {
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