#include "world.h"

#include <algorithm>

#include "game_object.h"
#include "vec.h"
#include "physics.h"
#include "fsm.h"
#include "keyboard_input.h"
#include "level.h"
#include "audio.h"

World::World(const Level& level, Audio& audio, GameObject* player, Events events)
    : tilemap{level.width, level.height}, audio{&audio}, player{player}, events{events} {
    load_level(level);
}

void World::add_platform(float x, float y, float width, float height) {
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            tilemap(x+j, y+i) = Tile{};
        }
    }
}

bool World::collides(const Vec<float>& position) const {
    int x = std::floor(position.x);
    int y = std::floor(position.y);
    return tilemap(x, y).blocking;
}

void World::update(float dt) {
    // currently only updating player
    auto position = player->physics.position;
    auto velocity = player->physics.velocity;
    auto acceleration = player->physics.acceleration;

    velocity += 0.5f * acceleration * dt;
    position += velocity * dt;
    velocity += 0.5f * acceleration * dt;
    velocity.x *= player->physics.damping;

    velocity.x = std::clamp(velocity.x, -player->physics.terminal_velocity, player->physics.terminal_velocity);
    velocity.y = std::clamp(velocity.y, -player->physics.terminal_velocity, player->physics.terminal_velocity);

    // check for x collisions
    // Check for collisions with the world - x direction
    Vec<float> future_position{position.x, player->physics.position.y};
    Vec<float> future_velocity{velocity.x, 0};
    move_to(future_position, player->size, future_velocity);

    // y direction attempt after (maybe) moving in x
    future_velocity.y = velocity.y;
    future_position.y = position.y;
    move_to(future_position, player->size, future_velocity);

    // update player
    player->physics.position = future_position;
    player->physics.velocity = future_velocity;

    touch_tiles(*player);
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

void World::load_level(const Level& level) {
    audio->load_sounds({});
    for (const auto& [pos, tile_id] : level.tile_locations) {
        tilemap(pos.x, pos.y) = level.tile_types.at(tile_id);
    }
    // get all enemies
    for (const auto& [pos, enemy_name] : level.enemy_locations) {
        GameObject enemy{enemy_name, nullptr, nullptr, {255, 255, 0, 255}};
        enemy.physics.position = pos;
        game_objects.push_back(enemy);
    }
}

void World::touch_tiles(GameObject& obj) {
    int x = std::floor(obj.physics.position.x);
    int y = std::floor(obj.physics.position.y);
    const std::vector<Vec<int>> displacements{
            {0,0},
            {obj.size.x, 0},
            {0, obj.size.y},
            {obj.size.x, obj.size.y}
    };
    for (const auto& displacement : displacements) {
        Tile& tile = tilemap(x+displacement.x, y+displacement.y);
        if (!tile.event_name.empty()) {
            auto itr = events.find(tile.event_name);
            if (itr == events.end()) {
                throw std::runtime_error("Cannot find event: " + tile.event_name);
            }
            itr->second->perform(*this, obj);
        }
    }
}