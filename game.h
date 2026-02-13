#pragma once
#include "world.h"
#include "camera.h"

class Game {
public:
    Game(std::string title, int width, int height);
    void input();
    void update();
    void render();

private:
    GameObject* player;
    Graphics graphics;
    World world;
    Camera camera;

    // timing
    double dt;
    double lag;
    Uint64 performance_frequency;
    Uint64 prev_counter;

};
