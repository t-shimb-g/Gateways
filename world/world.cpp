#include "world.h"
#include <algorithm>
#include "vec.h"
#include "physics.h"
#include "fsm.h"
#include "states.h"
#include "keyboard_input.h"

World::World(int width, int height)
    : tilemap{width, height} {}

void World::add_platform(float x, float y, float width, float height) {
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            tilemap(x+j, y+i) = Tile::Platform;
        }
    }
}

bool World::collides(const Vec<float>& position) {
    int x = std::floor(position.x);
    int y = std::floor(position.y);
    return tilemap(x, y) == Tile::Platform;
}

GameObject* World::create_player() {
    // Create FSM
    Transitions transitions = {
        {{StateType::Standing, Transition::Jump}, StateType::InAir},
        {{StateType::Standing, Transition::Move}, StateType::Running},
        {{StateType::Standing, Transition::Crouch}, StateType::Crouching},
        {{StateType::InAir, Transition::Stop}, StateType::Standing},
        {{StateType::Running, Transition::Stop}, StateType::Standing},
        {{StateType::Running, Transition::Jump}, StateType::InAir},
        {{StateType::Running, Transition::Crouch}, StateType::Crawling},
        {{StateType::Crouching, Transition::Crouch}, StateType::Standing},
        {{StateType::Crouching, Transition::Move}, StateType::Crawling},
        {{StateType::Crawling, Transition::Stop}, StateType::Crouching},
        {{StateType::Crawling, Transition::Crouch}, StateType::Running},
    };
    States states = {
        {StateType::Standing, new Standing()},
        {StateType::InAir, new InAir()},
        {StateType::Running, new Running()},
        {StateType::Crouching, new Crouching()},
        {StateType::Crawling, new Crawling()},
    };
    FSM* fsm = new FSM{transitions, states, StateType::Standing};

    // player input
    KeyboardInput* input = new KeyboardInput();

    player = std::make_unique<GameObject>(Vec<float>{10, 5}, Vec<int>{1, 1}, *this, fsm, input, Color{255, 0, 255, 255});
    return player.get();
}

void World::update(float dt) {
    // currently only updating player (will have other game objects later)
    auto position = player->physics.position;
    auto velocity = player->physics.velocity;
    auto acceleration = player->physics.acceleration;

    velocity += 0.5f * acceleration * dt;
    position += velocity * dt;
    velocity += 0.5f * acceleration * dt;
    velocity.x *= player->physics.damping;

    velocity.x = std::clamp(velocity.x, -player->physics.terminal_velocity, player->physics.terminal_velocity);
    velocity.y = std::clamp(velocity.y, -player->physics.terminal_velocity, player->physics.terminal_velocity);

    // check for collisions in the world - x direction
    Vec<float> future_position{position.x, player->physics.position.y};
    Vec<float> future_velocity{velocity.x, 0};
    move_to(future_position, player->size, future_velocity);

    // now y direction after (maybe) moving in x
    future_velocity.y = velocity.y;
    future_position.y = position.y;
    move_to(future_position, player->size, future_velocity);

    // update the player position and velocity
    player->physics.position = future_position;
    player->physics.velocity = future_velocity;
}

void World::move_to(Vec<float>& position, const Vec<int>& size, Vec<float>& velocity) {
    // cache corner collisions
    const bool bottom_left_collides = collides(position);
    const bool bottom_right_collides = collides(Vec{position.x + size.x, position.y});
    const bool top_left_collides = collides(Vec{position.x, position.y + size.y});
    const bool top_right_collides = collides(Vec{position.x + size.x, position.y + size.y});

    // test for collisions on the bottom or top first
    if (bottom_left_collides && bottom_right_collides) { // bottom
        position.y = std::ceil(position.y);
        velocity.y = 0;
    }
    else if (top_left_collides && top_right_collides) { // top
        position.y = std::floor(position.y);
        velocity.y = 0;
    }

    // then test for collisions on the left and right sides
    if (bottom_left_collides && top_left_collides) { // left
        position.x = std::ceil(position.x);
        velocity.x = 0;
    }
    else if (bottom_right_collides && top_right_collides) { // right
        position.x = std::floor(position.x);
        velocity.x = 0;
    }

    // cache offsets
    const auto dx_left = std::ceil(position.x) - position.x;
    const auto dx_right = position.x - std::floor(position.x);
    const auto dy_bottom = std::ceil(position.y) - position.y;
    const auto dy_top = position.y - std::floor(position.y);

    // now test each corner
    if (bottom_left_collides) {
        // if it's shorter/easier to move in the y-axis compared to the x-axis
        if (dx_left > dy_bottom) {
            // move in y
            position.y = std::ceil(position.y);
            velocity.y = 0;
        }
        else {
            // move in x
            position.x = std::ceil(position.x);
            velocity.x = 0;
        }
    }
    else if (bottom_right_collides) {
        if (dx_right > dy_bottom) {
            position.y = std::ceil(position.y);
            velocity.y = 0;
        }
        else {
            position.x = std::floor(position.x);
            velocity.x = 0;
        }
    }
    else if (top_left_collides) {
        if (dx_left > dy_top) {
            position.y = std::floor(position.y);
            velocity.y = 0;
        }
        else {
            position.x = std::ceil(position.x);
            velocity.x = 0;
        }
    }
    else if (top_right_collides) {
        if (dx_right > dy_top) {
            position.y = std::floor(position.y);
            velocity.y = 0;
        }
        else {
            position.x = std::floor(position.x);
            velocity.x = 0;
        }
    }
}