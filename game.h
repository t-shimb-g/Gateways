#pragma once

#include "world.h"
#include "camera.h"
#include "game_object.h"
#include "graphics.h"
#include "audio.h"

#include <memory>

class Game {
public:
    Game(std::string title, int width, int height);
    void handle_event(SDL_Event* event);
    void input();
    void update();
    void render();

private:
    Graphics graphics;
    std::unique_ptr<GameObject> player;
    World* world;
    Camera camera;
    Audio audio;

    // timing
    double dt;
    double lag;
    Uint64 performance_frequency;
    Uint64 prev_counter;
};
