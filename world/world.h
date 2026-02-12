#pragma once

#include <memory>
#include "tilemap.h"
#include "vec.h"

class Player;

class World {
public:
    World(int width, int height);

    void add_platform(float x, float y, float width, float height);
    bool collides(const Vec<float>& position);
    Player* create_player();
    void update(float dt);

    Tilemap tilemap;

private:
    std::unique_ptr<Player> player;
};
