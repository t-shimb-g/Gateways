#include "game.h"

#include <iostream>

#include "ai_input.h"
#include "asset_manager.h"
#include "fsm.h"
#include "input.h"
#include "level.h"
#include "keyboard_input.h"
#include "states.h"

Game::Game(std::string title, int width, int height)
    : graphics{title, width, height}, camera{graphics, 64}, dt{1.0/60.0}, lag{0.0},
    performance_frequency{SDL_GetPerformanceFrequency()}, prev_counter{SDL_GetPerformanceCounter()} {

    // load events
    get_events();

    // Giver player its assets then put it in the correct state
    create_player();
    AssetManager::get_game_object_details("player", graphics, *player);

    // load first level
    load_level();
}

Game::~Game() {
    delete world;
    for (auto [_, event] : events) {
        delete event;
    }
}

void Game::handle_event(SDL_Event* event) {
    switch (mode) {
        case GameMode::Playing:
            auto action = player->input->collect_discrete_event(event);
            if (action) {
                action->perform(*world, *player);
                delete action;
            }
            break;
    }

}


void Game::input() {
    switch (mode) {
        case GameMode::Playing:
            player->input->get_input();
            camera.handle_input();
            break;
    }
}

void Game::update() {
    Uint64 now = SDL_GetPerformanceCounter();
    lag += (now - prev_counter) / (float)performance_frequency;
    prev_counter = now;
    while (lag >= dt) {
        switch (mode) {
        case GameMode::Playing:
                for (auto obj : world->game_objects) {
                    obj->input->handle_input(*world, *obj);
                }

                world->update(dt);

                // put the camera slightly ahead of the player
                float L = length(player->physics.velocity);
                Vec displacement = 2.0f * player->physics.velocity / (1.0f + L);
                camera.update(player->physics.position + displacement, dt);

                // check for level end
                if (world->end_level) {
                    load_level();
                }

                // check for game over
                if (world->end_game) {
                    if (world->win) {
                        mode = GameMode::Winner;
                    }
                    else {
                        mode = GameMode::GameOver;
                    }
                }
                break;
        }
        lag -= dt;
    }
}

void Game::render() {
    // clear
    graphics.clear();

    // draw the backgrounds
    camera.render(world->backgrounds);

    // draw the world
    camera.render(world->tilemap);

    // draw portals
    Sprite blue_sprite = world->tilemap(world->active_blue_pos.x, world->active_blue_pos.y).sprite;
    Sprite orange_sprite = world->tilemap(world->active_orange_pos.x, world->active_orange_pos.y).sprite;
    camera.render(world->blue_portal_pos_a, blue_sprite, world->active_blue_pos != world->blue_portal_pos_a);
    camera.render(world->blue_portal_pos_b, blue_sprite, world->active_blue_pos != world->blue_portal_pos_b);
    camera.render(world->orange_portal_pos_a, orange_sprite, world->active_orange_pos != world->orange_portal_pos_a);
    camera.render(world->orange_portal_pos_b, orange_sprite, world->active_orange_pos != world->orange_portal_pos_b);

    // draw the player
    camera.render(*player);

    // enemies
    for (auto& obj : world->game_objects) {
        camera.render(*obj);
    }

    // projectiles
    for (auto& projectile : world->projectiles) {
        camera.render(*projectile);
    }

    if (mode == GameMode::GameOver) {
        camera.render_game_over();
    }
    if (mode == GameMode::Winner) {
        camera.render_winner();
    }

    // update
    graphics.update();
}

void Game::get_events() {
    events["next_level"] = new NextLevel();
    events["send_to_blue"] = new SendToBlue();
    events["send_to_orange"] = new SendToOrange();
    events["toggle_blue"] = new ToggleBlue();
    events["toggle_orange"] = new ToggleOrange();
    events["win_game"] = new WinButton();
}

void Game::create_player() {
    // Create FSM
    Transitions transitions = {
        {{StateType::Standing, Transition::Jump}, StateType::InAir},
        {{StateType::InAir, Transition::Stop}, StateType::Standing},
        {{StateType::Standing, Transition::Move}, StateType::Running},
        {{StateType::Running, Transition::Stop}, StateType::Standing},
        {{StateType::Running, Transition::Jump}, StateType::InAir},
        {{StateType::InAir, Transition::Move}, StateType::Strafing},
        {{StateType::Strafing, Transition::Stop}, StateType::InAir}
    };
    States states = {
        {StateType::Standing, new Standing()},
        {StateType::InAir, new InAir()},
        {StateType::Running, new Running()},
        {StateType::Strafing, new Strafing()}
    };
    FSM* fsm = new FSM{transitions, states, StateType::Standing};

    // player input
    KeyboardInput* input = new KeyboardInput();

    player = std::make_unique<GameObject>("player", fsm, input, Color{255, 0, 0, 255});
}

void Game::load_level() {
    std::string level_name = "level_" + std::to_string(++current_level);
    Level level{level_name};
    AssetManager::get_level_details(graphics, level);

    // create the world
    delete world;
    world = new World(level, audio, player.get(), events);

    // get available items
    AssetManager::get_available_items("items", graphics, *world);

    // assets for objs
    for (auto obj : world->game_objects) {
        if (obj == world->player) continue;
        update_enemy(*obj);
        AssetManager::get_game_object_details(obj->obj_name + "-enemy", graphics, *obj, true);
    }

    player->physics.position = {static_cast<float>(level.player_spawn_location.x),
                                   static_cast<float>(level.player_spawn_location.y)};
    player->fsm->current_state->on_enter(*world, *player);
    camera.set_location(player->physics.position);
    audio.play_sound("background", true);
}

void Game::update_enemy(GameObject& obj) {
    Transitions transitions;
    States states;

    if (obj.obj_name == "bee" || obj.obj_name == "slime") {
        transitions = {
            {{StateType::Standing, Transition::Move}, StateType::Patrolling},
            {{StateType::Patrolling, Transition::Stop}, StateType::Standing}
        };
        states = {
            {StateType::Standing, new Standing()},
        };
    }
    else {
        // throw an error, unknown enemy
    }

    FSM* fsm = new FSM{transitions, states, StateType::Patrolling};
    obj.fsm = fsm;

    Input* input = new AiInput{};
    input->next_action_type = ActionType::MoveRight;
    obj.input = input;
}
