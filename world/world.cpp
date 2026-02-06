#include "world.h"
#include "player.h"
#include <SDL3/SDL_rect.h>
#include <algorithm>
#include "vec.h"
#include "physics.h"

void World::add_platform(float x, float y, float width, float height) {
    SDL_FRect rect{x, y, width, height};
    platforms.push_back(rect);
}

const std::vector<SDL_FRect>& World::get_platforms() const {
    return platforms;
}

bool World::has_any_collisions(const SDL_FRect& box) const {
    return std::any_of(std::begin(platforms), std::end(platforms),
                       [&](const SDL_FRect& platform){return SDL_HasRectIntersectionFloat(&platform, &box);});
}

Player* World::create_player() {
    player = std::make_unique<Player>(Vec<float>{600, 300}, Vec<float>{64, 64});
    return player.get();
}

void World::update(float dt) {
    // currently only updating player (will have other game objects later)
    auto position = player->position;
    auto velocity = player->velocity;
    auto acceleration = player->acceleration;

    velocity += 0.5f * acceleration * dt;
    position += velocity * dt;
    velocity += 0.5f * acceleration * dt;
    velocity.x *= damping;

    // check for x collisions
    SDL_FRect future{position.x, player->position.y, player->size.x, player->size.y};
    if (has_any_collisions(future)) {
        player->velocity.x = 0;
        player->acceleration.x = 0;
    }
    else {
        player->position.x = position.x;
        player->velocity.x = velocity.x;
        player->acceleration.x = acceleration.x;
    }
    // check for y collisions
    future.x = player->position.x;
    future.y = position.y;
    if (has_any_collisions(future)) {
        player->velocity.y = 0;
        player->acceleration.x = 0;
    }
    else {
        player->position.y = position.y;
        player->velocity.y = velocity.y;
        player->acceleration.y = acceleration.y;
    }

}