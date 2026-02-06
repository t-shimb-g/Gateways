#include "game.h"

Game::Game(std::string title, int width, int height)
    : graphics{title, width, height}, dt{1.0/60.0}, lag{0.0},
    performance_frequency{SDL_GetPerformanceFrequency()}, prev_counter{SDL_GetPerformanceCounter()} {

    // load the first "level"
    world.add_platform(0, 656, 1280, 64);
    world.add_platform(200, 450, 400, 64);
    world.add_platform(600, 200, 250, 64);
    world.add_platform(0, 0, 64, 720);
    world.add_platform(1216, 0, 64, 720);
    player = world.create_player();
}

void Game::input() {
    player->handle_input();
}

void Game::update() {
    Uint64 now = SDL_GetPerformanceCounter(); // how many ticks am i at right now?
    lag += (now - prev_counter) / (float)performance_frequency;
    prev_counter = now;
    while (lag >= dt) {
        world.update(dt);
        lag -= dt;
    }
}

void Game::render() {
    // clear
    graphics.clear();

    // draw the world
    for (auto platform : world.get_platforms()) {
        graphics.draw(platform, {0, 255, 0, 255});
    }
    auto [player_sprite, color] = player->get_sprite();
    graphics.draw(player_sprite, color);

    // update
    graphics.update();
}