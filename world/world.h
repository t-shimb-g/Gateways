#pragma once

#include <SDL3/SDL.h>
#include <vector>
#include <memory>

class Player;

class World {
public:
    void add_platform(float x, float y, float width, float height);
    const std::vector<SDL_FRect>& get_platforms() const;
    bool has_any_collisions(const SDL_FRect& box) const;
    Player* create_player();
    void update(float dt);

private:
    std::vector<SDL_FRect> platforms;
    std::unique_ptr<Player> player;
};
